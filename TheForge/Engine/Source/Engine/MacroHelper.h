#pragma once
#include <string>
#include <typeinfo>
#ifdef __GNUG__
#include <cxxabi.h>
#endif

namespace Engine
{
    String ExtractClassName(const String& mangledName)
    {
    #ifdef __GNUG__
        int status = 0;
        char* demangledName = abi::__cxa_demangle(mangledName.c_str(), nullptr, nullptr, &status);
        String result = (status == 0) ? demangledName : mangledName;
        free(demangledName);
        return result;
    #else
        return mangledName; // On platforms like MSVC, typeid().name() is usually clean already
    #endif
    }
}