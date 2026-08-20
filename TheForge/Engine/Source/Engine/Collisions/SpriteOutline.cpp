#include "SpriteOutline.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <unordered_map>

#include <glm/geometric.hpp>

#include "Engine/System.h"
#include "stb_image.h"

namespace
{
    using Engine::ConvexPolygon;

    constexpr float EPSILON = 1e-4f;

    // --- pixel mask -------------------------------------------------------------

    struct Mask
    {
        int width = 0;
        int height = 0;
        std::vector<uint8_t> solid;

        bool At(const int x, const int y) const
        {
            if (x < 0 || y < 0 || x >= width || y >= height) return false;
            return solid[static_cast<size_t>(y) * width + x] != 0;
        }
    };

    // --- boundary tracing -------------------------------------------------------

    // A grid point on the pixel lattice. Integer, so points can be compared and used as
    // map keys exactly -- the whole trace depends on two edges agreeing that they meet
    // at the same corner, which floats would not guarantee.
    struct GridPoint
    {
        int x = 0;
        int y = 0;

        bool operator==(const GridPoint& other) const { return x == other.x && y == other.y; }
        bool operator<(const GridPoint& other) const { return y != other.y ? y < other.y : x < other.x; }
    };

    // Walks the outline of every solid region by following pixel edges.
    //
    // Each solid pixel contributes an edge for every side whose neighbour is empty,
    // directed so that the solid is consistently on one side. Chained head-to-tail those
    // edges form closed loops: the outside of each region, and the inside of each hole.
    //
    // Marching squares would do the same job, but on axis-aligned pixel data it produces
    // exactly these edges anyway, with a case table in between.
    std::vector<std::vector<GridPoint>> TraceBoundaryLoops(const Mask& mask)
    {
        // Every boundary edge, keyed by where it starts. A corner can be the start of two
        // edges where two regions touch diagonally, so this is a multimap: the trace then
        // takes whichever is still unused, which splits the figure-eight into two loops
        // rather than failing.
        std::multimap<GridPoint, GridPoint> edges;

        for (int y = 0; y < mask.height; ++y)
        {
            for (int x = 0; x < mask.width; ++x)
            {
                if (!mask.At(x, y)) continue;

                const GridPoint topLeft{ x, y };
                const GridPoint topRight{ x + 1, y };
                const GridPoint bottomLeft{ x, y + 1 };
                const GridPoint bottomRight{ x + 1, y + 1 };

                // Wound so the four edges of a lone pixel chain into one closed loop.
                if (!mask.At(x, y - 1)) edges.emplace(topRight, topLeft);
                if (!mask.At(x - 1, y)) edges.emplace(topLeft, bottomLeft);
                if (!mask.At(x, y + 1)) edges.emplace(bottomLeft, bottomRight);
                if (!mask.At(x + 1, y)) edges.emplace(bottomRight, topRight);
            }
        }

        std::vector<std::vector<GridPoint>> loops;

        while (!edges.empty())
        {
            const GridPoint start = edges.begin()->first;

            std::vector<GridPoint> loop;
            GridPoint current = start;

            while (true)
            {
                const auto it = edges.find(current);
                if (it == edges.end()) break;

                loop.push_back(current);
                const GridPoint next = it->second;
                edges.erase(it);

                current = next;
                if (current == start) break;
            }

            if (loop.size() >= 4)
                loops.push_back(std::move(loop));
        }

        return loops;
    }

    // --- simplification ---------------------------------------------------------

    float PerpendicularDistance(const glm::vec2 point, const glm::vec2 lineStart, const glm::vec2 lineEnd)
    {
        const glm::vec2 line = lineEnd - lineStart;
        const float lengthSquared = dot(line, line);
        if (lengthSquared <= EPSILON) return glm::length(point - lineStart);

        const float t = std::clamp(dot(point - lineStart, line) / lengthSquared, 0.0f, 1.0f);
        return glm::length(point - (lineStart + t * line));
    }

    void DouglasPeucker(const std::vector<glm::vec2>& points, const size_t first, const size_t last,
                        const float tolerance, std::vector<glm::vec2>& out)
    {
        if (last <= first + 1) return;

        float worst = 0.0f;
        size_t worstIndex = first;

        for (size_t i = first + 1; i < last; ++i)
        {
            if (const float distance = PerpendicularDistance(points[i], points[first], points[last]); distance > worst)
            {
                worst = distance;
                worstIndex = i;
            }
        }

        if (worst <= tolerance) return;

        DouglasPeucker(points, first, worstIndex, tolerance, out);
        out.push_back(points[worstIndex]);
        DouglasPeucker(points, worstIndex, last, tolerance, out);
    }

    // Douglas-Peucker over a closed loop.
    //
    // The algorithm needs two fixed endpoints, and a loop has none -- so the two points
    // furthest apart are used, which are guaranteed to survive any simplification and so
    // cannot distort the result the way an arbitrary starting vertex would.
    ConvexPolygon SimplifyLoop(const ConvexPolygon& loop, const float tolerance)
    {
        if (loop.size() < 4 || tolerance <= 0.0f) return loop;

        size_t anchorA = 0;
        size_t anchorB = 0;
        float furthest = -1.0f;

        for (size_t i = 0; i < loop.size(); ++i)
        {
            for (size_t j = i + 1; j < loop.size(); ++j)
            {
                if (const float distanceSquared = dot(loop[j] - loop[i], loop[j] - loop[i]); distanceSquared > furthest)
                {
                    furthest = distanceSquared;
                    anchorA = i;
                    anchorB = j;
                }
            }
        }

        // The loop cut into two open chains at the anchors, each simplified on its own.
        std::vector<glm::vec2> first(loop.begin() + anchorA, loop.begin() + anchorB + 1);

        std::vector<glm::vec2> second(loop.begin() + anchorB, loop.end());
        second.insert(second.end(), loop.begin(), loop.begin() + anchorA + 1);

        ConvexPolygon result;
        result.push_back(first.front());
        DouglasPeucker(first, 0, first.size() - 1, tolerance, result);
        result.push_back(first.back());
        DouglasPeucker(second, 0, second.size() - 1, tolerance, result);

        return result;
    }

    // Drops points that repeat or sit on the straight line between their neighbours.
    // Ear clipping treats a zero-area corner as a valid ear and emits degenerate
    // triangles from it, which then poison the convexity tests during merging.
    ConvexPolygon RemoveDegenerateVertices(const ConvexPolygon& polygon)
    {
        ConvexPolygon result;
        const size_t count = polygon.size();

        for (size_t i = 0; i < count; ++i)
        {
            const glm::vec2 previous = result.empty() ? polygon[(i + count - 1) % count] : result.back();
            const glm::vec2 current = polygon[i];
            const glm::vec2 next = polygon[(i + 1) % count];

            if (glm::length(current - previous) <= EPSILON) continue;

            const glm::vec2 in = current - previous;
            const glm::vec2 out = next - current;
            if (std::abs(in.x * out.y - in.y * out.x) <= EPSILON &&
                dot(in, out) > 0.0f)
                continue;

            result.push_back(current);
        }

        return result;
    }

    // --- polygon helpers --------------------------------------------------------

    float Cross(const glm::vec2 a, const glm::vec2 b) { return a.x * b.y - a.y * b.x; }

    // Strictly inside: a point exactly on an edge does not count.
    //
    // The distinction decides whether ear clipping works at all on pixel art. A traced
    // outline is rectilinear, so vertices land on the lines through other vertices
    // constantly -- an inclusive test finds one sitting on almost every candidate ear,
    // rejects all of them, and reports a perfectly good polygon as untriangulable.
    // Allowing a vertex to touch an ear's edge costs at most a degenerate sliver.
    //
    // Assumes a counter-clockwise triangle, which every ear is by the time it is tested.
    bool PointInTriangle(const glm::vec2 point, const glm::vec2 a, const glm::vec2 b, const glm::vec2 c)
    {
        return Cross(b - a, point - a) > EPSILON
            && Cross(c - b, point - b) > EPSILON
            && Cross(a - c, point - c) > EPSILON;
    }

    bool SegmentsProperlyIntersect(const glm::vec2 p1, const glm::vec2 p2, const glm::vec2 q1, const glm::vec2 q2)
    {
        const float d1 = Cross(p2 - p1, q1 - p1);
        const float d2 = Cross(p2 - p1, q2 - p1);
        const float d3 = Cross(q2 - q1, p1 - q1);
        const float d4 = Cross(q2 - q1, p2 - q1);

        // Deliberately proper: a bridge to a hole touches the outline at its endpoints,
        // and counting that as a crossing would reject every candidate bridge there is.
        return ((d1 > EPSILON && d2 < -EPSILON) || (d1 < -EPSILON && d2 > EPSILON)) &&
               ((d3 > EPSILON && d4 < -EPSILON) || (d3 < -EPSILON && d4 > EPSILON));
    }

    // Whether the loop crosses itself anywhere. Touching at a shared vertex is fine --
    // that is what adjacent edges do, and what a bridged hole does at the cut.
    bool IsSimplePolygon(const ConvexPolygon& polygon)
    {
        const size_t count = polygon.size();
        if (count < 4) return true;

        for (size_t i = 0; i < count; ++i)
        {
            for (size_t j = i + 2; j < count; ++j)
            {
                if ((j + 1) % count == i) continue;

                if (SegmentsProperlyIntersect(polygon[i], polygon[(i + 1) % count],
                                              polygon[j], polygon[(j + 1) % count]))
                    return false;
            }
        }

        return true;
    }

    // Simplification that is guaranteed to hand back a polygon.
    //
    // Douglas-Peucker can pull an outline across itself: shortcutting a corner on one
    // side of a narrow neck can put the shortcut through the wall on the other side. The
    // result is still a list of points, and still looks plausible, but it is no longer a
    // simple polygon -- ear clipping finds no ear in it and gives up, and the shape falls
    // back to its convex hull. On a cockpit canopy that is nearly twice the real area,
    // and the collider silently becomes a box.
    //
    // Rather than hoping one tolerance is safe for every sprite, the result is checked
    // and the tolerance halved until it is. The raw trace follows pixel edges, so the
    // last resort cannot self-intersect.
    ConvexPolygon SimplifyLoopSafely(const ConvexPolygon& loop, float tolerance)
    {
        for (int attempt = 0; attempt < 5 && tolerance > 0.01f; ++attempt, tolerance *= 0.5f)
        {
            ConvexPolygon simplified = RemoveDegenerateVertices(SimplifyLoop(loop, tolerance));
            if (simplified.size() >= 3 && IsSimplePolygon(simplified))
                return simplified;
        }

        return RemoveDegenerateVertices(loop);
    }

    bool PointInPolygon(const glm::vec2 point, const ConvexPolygon& polygon)
    {
        bool inside = false;
        for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++)
        {
            if ((polygon[i].y > point.y) == (polygon[j].y > point.y)) continue;

            const float crossingX = polygon[i].x +
                (point.y - polygon[i].y) / (polygon[j].y - polygon[i].y) * (polygon[j].x - polygon[i].x);

            if (point.x < crossingX) inside = !inside;
        }

        return inside;
    }

    ConvexPolygon ConvexHull(std::vector<glm::vec2> points)
    {
        if (points.size() < 3) return points;

        std::ranges::sort(points, [](const glm::vec2 a, const glm::vec2 b)
        {
            return a.x != b.x ? a.x < b.x : a.y < b.y;
        });

        // Monotone chain: the lower hull then the upper, each keeping only left turns.
        ConvexPolygon hull;
        for (int pass = 0; pass < 2; ++pass)
        {
            const size_t start = hull.size();
            for (const glm::vec2 point : points)
            {
                while (hull.size() >= start + 2 &&
                       Cross(hull[hull.size() - 1] - hull[hull.size() - 2], point - hull[hull.size() - 2]) <= 0.0f)
                    hull.pop_back();

                hull.push_back(point);
            }

            hull.pop_back();
            std::ranges::reverse(points);
        }

        return hull;
    }

    // --- hole bridging ----------------------------------------------------------

    // Splices a hole into the outline that contains it, joining them with a zero-width
    // cut so what remains is one simple polygon that ear clipping can handle.
    //
    // Without this a hole is simply not represented: the decomposition fills it in, and
    // a ring-shaped sprite collides as a solid disc.
    bool BridgeHole(ConvexPolygon& outer, const ConvexPolygon& hole)
    {
        if (hole.size() < 3) return false;

        // The hole's rightmost vertex, because the cut leaves from there towards the
        // outline and the rightmost point of an inner loop always has clear line of
        // sight to *something* outside it.
        size_t holeIndex = 0;
        for (size_t i = 1; i < hole.size(); ++i)
            if (hole[i].x > hole[holeIndex].x)
                holeIndex = i;

        const glm::vec2 from = hole[holeIndex];

        // The nearest outline vertex the cut can reach without crossing anything. Tested
        // rather than derived: the textbook version computes a visible vertex from the
        // ray crossing, which is faster and much easier to get subtly wrong, and these
        // loops are tens of vertices after simplification.
        size_t bestIndex = outer.size();
        float bestDistanceSquared = 0.0f;

        for (size_t candidate = 0; candidate < outer.size(); ++candidate)
        {
            const glm::vec2 to = outer[candidate];
            const float distanceSquared = dot(to - from, to - from);
            if (bestIndex != outer.size() && distanceSquared >= bestDistanceSquared) continue;

            bool blocked = false;

            for (size_t i = 0; i < outer.size() && !blocked; ++i)
                blocked = SegmentsProperlyIntersect(from, to, outer[i], outer[(i + 1) % outer.size()]);

            for (size_t i = 0; i < hole.size() && !blocked; ++i)
                blocked = SegmentsProperlyIntersect(from, to, hole[i], hole[(i + 1) % hole.size()]);

            // The midpoint test catches a cut that crosses nothing but still runs outside
            // the material -- across the mouth of a concave bay, say.
            if (!blocked && !PointInPolygon((from + to) * 0.5f, outer)) blocked = true;
            if (!blocked && PointInPolygon((from + to) * 0.5f, hole)) blocked = true;

            if (blocked) continue;

            bestIndex = candidate;
            bestDistanceSquared = distanceSquared;
        }

        if (bestIndex == outer.size()) return false;

        // outline[0..bridge] + the whole hole starting and ending at `from` + the rest of
        // the outline starting at the bridge vertex again. The two repeated vertices are
        // the cut, and have zero width by construction.
        ConvexPolygon spliced(outer.begin(), outer.begin() + bestIndex + 1);

        for (size_t i = 0; i < hole.size(); ++i)
            spliced.push_back(hole[(holeIndex + i) % hole.size()]);

        spliced.push_back(from);
        spliced.insert(spliced.end(), outer.begin() + bestIndex, outer.end());

        outer = std::move(spliced);
        return true;
    }

    // --- decomposition ----------------------------------------------------------

    std::vector<ConvexPolygon> EarClip(const ConvexPolygon& polygon)
    {
        std::vector<ConvexPolygon> triangles;

        std::vector<size_t> remaining(polygon.size());
        for (size_t i = 0; i < polygon.size(); ++i) remaining[i] = i;

        size_t failures = 0;

        while (remaining.size() > 3)
        {
            const size_t count = remaining.size();
            bool clipped = false;

            for (size_t i = 0; i < count; ++i)
            {
                const glm::vec2 previous = polygon[remaining[(i + count - 1) % count]];
                const glm::vec2 current = polygon[remaining[i]];
                const glm::vec2 next = polygon[remaining[(i + 1) % count]];

                // Reflex corners are not ears.
                if (Cross(current - previous, next - current) <= EPSILON) continue;

                // Only reflex vertices can block an ear: a convex one cannot lie inside
                // its own polygon's ear without the outline crossing itself. Skipping the
                // rest is both faster and one less chance to reject a valid ear.
                bool contains = false;
                for (size_t j = 0; j < count && !contains; ++j)
                {
                    if (j == i || j == (i + count - 1) % count || j == (i + 1) % count) continue;

                    const glm::vec2 before = polygon[remaining[(j + count - 1) % count]];
                    const glm::vec2 at = polygon[remaining[j]];
                    const glm::vec2 after = polygon[remaining[(j + 1) % count]];
                    if (Cross(at - before, after - at) > EPSILON) continue;

                    contains = PointInTriangle(at, previous, current, next);
                }

                if (contains) continue;

                triangles.push_back({ previous, current, next });
                remaining.erase(remaining.begin() + static_cast<long long>(i));
                clipped = true;
                break;
            }

            if (clipped)
            {
                failures = 0;
                continue;
            }

            // No ear anywhere. A well-formed simple polygon always has one, so reaching
            // here means the outline self-intersects -- which a bridged hole can produce
            // if the cut was chosen badly. Giving up beats looping forever; the caller
            // falls back to the convex hull.
            if (++failures > 0) return {};
        }

        if (remaining.size() == 3)
            triangles.push_back({ polygon[remaining[0]], polygon[remaining[1]], polygon[remaining[2]] });

        return triangles;
    }

    bool IsConvex(const ConvexPolygon& polygon)
    {
        if (polygon.size() < 3) return false;

        for (size_t i = 0; i < polygon.size(); ++i)
        {
            const glm::vec2 a = polygon[i];
            const glm::vec2 b = polygon[(i + 1) % polygon.size()];
            const glm::vec2 c = polygon[(i + 2) % polygon.size()];

            if (Cross(b - a, c - b) < -EPSILON) return false;
        }

        return true;
    }

    // Merges neighbouring pieces back together wherever the result stays convex.
    //
    // This is what makes the decomposition usable: ear clipping a 40-vertex outline
    // gives 38 triangles, and every one of them is a shape to test against every frame.
    // Merging typically brings that down by an order of magnitude, and each merge is
    // strictly an improvement -- a shared edge that disappears is an axis nobody has to
    // test any more.
    void MergeConvexPieces(std::vector<ConvexPolygon>& pieces)
    {
        bool merged = true;
        while (merged)
        {
            merged = false;

            for (size_t a = 0; a < pieces.size() && !merged; ++a)
            {
                for (size_t b = a + 1; b < pieces.size() && !merged; ++b)
                {
                    const ConvexPolygon& first = pieces[a];
                    const ConvexPolygon& second = pieces[b];

                    for (size_t i = 0; i < first.size() && !merged; ++i)
                    {
                        const glm::vec2 edgeStart = first[i];
                        const glm::vec2 edgeEnd = first[(i + 1) % first.size()];

                        for (size_t j = 0; j < second.size(); ++j)
                        {
                            // The same edge seen from the other side runs the other way.
                            if (glm::length(second[j] - edgeEnd) > EPSILON) continue;
                            if (glm::length(second[(j + 1) % second.size()] - edgeStart) > EPSILON) continue;

                            // Both pieces walked from the shared edge outwards, which
                            // drops the two duplicated vertices and closes the seam.
                            ConvexPolygon candidate;
                            for (size_t k = 1; k <= first.size(); ++k)
                                candidate.push_back(first[(i + k) % first.size()]);

                            for (size_t k = 2; k < second.size(); ++k)
                                candidate.push_back(second[(j + k) % second.size()]);

                            candidate = RemoveDegenerateVertices(candidate);
                            if (!IsConvex(candidate)) break;

                            pieces[a] = std::move(candidate);
                            pieces.erase(pieces.begin() + static_cast<long long>(b));
                            merged = true;
                            break;
                        }
                    }
                }
            }
        }
    }
}

float Engine::SignedArea(const ConvexPolygon& polygon)
{
    float area = 0.0f;
    for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++)
        area += polygon[j].x * polygon[i].y - polygon[i].x * polygon[j].y;

    return area * 0.5f;
}

std::vector<Engine::ConvexPolygon> Engine::DecomposeToConvex(const ConvexPolygon& loop, const int maxPieces)
{
    ConvexPolygon normalised = RemoveDegenerateVertices(loop);
    if (normalised.size() < 3) return {};

    // Counter-clockwise, which everything below assumes: ear clipping tests corners for
    // a positive turn, and the merge tests convexity the same way.
    if (SignedArea(normalised) < 0.0f)
        std::ranges::reverse(normalised);

    if (IsConvex(normalised))
        return { normalised };

    std::vector<ConvexPolygon> pieces = EarClip(normalised);
    if (pieces.empty())
        return { ConvexHull(normalised) };

    MergeConvexPieces(pieces);

    if (static_cast<int>(pieces.size()) > maxPieces)
        return { ConvexHull(normalised) };

    return pieces;
}

nlohmann::json Engine::SpriteOutline::Serialize() const
{
    nlohmann::json data = nlohmann::json::array();
    for (const ConvexPolygon& piece : pieces)
    {
        nlohmann::json loop = nlohmann::json::array();
        for (const glm::vec2 vertex : piece)
            loop.push_back({ vertex.x, vertex.y });

        data.push_back(loop);
    }

    return data;
}

Engine::SpriteOutline Engine::SpriteOutline::Deserialize(const nlohmann::json& data)
{
    SpriteOutline outline;
    if (!data.is_array()) return outline;

    for (const auto& loop : data)
    {
        if (!loop.is_array()) continue;

        ConvexPolygon piece;
        for (const auto& vertex : loop)
        {
            if (!vertex.is_array() || vertex.size() != 2) continue;
            piece.emplace_back(vertex[0].get<float>(), vertex[1].get<float>());
        }

        if (piece.size() >= 3)
            outline.pieces.push_back(std::move(piece));
    }

    return outline;
}

Engine::SpriteOutline Engine::BuildSpriteOutline(const std::string& imagePath, const OutlineSettings& settings)
{
    SpriteOutline outline;

    int width, height, channels;
    // Four channels regardless of what the file holds, so alpha is always at a known
    // offset -- an image with no alpha decodes as fully opaque, which is correct.
    unsigned char* pixels = stbi_load(imagePath.c_str(), &width, &height, &channels, 4);
    if (pixels == nullptr)
    {
        DEBUG_LOG("Outline: could not decode '%s'", imagePath.c_str())
        return outline;
    }

    Mask mask;
    mask.width = width;
    mask.height = height;
    mask.solid.resize(static_cast<size_t>(width) * height);

    for (size_t i = 0; i < mask.solid.size(); ++i)
        mask.solid[i] = pixels[i * 4 + 3] >= settings.alphaThreshold ? 1 : 0;

    stbi_image_free(pixels);

    const std::vector<std::vector<GridPoint>> traced = TraceBoundaryLoops(mask);
    if (traced.empty()) return outline;

    // Pixel lattice -> sprite-local units. The origin moves to the sprite's centre and Y
    // flips, because the image has Y down and the world has it up.
    const float halfWidth = static_cast<float>(width) * 0.5f;
    const float halfHeight = static_cast<float>(height) * 0.5f;

    std::vector<ConvexPolygon> solids;
    std::vector<ConvexPolygon> holes;

    for (const std::vector<GridPoint>& loop : traced)
    {
        ConvexPolygon converted;
        converted.reserve(loop.size());
        for (const GridPoint point : loop)
            converted.emplace_back(static_cast<float>(point.x) - halfWidth,
                                   halfHeight - static_cast<float>(point.y));

        converted = SimplifyLoopSafely(converted, settings.simplifyTolerance);
        if (converted.size() < 3) continue;

        // The winding says which it is: the trace runs solid boundaries one way and hole
        // boundaries the other, and flipping Y above turned that into "outer loops are
        // counter-clockwise".
        const float area = SignedArea(converted);
        if (std::abs(area) < settings.minimumArea) continue;

        if (area > 0.0f) solids.push_back(std::move(converted));
        else holes.push_back(std::move(converted));
    }

    if (solids.empty()) return outline;

    // Each hole belongs to whichever solid region encloses it.
    for (const ConvexPolygon& hole : holes)
    {
        ConvexPolygon* container = nullptr;
        float smallestArea = 0.0f;

        for (ConvexPolygon& solid : solids)
        {
            if (!PointInPolygon(hole[0], solid)) continue;

            // The smallest enclosing region, so a hole inside a region that itself sits
            // inside a larger one is attached to the right neighbour.
            if (const float area = std::abs(SignedArea(solid)); container == nullptr || area < smallestArea)
            {
                container = &solid;
                smallestArea = area;
            }
        }

        if (container == nullptr) continue;

        if (!BridgeHole(*container, hole))
            DEBUG_LOG("Outline: could not cut a hole into '%s'; it will collide as solid", imagePath.c_str())
    }

    for (const ConvexPolygon& solid : solids)
    {
        std::vector<ConvexPolygon> pieces = DecomposeToConvex(solid, settings.maxPieces);

        // One piece back from a concave region means the decomposition gave up and
        // returned the hull. Worth saying: the collider is then noticeably larger than
        // the sprite, which is exactly the kind of thing that gets blamed on the physics.
        if (pieces.size() == 1 && std::abs(SignedArea(pieces[0])) > std::abs(SignedArea(solid)) * 1.05f)
            DEBUG_LOG("Outline: a region of '%s' could not be decomposed; using its convex hull, which is %.0f%% larger",
                imagePath.c_str(),
                (std::abs(SignedArea(pieces[0])) / std::abs(SignedArea(solid)) - 1.0f) * 100.0f)

        outline.pieces.insert(outline.pieces.end(), pieces.begin(), pieces.end());
    }

    if (static_cast<int>(outline.pieces.size()) > settings.maxPieces)
    {
        DEBUG_LOG("Outline: '%s' decomposed into %zu pieces, past the limit of %d; using the convex hull",
            imagePath.c_str(), outline.pieces.size(), settings.maxPieces)

        std::vector<glm::vec2> everything;
        for (const ConvexPolygon& piece : outline.pieces)
            everything.insert(everything.end(), piece.begin(), piece.end());

        outline.pieces = { ConvexHull(std::move(everything)) };
    }

    return outline;
}
