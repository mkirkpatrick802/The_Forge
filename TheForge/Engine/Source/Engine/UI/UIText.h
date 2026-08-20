#pragma once
#include <string>
#include <vector>

#include "UIElement.h"

namespace Engine
{
    class Font;

    // How a label sits relative to its rect's horizontal extent.
    enum class EUITextAlign : uint8_t { Left = 0, Center, Right };

    // A line of text on the UI canvas.
    //
    // Distinct from the TextRenderer component, which is a world-space renderable tied
    // to a GameObject and hardcodes white, scale 1 and Consolas. This one carries its own
    // colour, size and alignment, and needs no GameObject -- the loading screen has to
    // draw text when there is no level loaded at all.
    //
    // The Font is shared through UIResources rather than owned, because a Font uploads
    // one texture per glyph: a label per line of a log feed, each with its own copy of
    // the ASCII range, is not a reasonable way to draw twelve lines of text.
    class UIText : public UIElement
    {
    public:
        UIText() = default;
        explicit UIText(const std::string& text, unsigned int fontSize = 16);

        void Draw(const glm::mat4& projection, const glm::vec2& canvasSize) override;
        void InvalidateResources() override;

        void SetText(const std::string& text) { _text = text; }
        const std::string& GetText() const { return _text; }

        // Rebuilds the glyph atlas at the new size on the next draw. No-op if unchanged,
        // so this is safe to call every frame.
        void SetFontSize(unsigned int fontSize);
        unsigned int GetFontSize() const { return _fontSize; }

        void SetFontPath(const std::string& fontPath);
        const std::string& GetFontPath() const { return _fontPath; }

        void SetColor(const glm::vec3& color) { _color = color; }
        const glm::vec3& GetColor() const { return _color; }

        void SetAlign(const EUITextAlign align) { _align = align; }
        EUITextAlign GetAlign() const { return _align; }

        // Breaks the text across lines so it fits the rect's width.
        //
        // Off by default, because most UI text is a label that is meant to be one line
        // and would rather overflow visibly than silently reflow. On for anything whose
        // length is not known when it is authored -- an error message naming a server,
        // a reason a connection was refused -- which would otherwise run off both edges
        // of the screen.
        //
        // Needs a rect width to wrap against; with none it behaves as though wrapping
        // were off.
        void SetWrap(bool wrap);
        bool IsWrapped() const { return _wrap; }

        // Gap between wrapped lines, as a multiple of the font size.
        void SetLineSpacing(const float spacing) { _lineSpacing = spacing; }

        // The size this label would occupy, in canvas units. Returns (0,0) until the
        // font has been resolved -- see PrepareForCanvas.
        glm::vec2 Measure() const;

        // Resolves the font for a given canvas without drawing.
        //
        // Needed because the atlas size depends on the canvas, so Measure cannot answer
        // anything before the first draw. A caller that has to lay out around a label's
        // size -- a button centring its own text -- would otherwise measure zero on the
        // first frame and visibly snap into place on the second.
        void PrepareForCanvas(const glm::vec2& canvasSize) { ResolveFont(canvasSize); }

    protected:
        // Picks the atlas size for the current window and canvas, and fetches the
        // matching font. Called from Draw, which is the first point that knows how big a
        // canvas unit is in real pixels.
        void ResolveFont(const glm::vec2& canvasSize);

        // Factor turning the atlas's pixel metrics into canvas units.
        float GetDrawScale() const;

        // Splits _text into lines that fit the rect width. Cached against the inputs it
        // depends on, because it measures every word and is called from Draw.
        const std::vector<std::string>& ResolveLines() const;

    protected:
        std::string _text;
        std::string _fontPath;
        unsigned int _fontSize = 16;
        glm::vec3 _color = glm::vec3(1.0f);
        EUITextAlign _align = EUITextAlign::Left;

        bool _wrap = false;
        float _lineSpacing = 1.15f;

        // Borrowed from UIResources, which owns it. Never deleted here.
        Font* _font = nullptr;

        // Wrapping is recomputed only when something it depends on changes -- the text,
        // the width, or the atlas the measurements came from.
        mutable std::vector<std::string> _lines;
        mutable std::string _lineCacheText;
        mutable float _lineCacheWidth = -1.0f;
        mutable unsigned int _lineCacheAtlas = 0;

        // The pixel size _font was rasterised at, which is not _fontSize: one is in
        // canvas units and the other in real pixels, and the ratio between them is what
        // GetDrawScale returns. Zero until the first draw.
        unsigned int _atlasSize = 0;
    };
}
