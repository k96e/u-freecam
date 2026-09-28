#pragma once

#include "umod/utype/utype.hpp"

#include <optional>
#include <utility>

namespace umod::UTYPE::unity_engine
{
    using Vector2 = UnityResolve::UnityType::Vector2;
    using Vector3 = UnityResolve::UnityType::Vector3;
    using Vector4 = UnityResolve::UnityType::Vector4;
    using Quaternion = UnityResolve::UnityType::Quaternion;
    using String = UnityResolve::UnityType::String;
    using Screen = UnityResolve::UnityType::Screen;
    using Color = UnityResolve::UnityType::Color;
    using Rect = UnityResolve::UnityType::Rect;
    using Ray = UnityResolve::UnityType::Ray;
    using Matrix4x4 = UnityResolve::UnityType::Matrix4x4;

    using UnityObject = UnityResolve::UnityType::UnityObject;

    enum class Space
    {
        World = 0x0,
        Self = 0x1
    };

    class Transform : public UTYPE::Transform
    {
        UNITY_CLASS_DECL("UnityEngine.CoreModule.dll", Transform)
    public:
        UNITY_METHOD(Transform *, GetParent, (void))
        UNITY_METHOD(void, SetParent, (UTYPE::Transform *const parent, bool worldPositionStays = true), parent,
                     worldPositionStays)
        inline auto GetChild(const int index) -> Transform *
        {
            return static_cast<Transform *>(UTYPE::Transform::GetChild(index));
        }

    public:
        inline auto Rotate(Vector3 axis, float angle, Space relativeTo) -> void
        {

            static UMethod *method;
            if (!method)
                method =
                    GetUClass()->Get<UMethod>("Rotate", {"UnityEngine.Vector3", "System.Single", "UnityEngine.Space"});
            return method->Invoke<void>(this, axis, angle, relativeTo);
        }
    };

    class GameObject : public UTYPE::GameObject
    {
        UNITY_CLASS_DECL("UnityEngine.CoreModule.dll", GameObject)
    public:
        inline static auto Create(const std::string &name) -> GameObject *
        {
            return static_cast<GameObject *>(UTYPE::GameObject::Create(name));
        }
        inline auto GetTransform() -> Transform *
        {
            return static_cast<Transform *>(UTYPE::GameObject::GetTransform());
        }
        inline auto IsDestoryed() -> bool
        {
            static UMethod *method;
            if (!method)
                method = UnityResolve::Get("UnityEngine.CoreModule.dll")->Get("Object")->Get<UMethod>("op_Equality");
            return method->Invoke<bool>(this, nullptr);
        }
        inline auto SetName(const std::string &name) -> void
        {
            static UMethod *const method =
                UnityResolve::Get("UnityEngine.CoreModule.dll")->Get("Object")->Get<UMethod>("set_name");
            if (method) method->Invoke<void>(this, String::New(name));
        }
    };

    class Component : public UTYPE::Component
    {
        UNITY_CLASS_DECL("UnityEngine.CoreModule.dll", Component)
    };

    class MonoBehaviour : public UTYPE::MonoBehaviour
    {
        UNITY_CLASS_DECL("UnityEngine.CoreModule.dll", MonoBehaviour)
    };

    class Camera : public UTYPE::Camera
    {
        UNITY_CLASS_DECL("UnityEngine.CoreModule.dll", Camera)
    public:
        enum class CameraClearFlags
        {
            Skybox = 1,
            Color,
            SolidColor = 2,
            Depth,
            Nothing
        };
        inline static auto GetMain() -> Camera * { return static_cast<Camera *>(UTYPE::Camera::GetMain()); }
        inline auto GetGameObject() -> GameObject *
        {
            return static_cast<GameObject *>(UTYPE::Camera::GetGameObject());
        }
        UNITY_METHOD(CameraClearFlags, get_clearFlags, ())
        UNITY_METHOD(void, set_clearFlags, (CameraClearFlags flags), flags)
        UNITY_METHOD(bool, get_orthographic, ())
        UNITY_METHOD(void, set_orthographic, (bool v), v)
        UNITY_METHOD(float, get_orthographicSize, ())
        UNITY_METHOD(void, set_orthographicSize, (float size), size)

        // Used by the stereo output. IL2CPP builds strip the ones a game never touches, so these report a missing
        // method instead of crashing on it.
        inline auto SetRect(const Rect &rect) -> bool
        {
            static UMethod *const method = FindStructSetter(GetUClass(), "rect");
            return InvokeStructSetter(method, this, rect);
        }
        inline auto SetProjectionMatrix(const Matrix4x4 &matrix) -> bool
        {
            static UMethod *const method = FindStructSetter(GetUClass(), "projectionMatrix");
            return InvokeStructSetter(method, this, matrix);
        }
        inline auto SetAspect(const float aspect) -> bool
        {
            static UMethod *const method = GetUClass()->Get<UMethod>("set_aspect");
            if (!method) return false;
            method->Invoke<void>(this, aspect);
            return true;
        }
        inline auto GetClipPlanes() -> std::optional<std::pair<float, float>>
        {
            static UMethod *const nearMethod = GetUClass()->Get<UMethod>("get_nearClipPlane");
            static UMethod *const farMethod = GetUClass()->Get<UMethod>("get_farClipPlane");
            if (!nearMethod || !farMethod) return std::nullopt;
            return std::pair{nearMethod->Invoke<float>(this), farMethod->Invoke<float>(this)};
        }
        inline auto IsOrthographic() -> bool
        {
            static UMethod *const method = GetUClass()->Get<UMethod>("get_orthographic");
            return method && method->Invoke<bool>(this);
        }
    };

    // struct Quaternion : public UTYPE::Quaternion
    // {
    //     UNITY_CLASS_DECL("UnityEngine.CoreModule.dll", Quaternion)
    // public:
    //     inline static auto AngleAxis(float angle, Vector3 axis) -> Quaternion
    //     {
    //         static UMethod *method;
    //         if (!method) method = GetUClass()->Get<UMethod>("AngleAxis", {"System.Single", "UnityEngine.Vector3"});
    //         return method->Invoke<Quaternion>(angle, axis);
    //     }
    // };

    class RectTransform : public Transform
    {
        UNITY_CLASS_DECL("UnityEngine.CoreModule.dll", RectTransform)
        UNITY_METHOD(UTYPE::Vector2, get_anchorMax, ())
        UNITY_METHOD(void, set_anchorMax, (const UTYPE::Vector2 &anchor), anchor)
        UNITY_METHOD(UTYPE::Vector2, get_anchorMin, ())
        UNITY_METHOD(void, set_anchorMin, (const UTYPE::Vector2 &anchor), anchor)
        UNITY_METHOD(UTYPE::Vector2, get_offsetMax, ())
        UNITY_METHOD(void, set_offsetMax, (const UTYPE::Vector2 &offset), offset)
        UNITY_METHOD(UTYPE::Vector2, get_offsetMin, ())
        UNITY_METHOD(void, set_offsetMin, (const UTYPE::Vector2 &offset), offset)
        UNITY_METHOD(UTYPE::Vector2, get_sizeDelta, ())
        UNITY_METHOD(void, set_sizeDelta, (const UTYPE::Vector2 &delta), delta)
        UNITY_METHOD(UTYPE::Vector2, get_anchoredPosition, ())
        UNITY_METHOD(void, set_anchoredPosition, (const UTYPE::Vector2 &anchor), anchor)
        UNITY_METHOD(UTYPE::Vector2, get_pivot, ())
        UNITY_METHOD(void, set_pivot, (const UTYPE::Vector2 &pivot), pivot)
    };
    class Material : public UTYPE::UnityObject
    {
        UNITY_CLASS_DECL("UnityEngine.CoreModule.dll", Material)
    };
    class Shader : public UTYPE::UnityObject
    {
        UNITY_CLASS_DECL("UnityEngine.CoreModule.dll", Shader)
        UNITY_STATIC_METHOD(Shader *, Find, (UTYPE::String * shader), shader)
    };
}
