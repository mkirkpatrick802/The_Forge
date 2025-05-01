#pragma once

#define HASH_COMPONENT(NAME) (static_cast<uint32_t>(std::hash<std::string>{}(NAME)))