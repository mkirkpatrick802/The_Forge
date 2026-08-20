#include "UIText.h"

#include <algorithm>
#include <cmath>

#include "UIResources.h"
#include "Engine/System.h"
#include "Engine/Rendering/Font.h"

namespace
{
    // FreeType is asked for an atlas at the size the text will actually occupy on
    // screen, so a resize does not reuse a stale one. Bounded because the size is
    // derived from the window: a wildly stretched window must not ask for a 4000-pixel
    // atlas, and a collapsed one must not ask for a zero-pixel atlas whose glyphs are
    // all empty.
    constexpr unsigned int MIN_ATLAS_SIZE = 6;
    constexpr unsigned int MAX_ATLAS_SIZE = 192;
}

Engine::UIText::UIText(const std::string& text, const unsigned int fontSize)
    : _text(text), _fontSize(fontSize)
{
}

void Engine::UIText::SetFontSize(const unsigned int fontSize)
{
    if (_fontSize == fontSize) return;

    _fontSize = fontSize;
    InvalidateResources();
}

void Engine::UIText::SetFontPath(const std::string& fontPath)
{
    if (_fontPath == fontPath) return;

    _fontPath = fontPath;
    InvalidateResources();
}

void Engine::UIText::SetWrap(const bool wrap)
{
    if (_wrap == wrap) return;

    _wrap = wrap;
    _lineCacheWidth = -1.0f;   // force a rebuild on the next draw
}

const std::vector<std::string>& Engine::UIText::ResolveLines() const
{
    const float maxWidth = _rect.size.x;

    // Rebuilt only when an input changed. Wrapping measures every word, and Draw runs
    // every frame -- a log feed of ten lines would otherwise re-measure a few hundred
    // words per frame to produce the same answer it had last time.
    if (_lineCacheText == _text && _lineCacheWidth == maxWidth && _lineCacheAtlas == _atlasSize)
        return _lines;

    _lineCacheText = _text;
    _lineCacheWidth = maxWidth;
    _lineCacheAtlas = _atlasSize;
    _lines.clear();

    if (!_wrap || maxWidth <= 0.0f || _font == nullptr)
    {
        _lines.push_back(_text);
        return _lines;
    }

    const float scale = GetDrawScale();

    std::string current;
    size_t wordStart = 0;

    // Split on spaces and fit whole words. A word longer than the whole line is left to
    // overflow rather than being broken mid-token: the long strings this exists for are
    // addresses and file paths, and cutting one in half makes it unreadable in exactly
    // the case where reading it matters.
    while (wordStart <= _text.size())
    {
        size_t wordEnd = _text.find(' ', wordStart);
        if (wordEnd == std::string::npos) wordEnd = _text.size();

        const std::string word = _text.substr(wordStart, wordEnd - wordStart);
        const std::string candidate = current.empty() ? word : current + " " + word;

        if (!current.empty() && _font->MeasureText(candidate, scale).x > maxWidth)
        {
            _lines.push_back(current);
            current = word;
        }
        else
        {
            current = candidate;
        }

        if (wordEnd == _text.size()) break;
        wordStart = wordEnd + 1;
    }

    if (!current.empty() || _lines.empty())
        _lines.push_back(current);

    return _lines;
}

void Engine::UIText::InvalidateResources()
{
    UIElement::InvalidateResources();

    // Only dropped, never deleted -- UIResources owns every font and hands out
    // borrowed pointers.
    _font = nullptr;
    _atlasSize = 0;

    // The wrap was measured against the old atlas, so it has to be measured again.
    _lineCacheWidth = -1.0f;
}

void Engine::UIText::ResolveFont(const glm::vec2& canvasSize)
{
    // Label size is authored in canvas units, but a glyph atlas is rasterised in real
    // pixels. Rasterising at the canvas size and letting the quad be magnified to fit
    // the window is what made the first version of this blurry -- a 16-unit label on a
    // 1080-tall window was a 16-pixel atlas stretched to roughly 96 pixels.
    //
    // So the atlas is built at the size the text will occupy on screen, and the glyph
    // quads are scaled back down into canvas units. Text stays crisp at any window size,
    // and the cost is one atlas per distinct on-screen size rather than per label.
    const glm::vec2 windowSize = GetAppWindowSize();

    float pixelsPerUnit = 1.0f;
    if (canvasSize.y > 0.0f && windowSize.y > 0.0f)
        pixelsPerUnit = windowSize.y / canvasSize.y;

    const auto wanted = static_cast<unsigned int>(std::lround(static_cast<float>(_fontSize) * pixelsPerUnit));
    const unsigned int atlasSize = std::clamp(wanted, MIN_ATLAS_SIZE, MAX_ATLAS_SIZE);

    if (_font != nullptr && _atlasSize == atlasSize) return;

    _font = UIResources::GetFont(_fontPath.empty() ? UI_DEFAULT_FONT : _fontPath, atlasSize);
    _atlasSize = atlasSize;
}

float Engine::UIText::GetDrawScale() const
{
    if (_atlasSize == 0) return 1.0f;

    // Turns the atlas's pixel metrics back into canvas units, so a label authored as
    // "16 units tall" is 16 units tall whatever size it was rasterised at.
    return static_cast<float>(_fontSize) / static_cast<float>(_atlasSize);
}

glm::vec2 Engine::UIText::Measure() const
{
    if (!_font) return glm::vec2(0.0f);

    const float scale = GetDrawScale();

    if (!_wrap)
        return _font->MeasureText(_text, scale);

    const std::vector<std::string>& lines = ResolveLines();

    // The widest line, and the height of all of them stacked -- so a caller laying out
    // around a wrapped block gets the block's size and not the first line's.
    glm::vec2 size(0.0f);
    for (const std::string& line : lines)
    {
        // Not std::max: Windows.h is pulled in through System.h and defines max() as a
        // macro, which turns any qualified call into a syntax error.
        const float width = _font->MeasureText(line, scale).x;
        if (width > size.x) size.x = width;
    }

    const float lineHeight = static_cast<float>(_fontSize) * _lineSpacing;
    size.y = lineHeight * static_cast<float>(lines.size());

    return size;
}

void Engine::UIText::Draw(const glm::mat4& projection, const glm::vec2& canvasSize)
{
    if (!IsVisible() || _text.empty()) return;

    // Resolved here rather than in EnsureResourcesResident because the right atlas size
    // depends on the canvas, and only the draw call is told what the canvas is.
    ResolveFont(canvasSize);
    if (_font == nullptr) return;

    const float drawScale = GetDrawScale();

    // The rect's own size is kept up to date from the measurement, so an author who
    // never set one still gets a sensible box to hit-test and to align against.
    const glm::vec2 measured = Measure();
    if (_rect.size.x <= 0.0f) _rect.size.x = measured.x;
    if (_rect.size.y <= 0.0f) _rect.size.y = measured.y;

    const glm::vec2 origin = _rect.ResolvePosition(canvasSize);

    // ResolvePosition gives the element's top-left, but RenderText takes a baseline.
    // Without the ascent every label would sit one cap-height too high, which reads as
    // "the anchor is wrong" rather than "the origin convention differs".
    const float baseline = origin.y + _font->GetAscent(drawScale);
    const float lineHeight = static_cast<float>(_fontSize) * _lineSpacing;

    const std::vector<std::string>& lines = ResolveLines();

    for (size_t i = 0; i < lines.size(); ++i)
    {
        // Alignment is per line, against the rect -- centring a wrapped paragraph means
        // centring each of its lines, not centring the block and left-aligning inside it.
        const float lineWidth = _font->MeasureText(lines[i], drawScale).x;

        float x = origin.x;
        switch (_align)
        {
        case EUITextAlign::Center: x += (_rect.size.x - lineWidth) * 0.5f; break;
        case EUITextAlign::Right:  x += _rect.size.x - lineWidth; break;
        default: break;
        }

        const glm::vec2 position(x, baseline + static_cast<float>(i) * lineHeight);

        _font->RenderText(lines[i], position, drawScale, _color * _tint, projection, _opacity);
    }
}
