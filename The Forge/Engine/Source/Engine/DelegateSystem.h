#pragma once
#include <functional>

#define DECLARE_DELEGATE(DelegateName) \
class DelegateName { \
    public: \
        using DelegateFunctionType = std::function<void()>; \
        template <typename T> \
        void Bind(T* instance, void (T::*func)()) { \
            m_Functions.push_back([instance, func]() { (instance->*func)(); }); \
        } \
        template <typename T> \
        void Bind(const T* instance, void (T::*func)() const) { \
            m_Functions.push_back([instance, func]() { (instance->*func)(); }); \
        } \
        void Broadcast() const { \
            for (const auto& func : m_Functions) { \
                if (func) { \
                    func(); \
                } \
            } \
        } \
    private: \
        std::vector<DelegateFunctionType> m_Functions; \
};

#define DECLARE_DELEGATE_ONE_PARAM(DelegateName, ParamType, ParamName) \
class DelegateName { \
    public: \
        using DelegateFunctionType = std::function<void(ParamType)>; \
        template <typename T> \
        void Bind(T* instance, void (T::*func)(ParamType)) { \
            m_Functions.push_back([instance, func](ParamType ParamName) { (instance->*func)(ParamName); }); \
        } \
        template <typename T> \
        void Bind(T* instance, void (T::*func)(const ParamType)) { \
            m_Functions.push_back([instance, func](ParamType ParamName) { (instance->*func)(ParamName); }); \
        } \
        template <typename T> \
        void Bind(const T* instance, void (T::*func)(ParamType) const) { \
            m_Functions.push_back([instance, func](ParamType ParamName) { (instance->*func)(ParamName); }); \
        } \
        template <typename T> \
        void Bind(const T* instance, void (T::*func)(const ParamType) const) { \
            m_Functions.push_back([instance, func](ParamType ParamName) { (instance->*func)(ParamName); }); \
        } \
        void Broadcast(ParamType ParamName) const { \
            for (const auto& func : m_Functions) { \
                if (func) { \
                    func(ParamName); \
                } \
            } \
        } \
    private: \
        std::vector<DelegateFunctionType> m_Functions; \
};