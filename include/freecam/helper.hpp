#pragma once

#include "umod/utype/unity_engine/core.hpp"

namespace freecam::helper
{
    auto isCurrentFreeCamera() -> bool;
    auto isDestroyed(umod::UTYPE::unity_engine::UnityObject *) -> bool;
    auto selectGameObject(umod::UTYPE::unity_engine::Transform *view) -> umod::UTYPE::unity_engine::Transform *;
    auto getMaxDepthCamera() -> umod::UTYPE::unity_engine::Camera *;
}
