#pragma once

namespace FWK::Struct
{
    struct ModelRenderTableINFO final
    {
        explicit ModelRenderTableINFO(const TypeINFO* const a_typeINFO,
                                      const UINT            a_elementByteStride,
                                      const bool            a_isMaterial) :
            k_typeINFO(a_typeINFO),

            k_elementByteStride(a_elementByteStride),

            k_isMaterial(a_isMaterial)
        {}
        ~ModelRenderTableINFO() = default;

        ModelRenderTableINFO(const ModelRenderTableINFO&)  = delete;
        ModelRenderTableINFO(      ModelRenderTableINFO&&) = delete;

        ModelRenderTableINFO& operator=(const ModelRenderTableINFO&)  = delete;
        ModelRenderTableINFO& operator=(      ModelRenderTableINFO&&) = delete;

        const TypeINFO* const k_typeINFO;

        const UINT k_elementByteStride;

        const bool k_isMaterial;
    };
}