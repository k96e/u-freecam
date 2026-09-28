#include "freecam/freecam.hpp"
#include "freecam/helper.hpp"

#include "umod/debug/logger.hpp"
#include "umod/runtime/helper/input.hpp"
#include "umod/runtime/helper/time.hpp"
#include "umod/utils/math.hpp"

#include "user/config.hpp"

#include <cmath>
#include <numbers>

using namespace umod::UTYPE::unity_engine;
using namespace umod::unity_runtime::helper;
using namespace umod::debug;
using namespace umod::utils;

using namespace user_config::freecam;

namespace freecam
{
    namespace
    {
        constexpr float kMinSeparation = 0.001f; // Hotkeys scale geometrically, so they need a non-zero start
        constexpr float kMaxSeparation = 100.f;
        constexpr float kMinConvergence = 0.01f;
        constexpr float kMaxConvergence = 10000.f;
        constexpr float kAdjustRate = 0.7f; // Roughly doubles or halves per second
        constexpr float kRad2Deg = 180.f / std::numbers::pi_v<float>;

        // Off-axis perspective projection, in the OpenGL convention Camera.projectionMatrix takes. The frustum of an
        // eye displaced by `eyeOffset` along camera x is sheared so both eyes' frustums coincide at `convergence`,
        // which puts that distance at zero parallax without the vertical disparity of toeing the eyes in.
        static auto offAxisPerspective(float fovY, float aspect, float nearPlane, float farPlane, float eyeOffset,
                                       float convergence) -> Matrix4x4
        {
            const float f = 1.f / std::tan(fovY / kRad2Deg / 2.f);
            Matrix4x4 m{}; // Column major, m[column][row]
            m[0][0] = f / aspect;
            m[1][1] = f;
            m[2][0] = -m[0][0] * eyeOffset / convergence;
            m[2][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
            m[2][3] = -1.f;
            m[3][2] = -2.f * farPlane * nearPlane / (farPlane - nearPlane);
            return m;
        }

        static void warnOnce(bool &warned, const char *message)
        {
            if (warned) return;
            logger::warn(message);
            warned = true;
        }
    }

    auto FreeCamera::enableStereo() -> bool
    {
        // Clone the free camera so the eyes keep its components and settings. Both are cloned before either gets
        // parented, otherwise the second clone would contain the first.
        for (auto &eye : eyeCams_)
            eye = static_cast<Camera *>(UnityObject::Instantiate(freeCam_));
        if (!eyeCams_[0] || !eyeCams_[1])
        {
            logger::error("Failed to create eye cameras");
            disableStereo();
            return false;
        }

        const auto rigTrans = static_cast<Transform *>(freeCam_->GetTransform());
        constexpr const char *kEyeNames[] = {kFreeCameraLeftName, kFreeCameraRightName};
        for (size_t i = 0; i < eyeCams_.size(); ++i)
        {
            const auto eye = eyeCams_[i];
            eye->GetGameObject()->SetName(kEyeNames[i]);
            const auto eyeTrans = static_cast<Transform *>(eye->GetTransform());
            eyeTrans->SetParent(rigTrans, false);
            eyeTrans->SetLocalPosition(Vector3(0, 0, 0));
            eyeTrans->SetLocalRotation(Quaternion(0, 0, 0, 1));
            eyeTrans->SetLocalScale(Vector3(1, 1, 1));
            eye->SetEnabled(true);
        }
        freeCam_->SetEnabled(false);
        logger::info("Stereo output enabled");
        return true;
    }

    auto FreeCamera::disableStereo() -> void
    {
        for (auto &eye : eyeCams_)
        {
            if (eye && !helper::isDestroyed(eye)) GameObject::Destroy(eye->GetGameObject());
            eye = nullptr;
        }
        if (!helper::isDestroyed(freeCam_)) freeCam_->SetEnabled(true);
        logger::info("Stereo output disabled");
    }

    auto FreeCamera::applyStereo() -> bool
    {
        static bool warnedAspect = false;
        static bool warnedProjection = false;

        const auto width = Screen::get_width();
        const auto height = Screen::get_height();
        if (width <= 0 || height <= 0) return true;

        const float screenAspect = static_cast<float>(width) / static_cast<float>(height);
        // Half SBS squeezes a full frame into each half, Full SBS gives each half its own undistorted frame
        const float aspect = stereo::Format == StereoFormat::HalfSBS ? screenAspect : screenAspect / 2.f;
        const float fov = freeCam_->GetFoV();
        const float halfSeparation = math::clamp(stereo::Separation, 0.f, kMaxSeparation) / 2.f;
        const float convergence = math::clamp(stereo::Convergence, kMinConvergence, kMaxConvergence);

        // Cameras render without their transform scale, so undo the rig's (e.g. attached to a scaled object) to keep
        // the separation in world units
        float rigScale = static_cast<Transform *>(freeCam_->GetTransform())->GetLossyScale().x;
        if (math::abs(rigScale) < 1e-6f) rigScale = 1.f;

        for (size_t i = 0; i < eyeCams_.size(); ++i)
        {
            const auto eye = eyeCams_[i];
            const float eyeOffset = (i == 0 ? -1.f : 1.f) * halfSeparation;
            const bool leftHalf = (i == 0) != stereo::SwapEyes;

            if (!eye->SetRect(Rect(leftHalf ? 0.f : 0.5f, 0.f, 0.5f, 1.f)))
            {
                logger::error("Camera.rect is not available in this game, stereo output is unsupported");
                return false;
            }
            if (!eye->SetAspect(aspect))
                warnOnce(warnedAspect, "Camera.aspect is not available, image may be distorted");
            eye->SetFoV(fov);

            // An orthographic view has no frustum to shear, toe the eyes in towards the convergence point instead
            const bool orthographic = eye->IsOrthographic();
            const float toeIn = orthographic ? std::atan2(eyeOffset, convergence) * kRad2Deg : 0.f;
            const auto eyeTrans = static_cast<Transform *>(eye->GetTransform());
            eyeTrans->SetLocalPosition(Vector3(eyeOffset / rigScale, 0, 0));
            eyeTrans->SetLocalRotation(Quaternion().Euler(0.f, -toeIn, 0.f));
            if (orthographic) continue;

            const auto planes = eye->GetClipPlanes();
            if (!planes || !eye->SetProjectionMatrix(
                               offAxisPerspective(fov, aspect, planes->first, planes->second, eyeOffset, convergence)))
                warnOnce(warnedProjection, "Camera.projectionMatrix is not available, convergence has no effect");
        }
        return true;
    }

    auto FreeCamera::updateStereo() -> void
    {
        if (!stereo::Enabled && !eyeCams_[0]) return;
        if (helper::isDestroyed(freeCam_))
        {
            eyeCams_ = {};
            return;
        }

        // Follows the config, so toggling from the WebUI works the same as the hotkey
        const bool active = eyeCams_[0] != nullptr;
        if (stereo::Enabled != active)
        {
            if (active)
                disableStereo();
            else if (!enableStereo())
                stereo::Enabled = false;
        }
        if (!eyeCams_[0]) return;

        if (!applyStereo())
        {
            stereo::Enabled = false;
            disableStereo();
        }
    }

    auto FreeCamera::updateStereoAdjust() -> void
    {
        if (InputUtils::GetKeyDown(keybind::ToggleStereo)) stereo::Enabled = !stereo::Enabled;
        if (!eyeCams_[0]) return;

        auto deltaTime = TimeUtils::getDeltaTime_s();
        if (deltaTime == 0) deltaTime = to_seconds(user_config::core::MockLoopDeltaTime);
        // Scale geometrically so the same keys suit whatever unit the game world uses
        const float rate = InputUtils::GetKey(keybind::SpeedUp) ? kAdjustRate * 3 : kAdjustRate;
        const float step = std::exp(rate * deltaTime);

        bool adjusting = false;
        const auto adjust = [&](float &value, const float factor, const float lower, const float upper)
        {
            value = math::clamp(math::clamp(value, lower, upper) * factor, lower, upper);
            adjusting = true;
        };
        if (InputUtils::GetKey(keybind::SeparationUp)) adjust(stereo::Separation, step, kMinSeparation, kMaxSeparation);
        if (InputUtils::GetKey(keybind::SeparationDown))
            adjust(stereo::Separation, 1.f / step, kMinSeparation, kMaxSeparation);
        if (InputUtils::GetKey(keybind::ConvergenceUp))
            adjust(stereo::Convergence, step, kMinConvergence, kMaxConvergence);
        if (InputUtils::GetKey(keybind::ConvergenceDown))
            adjust(stereo::Convergence, 1.f / step, kMinConvergence, kMaxConvergence);

        if (kFlags.stereo_adjusting && !adjusting)
            logger::info("Stereo separation: {:.4g}, convergence: {:.4g}", stereo::Separation, stereo::Convergence);
        kFlags.stereo_adjusting = adjusting;
    }
}
