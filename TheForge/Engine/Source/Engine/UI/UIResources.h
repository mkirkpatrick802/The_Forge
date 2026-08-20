#pragma once
#include <map>
#include <memory>
#include <string>

#include "Engine/Rendering/Shader.h"

namespace Engine
{
    class Font;
    class Texture;

    // GPU resources every UI element shares.
    //
    // Held once rather than per element for two reasons. A Shader is a compiled GL
    // program, and one per UIImage means recompiling the same sprite shader for every
    // button on a menu. A Font is worse: it uploads a texture per glyph, so two labels at
    // the same size would build two complete copies of the ASCII range.
    //
    // Nothing here is created until the first draw. That is the same contract
    // IRenderable::EnsureResourcesResident defines, and it is what lets a headless
    // dedicated server -- which has no GL context at all -- run without ever tripping
    // over a UI type.
    class UIResources
    {
    public:
        // The unit quad every UI image is drawn with, in the layout the sprite shader
        // expects. Built on first use.
        static unsigned int GetQuadVAO();

        // The stock sprite shader, compiled on first use. Shared, so callers must set
        // every uniform they depend on rather than assuming what the last draw left.
        static Shader& GetSpriteShader();

        // A single white pixel, built on first use.
        //
        // What lets a UIImage with no sprite draw as a solid, tinted rectangle: panels,
        // progress bars and button backgrounds are all just this stretched and tinted.
        // The alternative -- a second shader and a second draw path for untextured
        // quads -- buys nothing, because the sprite shader multiplies by the tint
        // already.
        static Texture* GetWhiteTexture();

        // A font at a given size, built on first request and kept. The path is the
        // engine's own Consolas unless a caller asks for another.
        static Font* GetFont(const std::string& fontPath, unsigned int fontSize);

        // Releases everything. Called from the renderer's shutdown, while the GL context
        // is still alive -- a GL object deleted after the context is gone is a crash on
        // some drivers and a silent leak on the rest.
        static void CleanUp();

    private:
        static unsigned int _quadVAO;
        static std::unique_ptr<Shader> _spriteShader;
        static std::unique_ptr<Texture> _whiteTexture;

        // Keyed by path *and* size, because Font::SetFontSize rebuilds every glyph
        // texture in place -- sharing one Font between a title and a caption would have
        // each of them resizing it out from under the other, every frame.
        static std::map<std::pair<std::string, unsigned int>, std::unique_ptr<Font>> _fonts;
    };

    // The engine's stock UI font. Shipped in the engine's own asset folder and copied
    // next to the game by the build, so it is always present.
    inline const std::string UI_DEFAULT_FONT = "Assets/Engine Assets/Fonts/Consolas.ttf";

    inline const std::string UI_SPRITE_VERTEX_SHADER = "Assets/Engine Assets/Shaders/Sprite.vert";
    inline const std::string UI_SPRITE_FRAGMENT_SHADER = "Assets/Engine Assets/Shaders/Sprite.frag";
}
