#pragma once

namespace FWK::Graphics
{
	class ComputeCommandList final : public DirectAndComputeCommandListBase<D3D12_COMMAND_LIST_TYPE_COMPUTE>
	{
	public:

		 ComputeCommandList()          = default;
		~ComputeCommandList() override = default;

		void Reset(const TypeAlias::ComputeCommandAllocator& a_computeCommandAllocator);

		// UAVのResourceStateは変更せず、
		// 先行UAVアクセスと後続UAVアクセスの実行順を保証する
		void UAVResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource2>& a_resource) const;

		void SetupComputePipeline(const std::weak_ptr<ComputePipelineState>& a_pipelineState);

		void SetupConstantBufferView(const D3D12_GPU_VIRTUAL_ADDRESS& a_gpuVirtualAddress, const RootSignature& a_rootSignature, const Enum::RootParameterType a_rootParameterType) const override;

		template <typename RootConstantType>
		void SetupRoot32BitConstants(const RootConstantType& a_rootConstantData, const RootSignature& a_rootSignature, const Enum::RootParameterType a_rootParameterType) const
		{
			// CommandListへByte列として記録するため
			// memcpy可能な単純データ型だけを許可する
			static_assert(std::is_trivially_copyable_v<RootConstantType>, "Root32BitConstantsへ渡す型は、triviallyCopyableである必要があります。");

			// RootConstantsは32bit単位で設定する。
			// そのため、構造体サイズが4Byte単位で
			// 割り切れなければ使用できない。
			static_assert(sizeof(RootConstantType) % k_root32BitValueByteSize == static_cast<std::size_t>(Constant::k_noRemainder));

			constexpr auto l_rootConstantCount = static_cast<UINT>(sizeof(RootConstantType) / k_root32BitValueByteSize);

			SetupRoot32BitConstants(a_rootSignature,
									&a_rootConstantData,
									a_rootParameterType,
									l_rootConstantCount,
									k_rootConstantStartOffset);
		}

		void Dispatch(const UINT a_threadGroupCountX, const UINT a_threadGroupCountY, const UINT a_threadGroupCountZ) const;

	protected:

		void SetupRootSignature(ID3D12GraphicsCommandList6& a_commandList, ID3D12RootSignature& a_rootSignature) override;

	private:

		void SetupRoot32BitConstants(const RootSignature&          a_rootSignature, 
									 const void*                   a_rootConstantData,
			                         const Enum::RootParameterType a_rootParameterType, 
			                         const UINT                    a_rootConstantCount,  
			                         const UINT                    a_destinationOffset) const;

		static constexpr std::size_t k_root32BitValueByteSize = sizeof(std::uint32_t);

		static constexpr std::size_t k_rootConstantStartOffset = 0U;

		static constexpr UINT k_invalidRootConstantCount = 0U;
	};
}