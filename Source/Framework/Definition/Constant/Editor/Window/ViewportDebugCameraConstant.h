#pragma once

namespace FWK::Constant
{
#if defined(_DEBUG)
    inline constexpr bool k_isViewportDebugCameraAvailable = true;
#else
    inline constexpr bool k_isViewportDebugCameraAvailable = false;
#endif

    inline constexpr float k_viewportDebugCameraDefaultPositionX = 0.0F;
    inline constexpr float k_viewportDebugCameraDefaultPositionY = 2.0F;
    inline constexpr float k_viewportDebugCameraDefaultPositionZ = -5.0F;

    inline constexpr float k_viewportDebugCameraDefaultYawDegree   = 0.0F;
    inline constexpr float k_viewportDebugCameraDefaultPitchDegree = 10.0F;

    inline constexpr float k_viewportDebugCameraDefaultMoveSpeed           = 5.0F;
    inline constexpr float k_viewportDebugCameraDefaultRotationSpeedDegree = 0.15F;

    inline constexpr bool k_viewportDebugCameraDefaultIsEnabled                = false;
    inline constexpr bool k_viewportDebugCameraDefaultIsCullingDebugVisualize  = true;
    inline constexpr bool k_viewportDebugCameraDefaultIsDrawMainCameraFrustum = true;
}
