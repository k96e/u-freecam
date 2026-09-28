#pragma once

#include "umod/runtime/helper/camera.hpp"
#include "umod/runtime/helper/transform.hpp"
#include "umod/utype/unity_engine/core.hpp"

#include <array>
#include <memory>

namespace freecam
{
    enum class Mode
    {
        Orignal,
        Depth,
        MainCamera
    };

    enum class StereoFormat
    {
        HalfSBS, // Each eye squeezed into half the width, for 3D TVs / glasses that stretch it back
        FullSBS  // Each eye keeps its own aspect, for free viewing or double-width displays
    };

    constexpr auto kFreeArchorName = "UE_Freecam_Archor";
    constexpr auto kFreeCameraName = "UE_Freecam";
    constexpr auto kFreeCameraLeftName = "UE_Freecam_L";
    constexpr auto kFreeCameraRightName = "UE_Freecam_R";

    class FreeCamera
    {
    protected:
        FreeCamera() = default;

    public:
        static FreeCamera create(Mode mode)
        {
            FreeCamera freeCam{};
            freeCam.mode = mode;
            return freeCam;
        }

    public:
        bool enabled = false;
        Mode mode;

    public:
        auto enable() -> void;
        auto disable() -> void;
        auto update() -> void;

    protected:
        using CameraHelper = umod::unity_runtime::helper::CameraHelper;
        using TransformHelper = umod::unity_runtime::helper::TransformHelper;

        struct
        {
            bool ui_layer = false;
            bool zoom_mode = false;
            bool attach_mode = false;
            bool stereo_adjusting = false;
        } kFlags;

        umod::UTYPE::unity_engine::Camera *freeCam_{};
        // Left and right eye cameras under freeCam_ while stereo output is on, freeCam_ itself stops rendering then
        std::array<umod::UTYPE::unity_engine::Camera *, 2> eyeCams_{};
        umod::UTYPE::unity_engine::Transform *anchorTrans_{};
        std::unique_ptr<CameraHelper> cameraHelper_;
        std::unique_ptr<TransformHelper> freeTransHelper_;
        std::unique_ptr<TransformHelper> anchorTransHelper_;

    protected:
        umod::UTYPE::unity_engine::Camera *origCamera_{};
        umod::UTYPE::unity_engine::Vector3 origPosition_{};
        umod::UTYPE::unity_engine::Quaternion origRotation_{};

        auto createCamera(Mode) -> umod::UTYPE::unity_engine::Camera *;
        auto backupOrigCamera() -> void;
        auto updateMove() -> void;
        auto updateRotate() -> void;
        auto updateRoll() -> void;
        auto updateZoom() -> void;
        auto updateAttachMode() -> void;
        auto updateStereoAdjust() -> void;
        auto updateStereo() -> void;

        auto enterAttachMode(umod::UTYPE::unity_engine::Transform *target) -> void;
        auto exitAttachMode() -> void;

        auto enableStereo() -> bool;
        auto disableStereo() -> void;
        auto applyStereo() -> bool;
    };
}
