#pragma once
#include "ReflectionUtils.h"

#define REFLECT() \
public: \
static ReflectionInfo* GetReflectionInfo(); \
virtual ReflectionInfo* GetInstanceReflectionInfo() const { return GetReflectionInfo(); }

#define REPLICATE()
