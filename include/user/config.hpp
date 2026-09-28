#pragma once

#include "freecam/freecam.hpp"
#include "umod/core.hpp"
#include "umod/runtime/helper/input.hpp"

#include <chrono>
#include <vector>

namespace user_config
{
    // Loader
    namespace loader
    {
        using namespace std::chrono_literals;

        inline bool Hardened = true;
        inline std::chrono::duration<float> WaitBeforeInit = 5s;
    }

    namespace core
    {
        using namespace std::chrono_literals;
        using namespace umod::core;
        using namespace player_loop;

        inline player_loop::Type PlayerLoopType = player_loop::Type::Internal;
        inline std::chrono::duration<float> MockLoopDeltaTime = 16.7ms;
        inline std::vector<std::string> EnabledModules = {"FreeCam"};
    }

    namespace freecam
    {
        using CameraMode = ::freecam::Mode;

        inline CameraMode Mode = CameraMode::Depth; // 相机模式
        inline bool DisableOrigCam = false;         // 禁用原相机

        // 镜头属性
        namespace property
        {
#ifdef __ANDROID__
            inline float RotationSpeed = 100.f; // 镜头旋转速度
#else
            inline float RotationSpeed = 300.f; // 镜头旋转速度
#endif
            inline float RollSpeed = 100.f;   // 镜头滚动速度
            inline float BaseMoveSpeed = 1.f; // 基础移动速度
            inline float MaxMoveSpeed = 50.f; // 最大移动速度
        }

        // SBS 3D 输出
        namespace stereo
        {
            using StereoFormat = ::freecam::StereoFormat;

            inline bool Enabled = false;                        // 启用 SBS 3D 输出
            inline StereoFormat Format = StereoFormat::HalfSBS; // 输出格式
            inline float Separation = 0.064f;                   // 眼距（世界单位）
            inline float Convergence = 10.f;                    // 汇聚距离（世界单位）
            inline bool SwapEyes = false;                       // 交换左右画面
        }

        // 键位
        namespace keybind
        {
            using umod::unity_runtime::helper::InputUtils::Key;
            using enum Key;

            inline Key ToggleFreeCam = Enter; // 开启/关闭自由镜头

            // 移动
            inline Key Up = Space;        // 上升
            inline Key Down = Ctrl_L;     // 下降
            inline Key Forward = W;       // 前
            inline Key Back = S;          // 后
            inline Key Left = A;          // 左
            inline Key Right = D;         // 右
            inline Key SpeedUp = Shift_L; // 加速

            // 转动
            inline Key RotateUp = UpArrow;       // 镜头向上转动
            inline Key RotateDown = DownArrow;   // 镜头向下转动
            inline Key RotateLeft = LeftArrow;   // 镜头向左转动
            inline Key RotateRight = RightArrow; // 镜头向右转动
            inline Key RollLeft = Q;             // 镜头逆时针滚动
            inline Key RollRight = E;            // 镜头顺时针滚动
            inline Key ResetRoll = R;            // 复位镜头滚动

            // 镜头缩放
            inline Key ZoomMode = Z; // 镜头缩放模式
            inline Key ZoomIn = X;   // 放大
            inline Key ZoomOut = C;  // 缩小

            inline Key PinAnchor = M; //固定锚点

            inline Key UIMode = U;     // 开启/关闭 UI 模式
            inline Key AttachMode = T; // 开启/退出依附模式

            // SBS 3D
            inline Key ToggleStereo = B;    // 开启/关闭 SBS 3D 输出
            inline Key SeparationUp = L;    // 增大眼距
            inline Key SeparationDown = J;  // 减小眼距
            inline Key ConvergenceUp = I;   // 推远汇聚距离
            inline Key ConvergenceDown = K; // 拉近汇聚距离
        }
    }

    namespace speed_gear
    {
        namespace keybind
        {
            using umod::unity_runtime::helper::InputUtils::Key;
            using enum Key;

            inline Key ToggleFreeze = Backspace; // 开启/关闭时间流动
            inline Key SpeedUp = Equals;         // 加速时间流动
            inline Key SpeedDown = Minus;        // 减速时间流动
        }
    }
}
