#include "freecam/helper.hpp"
#include "freecam/freecam.hpp"

#include "umod/debug/logger.hpp"
#include "umod/utype/unity_engine/core.hpp"
#include "umod/utype/unity_engine/physics.hpp"
#include "user/config.hpp"

using namespace umod::UTYPE::unity_engine;
using namespace umod::debug;

namespace freecam::helper
{
    auto isDestroyed(UnityObject *obj) -> bool
    {
        using umod::UTYPE::UMethod;
        static UMethod *const method =
            UnityResolve::Get("UnityEngine.CoreModule.dll")->Get("Object")->Get<UMethod>("op_Equality");
        return !obj || method->Invoke<bool>(obj, nullptr);
    }

    auto selectGameObject(Transform *view) -> Transform *
    {
        // Cast along the view axis rather than through the screen center, which stereo output splits across two eyes
        Ray ray{};
        ray.m_vOrigin = view->GetPosition();
        ray.m_vDirection = view->GetForward();
        const auto hit = std::make_unique<RaycastHit>();
        if (Physics::Raycast(ray, &*hit, 100.f))
        {
            const auto target = hit->get_collider()->GetGameObject();
            // if (const auto animator = target->GetComponentInChildren<UTYPE::Animator *>(UTYPE::Animator::GetUClass()))
            // {
            //     logger::debug("Found Animator");
            //     if (const auto head = animator->GetBoneTransform(Animator::HumanBodyBones::Head))
            //     {
            //         logger::debug("Found Head");
            //         return head;
            //     }
            // }
            return static_cast<Transform *>(target->GetTransform());
        }
        return nullptr;
    }

    auto getMaxDepthCamera() -> Camera *
    {
        Camera *max{};
        for (const auto cam : Camera::GetAllCamera())
        {
            if (!max)
                max = static_cast<Camera *>(cam);
            else if (max->GetDepth() < cam->GetDepth())
                max = static_cast<Camera *>(cam);
        }
        return max;
    }

    bool isCurrentFreeCamera()
    {
        const auto curCam = user_config::freecam::Mode == Mode::Depth ? getMaxDepthCamera() : Camera::GetMain();
        if (curCam == nullptr) return false;
        const auto curCamName = curCam->GetName()->ToString();
        if (curCamName == kFreeCameraName) return true;

        logger::debug("Not FreeCamera: {}", curCamName);
        // NOTE: This must be `Camera -> GameObject` -> Tag and not `Camera -> Tag`，otherwise wired bug can happen in some games.
        logger::debug("Tag: {}", curCam->GetGameObject()->GetTag()->ToString());
        return false;
    }
}
