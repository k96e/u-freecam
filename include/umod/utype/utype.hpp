#pragma once

#include "umod/runtime/UnityResolve.hpp"
#include "umod/runtime/backend.hpp"

namespace umod::UTYPE
{
    using UClass = UnityResolve::Class;
    using UMethod = UnityResolve::Method;
    using UTYPE = UnityResolve::UnityType;

    // Setter of a struct-typed property. IL2CPP keeps the plain setter taking the struct by value, while Mono goes
    // through the injected icall (INTERNAL_ before Unity 2018.3) taking it by reference.
    inline auto FindStructSetter(UClass *klass, const std::string &property) -> UMethod *
    {
        if (!klass) return nullptr;
        if (unity_runtime::Backend == UnityResolve::Mode::Il2Cpp) return klass->Get<UMethod>("set_" + property);
        if (const auto method = klass->Get<UMethod>("set_" + property + "_Injected")) return method;
        return klass->Get<UMethod>("INTERNAL_set_" + property);
    }

    template <typename T>
    inline auto InvokeStructSetter(UMethod *method, UnityResolve::UnityType::UnityObject *self, const T &value) -> bool
    {
        if (!method) return false;
        if (unity_runtime::Backend == UnityResolve::Mode::Il2Cpp)
            method->Invoke<void>(self, value);
        else if (method->static_function) // Unity 6 injected icalls take the native object pointer
            method->Invoke<void>(self->m_CachedPtr, &value);
        else
            method->Invoke<void>(self, &value);
        return true;
    }
}

#define UNITY_CLASS_DECL(MODULE, CLS)                                                                                                                                                                  \
private:                                                                                                                                                                                               \
    inline static constexpr auto MODULE_NAME = MODULE;                                                                                                                                                 \
    inline static constexpr auto CLS_NAME = #CLS;                                                                                                                                                      \
                                                                                                                                                                                                       \
public:                                                                                                                                                                                                \
    inline static auto __ctor__(CLS *self) -> void                                                                                                                                                     \
    {                                                                                                                                                                                                  \
        static UMethod *method;                                                                                                                                                                        \
        if (!method) method = GetUClass()->Get<UMethod>(".ctor");                                                                                                                                      \
        return method->Invoke<void>(self);                                                                                                                                                             \
    }                                                                                                                                                                                                  \
    inline static auto GetUClass() -> UClass *                                                                                                                                                         \
    {                                                                                                                                                                                                  \
        static UClass *klass;                                                                                                                                                                          \
        if (!klass) klass = UnityResolve::Get(MODULE_NAME)->Get(CLS_NAME);                                                                                                                             \
        return klass;                                                                                                                                                                                  \
    }

// #define UNITY_FIELD(FIELD_TY, FIELD_NAME)                                                                                                                                                              \
//     auto Get##FIELD_NAME() -> FIELD_TY                                                                                                                                                                 \
//     {                                                                                                                                                                                                  \
//         static UMethod *method;                                                                                                                                                                        \
//         if (!method) method = GetUClass()->Get<UMethod>("get_collider");                                                                                                                               \
//         return method->Invoke<FIELD_TY>(this);                                                                                                                                                         \
//     }

#define UNITY_METHOD(RET_TY, METHOD_NAME, PARAMS, ...)                                                                                                                                                 \
    auto METHOD_NAME PARAMS->RET_TY                                                                                                                                                                    \
    {                                                                                                                                                                                                  \
        static UMethod *method;                                                                                                                                                                        \
        if (!method) method = GetUClass()->Get<UMethod>(#METHOD_NAME);                                                                                                                                 \
        return method->Invoke<RET_TY>(this __VA_OPT__(, ) __VA_ARGS__);                                                                                                                                \
    }
#define UNITY_STATIC_METHOD(RET_TY, METHOD_NAME, PARAMS, ...)                                                                                                                                          \
    inline static auto METHOD_NAME PARAMS->RET_TY                                                                                                                                                      \
    {                                                                                                                                                                                                  \
        static UMethod *method;                                                                                                                                                                        \
        if (!method) method = GetUClass()->Get<UMethod>(#METHOD_NAME);                                                                                                                                 \
        return method->Invoke<RET_TY>(__VA_ARGS__);                                                                                                                                                    \
    }
