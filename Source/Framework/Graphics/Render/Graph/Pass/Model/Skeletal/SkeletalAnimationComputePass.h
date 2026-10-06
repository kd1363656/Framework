#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationComputePass final : public RenderGraphPassBase
    {
    public:

         SkeletalAnimationComputePass();
        ~SkeletalAnimationComputePass() override;

        void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph) override;

    private:

        bool UploadBoneMatrix(const ComputeCommandList& a_computeCommandList, Struct::SkeletalAnimationPlayerFrameData& a_frameData) const;

        bool DispatchVertexSkinning(const RootSignature&                                                         a_rootSignature,
                                    const ComputeCommandList&                                                    a_computeCommandList,
                                    const Struct::SkeletalAnimationModelData&                                    a_modelData,
                                          SkeletalAnimationVertexSkinningPerObjectDynamicConstantBufferUploader& a_constantBufferUploader,
                                          Struct::SkeletalAnimationPlayerFrameData&                              a_frameData);

        bool DispatchMeshletBoundsUpdate(const RootSignature&                                                              a_rootSignature,
                                         const ComputeCommandList&                                                         a_computeCommandList,
                                         const Struct::SkeletalAnimationModelData&                                         a_modelData,
                                               SkeletalAnimationMeshletBoundsUpdatePerObjectDynamicConstantBufferUploader& a_constantBufferUploader,
                                               Struct::SkeletalAnimationPlayerFrameData&                                   a_frameData) const;

        static constexpr UINT64 k_boneMatrixBufferCopyDestinationOffset = 0ULL;
        static constexpr UINT64 k_boneMatrixBufferCopySourceOffset      = 0ULL;

        static constexpr std::size_t k_firstModelMeshIndex = 0ULL;

        static constexpr UINT k_vertexSkinningThreadCountX = 64U;
        static constexpr UINT k_singleThreadGroupCount     = 1U;

        FWK_DEFINE_TYPE_INFO(SkeletalAnimationComputePass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::SkeletalAnimationComputePass)