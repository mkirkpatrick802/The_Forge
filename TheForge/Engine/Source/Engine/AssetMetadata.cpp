#include "AssetMetadata.h"

#include <filesystem>
#include <fstream>

#include "System.h"
#include "json.hpp"
#include "stb_image.h"

namespace
{
	constexpr const char* METADATA_EXTENSION = ".meta";

	// Bumped when the sidecar layout changes, so a stale file can be regenerated
	// rather than silently misread.
	//
	// 2 added the traced collision shape. Sidecars written before it are ignored, so
	// every image wants a re-import -- Content Drawer, "Reimport All".
	constexpr int METADATA_VERSION = 2;

	constexpr const char* KEY_VERSION = "Version";
	constexpr const char* KEY_WIDTH = "Width";
	constexpr const char* KEY_HEIGHT = "Height";
	constexpr const char* KEY_COLLISION_SHAPE = "Collision Shape";

	std::string MetadataPathFor(const std::string& assetPath)
	{
		return assetPath + METADATA_EXTENSION;
	}

	// Reads width/height straight from the image header. No decode, no upload.
	bool ReadImageHeader(const std::string& imagePath, int& outWidth, int& outHeight)
	{
		int channels;
		return stbi_info(imagePath.c_str(), &outWidth, &outHeight, &channels) != 0;
	}
}

bool Engine::ImportImageAsset(const std::string& imagePath)
{
	int width, height;
	if (!ReadImageHeader(imagePath, width, height))
	{
		DEBUG_LOG("Asset import failed, could not read image header: %s", imagePath.c_str())
		return false;
	}

	nlohmann::json data;
	data[KEY_VERSION] = METADATA_VERSION;
	data[KEY_WIDTH] = width;
	data[KEY_HEIGHT] = height;

	// The one genuinely expensive part of an import, and the reason it is done here
	// rather than on load: this decodes the image and walks every pixel.
	data[KEY_COLLISION_SHAPE] = BuildSpriteOutline(imagePath).Serialize();

	std::ofstream file(MetadataPathFor(imagePath));
	if (!file.is_open())
	{
		DEBUG_LOG("Asset import failed, could not write metadata for: %s", imagePath.c_str())
		return false;
	}

	file << data.dump(4);
	return true;
}

bool Engine::EnsureImageAssetImported(const std::string& imagePath)
{
	if (std::filesystem::exists(MetadataPathFor(imagePath)))
		return true;

	return ImportImageAsset(imagePath);
}

glm::vec2 Engine::GetImageSize(const std::string& imagePath)
{
	if (std::ifstream file(MetadataPathFor(imagePath)); file.is_open())
	{
		try
		{
			nlohmann::json data;
			file >> data;

			if (data.contains(KEY_VERSION) && data[KEY_VERSION] == METADATA_VERSION &&
				data.contains(KEY_WIDTH) && data.contains(KEY_HEIGHT))
			{
				return { data[KEY_WIDTH].get<float>(), data[KEY_HEIGHT].get<float>() };
			}
		}
		catch (const std::exception&)
		{
			// Fall through to the header read below.
		}
	}

	// No usable sidecar. Reading the header still avoids decoding the image, but it
	// means opening a file a server should not need -- so say so.
	int width, height;
	if (ReadImageHeader(imagePath, width, height))
	{
		DEBUG_LOG("No import metadata for '%s'; read its header instead. Re-import it.", imagePath.c_str())
		return { static_cast<float>(width), static_cast<float>(height) };
	}

	DEBUG_LOG("Could not determine size of image: %s", imagePath.c_str())
	return { 0.0f, 0.0f };
}

Engine::SpriteOutline Engine::GetImageCollisionShape(const std::string& imagePath)
{
	std::ifstream file(MetadataPathFor(imagePath));
	if (!file.is_open()) return {};

	try
	{
		nlohmann::json data;
		file >> data;

		if (!data.contains(KEY_VERSION) || data[KEY_VERSION] != METADATA_VERSION) return {};
		if (!data.contains(KEY_COLLISION_SHAPE)) return {};

		return SpriteOutline::Deserialize(data[KEY_COLLISION_SHAPE]);
	}
	catch (const std::exception&)
	{
		return {};
	}
}
