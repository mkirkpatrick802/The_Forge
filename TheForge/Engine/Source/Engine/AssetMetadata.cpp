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
	constexpr int METADATA_VERSION = 1;

	constexpr const char* KEY_VERSION = "Version";
	constexpr const char* KEY_WIDTH = "Width";
	constexpr const char* KEY_HEIGHT = "Height";

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
