#pragma once

namespace FWK::Constant
{
    inline const std::filesystem::path k_lowerModelMaterialExtension = ".mat";

    inline constexpr std::wstring_view k_modelMaterialDefaultSubMeshName    = L"Default";
    inline constexpr std::wstring_view k_standardLitModelMaterialFilePrefix = L"StandardLit_";

    inline constexpr TypeAlias::Math::Color k_errorModelMaterialBaseColor = { 1.0F,
                                                                              0.0F,
                                                                              1.0F,
                                                                              1.0F };
}