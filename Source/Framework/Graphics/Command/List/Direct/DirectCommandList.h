#pragma once

namespace FWK::Graphics
{
    class DirectCommandList final : public DirectAndComputeCommandListBase<D3D12_COMMAND_LIST_TYPE_DIRECT>
    {
    public:

         DirectCommandList()          = default;
        ~DirectCommandList() override = default;

        void Reset(const TypeAlias::DirectCommandAllocator& a_directCommandAllocator);

        void SetupRenderTarget(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, const UINT a_rtvDescriptorIndex)                                                                                                const;
        void ClearRenderTarget(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, const UINT a_rtvDescriptorIndex, const TypeAlias::Math::Color& a_clearColor = DirectCommandList::k_defaultBackBufferClearColor) const;

        void SetupDepthStencil(const TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool, const TypeAlias::DescriptorIndex a_dsvDescriptorIndex) const;

        void SetupRenderTargetAndDepthStencil(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool,
                                              const TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool,
                                              const TypeAlias::DescriptorIndex    a_rtvDescriptorIndex,
                                              const TypeAlias::DescriptorIndex    a_dsvDescriptorIndex) const;

        void ClearDepthStencil(const TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool,
                               const FLOAT                         a_depthClearValue,
                               const TypeAlias::DescriptorIndex    a_dsvDescriptorIndex,
                               const UINT8                         a_stencilClearValue) const;

        void SetupRenderArea(const RenderArea& a_renderArea) const;

        void SetupRenderPipeline(const std::weak_ptr<GraphicsPipelineStateBase>& a_pipelineState);

        void SetupConstantBufferView(const RootSignature& a_rootSignature, const D3D12_GPU_VIRTUAL_ADDRESS& a_gpuVirtualAddress, const Enum::RootParameterType a_rootParameterType) const override;

        template <typename RootConstantType>
        void SetupRoot32BitConstants(const RootConstantType& a_rootConstantData, const RootSignature& a_rootSignature, const Enum::RootParameterType a_rootParameterType) const
        {
            // 4バイト単位で送るため、memcpyできる型で、大きさが4で割り切れる型だけを許可する
            static_assert(std::is_trivially_copyable_v<RootConstantType>, "Root32BitConstantsへ渡す型は、triviallyCopyableである必要があります。");
            static_assert(sizeof(RootConstantType) % sizeof(std::uint32_t) == static_cast<std::size_t>(Constant::k_noRemainder));

            // 例 : RCModelDrawItem(uint × 3 = 12バイト)なら3個の値として送る
            constexpr auto l_rootConstantCount = static_cast<UINT>(sizeof(RootConstantType) / sizeof(std::uint32_t));

            SetupRoot32BitConstants(a_rootSignature,
                                    &a_rootConstantData,
                                    a_rootParameterType,
                                    l_rootConstantCount,
                                    k_rootConstantStartOffset);
        }

        void SetupPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY a_primitiveTopology) const;

        void SetupVertexBufferView(const D3D12_VERTEX_BUFFER_VIEW& a_vertexBufferView) const;

        void DrawIndexedInstanced(const UINT a_indexCount,
                                  const UINT a_instanceCount,
                                  const UINT a_startIndexLocation,
                                  const UINT a_startInstanceLocation,
                                  const INT  a_baseVertexLocation);

        void DrawInstanced(const UINT a_vertexCount,
                           const UINT a_instanceCount,
                           const UINT a_startVertexLocation,
                           const UINT a_startInstanceLocation) const;

        void DispatchMesh(const UINT a_threadCountGroupX, const UINT a_threadCountGroupY, const UINT a_threadCountGroupZ) const;

        static constexpr TypeAlias::Math::Color k_defaultBackBufferClearColor =
        {
            1.0F,
            0.80F,
            1.0F,
            1.0F
        };

    protected:

        void SetupRootSignature(ID3D12GraphicsCommandList6& a_commandList, ID3D12RootSignature& a_rootSignature) override;

    private:

        static constexpr UINT k_emptySetupRenderTargetNUM  = 0U;
        static constexpr UINT k_singleSetupRenderTargetNUM = 1U;
        static constexpr UINT k_allRECTClear               = 0U;

        void SetupRoot32BitConstants(const RootSignature&          a_rootSignature,
                                     const void*                   a_rootConstantData,
                                     const Enum::RootParameterType a_rootParameterType,
                                     const UINT                    a_rootConstantCount,
                                     const UINT                    a_destinationOffset) const;

        static constexpr UINT k_rootConstantStartOffset  = 0U;
        static constexpr UINT k_invalidRootConstantCount = 0U;

        static constexpr UINT k_setViewportNUM       = 1U;
        static constexpr UINT k_setScissorRECTNUM    = 1U;

        static constexpr UINT k_invalidSizeInBytes        = 0U;
        static constexpr UINT k_vertexBufferViewStartSlot = 0U;
        static constexpr UINT k_vertexBufferViewCount     = 1U;

        static constexpr UINT k_invalidIndexCount    = 0U;
        static constexpr UINT k_invalidInstanceCount = 0U;
    };
}