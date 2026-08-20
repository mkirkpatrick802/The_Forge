#pragma once
#include <memory>
#include <string>

#include "UIElement.h"

namespace Engine
{
    class Texture;

    // A textured quad on the UI canvas.
    //
    // The sprite path is set at any time, but the image itself is not opened until the
    // first draw. Sizing comes from the asset's import sidecar via
    // AssetMetadata::GetImageSize, which reads dimensions without decoding the file --
    // so setting a sprite costs no image work and no GL work, and a headless build can
    // hold one of these without ever having a context.
    class UIImage : public UIElement
    {
    public:
        // Both constructors and the destructor are declared here and defined in the
        // .cpp, none of them defaulted inline.
        //
        // Texture is only forward-declared above, and _texture is a unique_ptr to one.
        // A constructor defaulted *in the header* is instantiated at every call site,
        // and instantiating it requires being able to unwind it -- which needs
        // ~unique_ptr<Texture>, which needs a complete Texture. Every file that
        // default-constructs a UIImage would have to include Texture.h, which drags in
        // System.h and therefore Windows.h.
        UIImage();
        explicit UIImage(const std::string& spritePath);
        ~UIImage() override;

        void Draw(const glm::mat4& projection, const glm::vec2& canvasSize) override;
        void EnsureResourcesResident() override;
        void InvalidateResources() override;

        // Setting a sprite drops the current texture rather than loading the new one:
        // the upload belongs on the next draw, not here. Passing the path already set
        // does nothing, so calling this every frame is harmless.
        void SetSprite(const std::string& spritePath);
        const std::string& GetSprite() const { return _spritePath; }

        // Sizes the element to the image's own pixel dimensions. Only meaningful once a
        // sprite is set, and reads the sidecar rather than the image.
        void SizeToSprite();

        // Whether this image has a sprite of its own, as opposed to drawing as a solid
        // tinted rectangle.
        bool HasSprite() const { return !_spritePath.empty(); }

    protected:
        // Draws an arbitrary rect with this image's texture and tint. Shared with
        // subclasses that draw more than one box from a single sprite -- a slider's
        // track and fill, a button's background.
        void DrawQuad(const glm::mat4& projection, const glm::vec2& topLeft, const glm::vec2& size,
                      const glm::vec3& tint, float opacity) const;

        // The texture to draw with: this image's own, or the shared white pixel when it
        // has no sprite. An image with no sprite is therefore a solid, tinted rectangle
        // rather than nothing at all -- which is what panels, dividers and progress bars
        // are made of.
        const Texture* ResolveTexture() const;

        std::string _spritePath;
        std::unique_ptr<Texture> _texture;
    };
}
