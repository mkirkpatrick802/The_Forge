#pragma once
#include <string>
#include <glm/vec2.hpp>

namespace Engine
{
	// Per-asset data produced when an asset is imported and stored in a sidecar file
	// next to it ("Sprites/logo.png" -> "Sprites/logo.png.meta").
	//
	// The point is that anything the engine needs to know *about* an asset is
	// available without opening the asset itself. A dedicated server can size a
	// sprite without decoding -- or even shipping -- the image, which is what keeps
	// level loading free of any GPU or image work. It is also the natural place to
	// put future import-time analysis: pivots, sheet slicing, filter mode,
	// alpha-derived bounds.

	// Analyses an image and writes its sidecar, overwriting any existing one.
	// Call when an asset is imported or re-imported.
	bool ImportImageAsset(const std::string& imagePath);

	// Writes a sidecar only if one is missing. Cheap to call over a whole folder.
	bool EnsureImageAssetImported(const std::string& imagePath);

	// An image's dimensions without decoding or uploading it. Prefers the sidecar;
	// falls back to reading the file header (and warns) so assets that predate the
	// import step still work. Returns (0,0) if neither is available.
	glm::vec2 GetImageSize(const std::string& imagePath);
}
