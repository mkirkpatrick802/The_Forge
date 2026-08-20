#include "PolygonCollider.h"

#include <algorithm>

#include <glm/glm.hpp>

#include "SpriteRenderer.h"
#include "imgui.h"
#include "Engine/AssetMetadata.h"
#include "Engine/GameObject.h"
#include "Engine/System.h"
#include "Engine/Rendering/ImGuiHelper.h"

Engine::PolygonCollider::PolygonCollider()
{
    type = EColliderType::ECT_Polygon;
}

void Engine::PolygonCollider::Start()
{
    ReloadShape();
}

void Engine::PolygonCollider::EnsureShapeLoaded() const
{
    if (_shapeLoaded) return;
    ReloadShape();
}

void Engine::PolygonCollider::ReloadShape() const
{
    _shapeLoaded = true;
    _cacheValid = false;
    _localShape = {};

    std::string path = _spritePath;

    // Falls back to the sprite on this object, which is the point of the component:
    // add it next to a SpriteRenderer and the collider is the sprite.
    if (path.empty() && gameObject != nullptr)
        if (const auto sprite = gameObject->GetComponent<SpriteRenderer>())
            path = sprite->GetSpritePath();

    if (path.empty())
    {
        DEBUG_LOG("PolygonCollider on '%s' has no sprite to take its shape from.",
            gameObject != nullptr ? gameObject->GetName().c_str() : "(no object)")
        return;
    }

    _localShape = GetImageCollisionShape(path);

    if (_localShape.IsEmpty())
        DEBUG_LOG("PolygonCollider on '%s': '%s' has no traced outline. Re-import it (Content Drawer, Reimport All).",
            gameObject != nullptr ? gameObject->GetName().c_str() : "(no object)", path.c_str())
}

const std::vector<Engine::ConvexPolygon>& Engine::PolygonCollider::GetWorldPieces() const
{
    EnsureShapeLoaded();

    const glm::vec2 center = GetCenter();
    const float rotation = gameObject != nullptr ? gameObject->GetWorldRotation() : 0.0f;

    if (_cacheValid &&
        _cachedScale == _scale &&
        _cachedRotation == rotation &&
        _cachedCenter == center)
        return _worldPieces;

    // Rotation is honoured here, unlike the rectangle and circle colliders, which are
    // deliberately axis-aligned. A traced outline is the one shape where orientation
    // actually carries information -- and the separating axis test handles an arbitrary
    // one for free, so there is nothing to be gained by throwing it away.
    const float radians = glm::radians(rotation);
    const float cosR = std::cos(radians);
    const float sinR = std::sin(radians);

    _worldPieces.resize(_localShape.pieces.size());

    for (size_t i = 0; i < _localShape.pieces.size(); ++i)
    {
        const ConvexPolygon& local = _localShape.pieces[i];
        ConvexPolygon& world = _worldPieces[i];
        world.resize(local.size());

        for (size_t v = 0; v < local.size(); ++v)
        {
            const glm::vec2 scaled = local[v] * _scale;
            world[v] = center + glm::vec2(scaled.x * cosR - scaled.y * sinR,
                                          scaled.x * sinR + scaled.y * cosR);
        }
    }

    _cachedCenter = center;
    _cachedRotation = rotation;
    _cachedScale = _scale;
    _cacheValid = true;

    return _worldPieces;
}

void Engine::PolygonCollider::GetWorldBounds(glm::vec2& outMin, glm::vec2& outMax) const
{
    const std::vector<ConvexPolygon>& pieces = GetWorldPieces();

    if (pieces.empty() || pieces[0].empty())
    {
        // A collider with no shape occupies the single point it sits at, rather than
        // everything or nothing -- both of which read as a bug somewhere else.
        outMin = GetCenter();
        outMax = GetCenter();
        return;
    }

    // Component-wise by hand, and the limits parenthesised. System.h pulls in Windows.h,
    // which defines min and max as macros -- they expand even after a glm:: or std::
    // qualifier, so neither library's version can be named here at all.
    outMin = pieces[0][0];
    outMax = pieces[0][0];

    for (const ConvexPolygon& piece : pieces)
    {
        for (const glm::vec2 vertex : piece)
        {
            if (vertex.x < outMin.x) outMin.x = vertex.x;
            if (vertex.y < outMin.y) outMin.y = vertex.y;
            if (vertex.x > outMax.x) outMax.x = vertex.x;
            if (vertex.y > outMax.y) outMax.y = vertex.y;
        }
    }
}

void Engine::PolygonCollider::DrawDetails()
{
    EnsureShapeLoaded();

    int pieceCount = 0;
    int vertexCount = 0;
    for (const ConvexPolygon& piece : _localShape.pieces)
    {
        ++pieceCount;
        vertexCount += static_cast<int>(piece.size());
    }

    if (pieceCount == 0)
        ImGui::TextColored(ImVec4(0.95f, 0.5f, 0.4f, 1.0f), "No traced outline -- re-import the sprite.");
    else
        ImGui::Text("%d convex piece(s), %d vertices", pieceCount, vertexCount);

    ImGui::Spacing();

    ImGui::PushItemWidth(150);
    if (ImGui::DragFloat("Scale", &_scale, 0.01f, 0.05f, 10.0f))
        _cacheValid = false;
    ImGui::PopItemWidth();

    ImGui::Spacing();

    // Empty means "whatever this object draws". Set it only to trace a different image
    // than the one on screen -- a simplified collision proxy, say.
    if (ImGuiHelper::DragDropFileButton("Shape Source", _spritePath, "FILE_PATH"))
        ReloadShape();

    ImGui::SameLine();
    if (ImGui::Button("Reload"))
        ReloadShape();

    ImGuiHelper::DisplayFilePath("Traced From", _spritePath.empty() ? std::string("(this object's sprite)") : _spritePath);

    Collider::DrawDetails();
}

nlohmann::json Engine::PolygonCollider::Serialize()
{
    nlohmann::json data = Collider::Serialize();

    // The shape itself is deliberately absent: it belongs to the image, and baking a
    // copy into every level and prefab that uses the sprite means re-importing the art
    // no longer changes the collider.
    data["Shape Source"] = _spritePath;
    data["Shape Scale"] = _scale;
    return data;
}

void Engine::PolygonCollider::Deserialize(const json& data)
{
    Collider::Deserialize(data);

    if (data.contains("Shape Source"))
        _spritePath = data["Shape Source"].get<std::string>();

    if (data.contains("Shape Scale"))
        _scale = data["Shape Scale"];

    _cacheValid = false;
}

void Engine::PolygonCollider::Write(NetCode::OutputByteStream& stream) const
{
    Collider::Write(stream);

    stream.Write(_spritePath);
    stream.Write(_scale);
}

void Engine::PolygonCollider::Read(NetCode::InputByteStream& stream)
{
    Collider::Read(stream);

    stream.Read(_spritePath);
    stream.Read(_scale);

    // The path is what travels; the shape is looked up from the receiving machine's own
    // copy of the asset, exactly as it is on the authority.
    ReloadShape();
}
