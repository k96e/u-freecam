#pragma once

#include "umod/runtime/UnityResolve.hpp"

namespace umod::unity_runtime
{
    // UnityResolve keeps the detected mode private, so the bootstrap records a copy here
    inline UnityResolve::Mode Backend = UnityResolve::Mode::Il2Cpp;
}
