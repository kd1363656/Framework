#pragma once

namespace FWK::Graphics
{
	// Compute命令をできるコマンドリストの共通基底クラス
	template <D3D12_COMMAND_LIST_TYPE CommandType>
		requires (CommandType == D3D12_COMMAND_LIST_TYPE_DIRECT ||
			      CommandType == D3D12_COMMAND_LIST_TYPE_COMPUTE)
	class DirectAndComputeCommandListBase : public CommandListBase<CommandType>
	{
	public:

		         DirectAndComputeCommandListBase() = default;
		virtual ~DirectAndComputeCommandListBase() = default;

		// UAVのResourceStateは変更せず、
		// 専攻UAVアk巣エスト後続UAVアクセスの実行順を保証する
		void UAVResourceBarrier(const TypeAlias::ComPtr<ID3D12Resource2>& a_resource) const 
		{
			FWK_ASSERT_RETURN_IF(!a_resource, "UAVBarrierを設定するGPUResourceが無効です。");

			const auto& l_commandList = this->GetREFCommandList();

			FWK_ASSERT_RETURN_IF(!l_commandList, "CommandListが作成されておらず、UAV Barrierの設定に失敗しました。");

			const auto& l_resourceBarrier = CD3DX12_RESOURCE_BARRIER::UAV(a_resource.Get());

			l_commandList->ResourceBarrier(k_singleSetupBarrierNUM, &l_resourceBarrier);
		}


		template <D3D12_DESCRIPTOR_HEAP_TYPE Type>
			requires (Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
		              Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		void SetupDescriptorHeap(const DescriptorPool<Type>& a_descriptorPool) const
		{
			const auto& l_directCommandList = this->GetREFCommandList();

			FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、ディスクリプタヒープの設定ができませんでした。");

			const auto& l_shaderVisibleDescriptorHeap = a_descriptorPool.GetREFShaderVisibleDescriptorHeap();

			FWK_ASSERT_RETURN_IF(!l_shaderVisibleDescriptorHeap, "ShaderVisibleなディストラクタヒープが無効になっており、ディスクリプタヒープの設定ができませんでした。");

			const auto& l_descriptorHeap = l_shaderVisibleDescriptorHeap->GetREFDescriptorHeap();

			FWK_ASSERT_RETURN_IF(!l_descriptorHeap, "ShaderVisibleなディスクリプタヒープが作成されておらず、ディスクリプタヒープの設定ができませんでした。");

			ID3D12DescriptorHeap* l_descriptorHeapList[] =
			{
				l_descriptorHeap.Get()
			};

			// シェーダーから参照するDescriptorHeapを設定する
			// SetDescriptorHeap(設定するヒープ数,
			//					 ヒープ配列の先頭アドレス);
			l_directCommandList->SetDescriptorHeaps(k_setDescriptorHeapNUM, l_descriptorHeapList);
		}

		virtual void SetupConstantBufferView(const D3D12_GPU_VIRTUAL_ADDRESS& a_gpuVirtualAddress, const RootSignature& a_rootSignature, const Enum::RootParameterType a_rootParameterType) const;

	protected:

		void Reset(const CommandAllocator<CommandType>& a_commandAllocator) override
		{
			CommandListBase<CommandType>::Reset(a_commandAllocator);

			m_currentRootSignature.reset();
			m_currentPipelineState.reset();
		}

		virtual void SetupRootSignature(ID3D12GraphicsCommandList6& a_commandList, ID3D12RootSignature& a_rootSignature) = 0;

		void SetupPipeline(const std::weak_ptr<PipelineStateBase>& a_pipelineState)
		{
			const auto& l_directCommandList = this->GetREFCommandList();

			FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、パイプラインの設定に失敗しました。");

			const auto& l_pipelineState = a_pipelineState.lock();

			FWK_ASSERT_RETURN_IF(!l_pipelineState, "パイプラインステートが無効なため、パイプラインの設定に失敗しました。");

			const auto& l_rootSignature = l_pipelineState->GetREFUseRootSignature().lock();

			FWK_ASSERT_RETURN_IF(!l_rootSignature, "パイプラインステートで使用するルートシグネチャが無効なため、パイプラインの設定に失敗しました。");

			const auto& l_d3dRootSignature = l_rootSignature->GetREFRootSignature();

			FWK_ASSERT_RETURN_IF(!l_d3dRootSignature, "ルートシグネチャが作成されておらず、パイプラインの設定に失敗しました。");

			const auto& l_d3dPipelineState = l_pipelineState->GetREFPipelineState();

			FWK_ASSERT_RETURN_IF(!l_d3dPipelineState, "パイプラインステートが作成されておらず、パイプラインの設定に失敗しました。");
			
			// コマンドリストにルートシグネチャを設定する関数
			// SetGraphicsRootSignature(描画パイプラインで使用するルートシグネチャのポインタ);
			// ルートシグネチャは、シェーダーにどのリソースをどう渡すかを表す設定情報
			// これを先に設定しておかないと、後続の描画で使用するリソースの結び付けルールが決まらない
			// 同じルートシグネチャの場合はセットしない(ルートシグネチャのセットは重いから)
			if (m_currentRootSignature.owner_before(l_pipelineState->GetREFUseRootSignature()) ||
				l_pipelineState->GetREFUseRootSignature().owner_before(m_currentRootSignature))
			{
				SetupRootSignature(*l_directCommandList.Get(), *l_d3dRootSignature.Get());

				m_currentRootSignature = l_pipelineState->GetREFUseRootSignature();
			}

			// コマンドリストにパイプラインステートをセットする関数
			// SetPipelineState(パイプラインステートのポインタ)
			// PSO(PipelineStateObject)には、
			// どのシェーダーを使うか、
			// どうラスタライズするか
			// 深度テストを使うか、など
			// 描画パイプラインの重要な設定がまとめて入っている
			// 同じパイプラインステートの場合はセットしない(パイプラインステートのセットは重いから)
			if (m_currentPipelineState.owner_before(a_pipelineState) ||
				l_pipelineState.owner_before(m_currentPipelineState))
			{
				l_directCommandList->SetPipelineState(l_d3dPipelineState.Get());

				m_currentPipelineState = a_pipelineState;
			}
		}

	private:

		static constexpr UINT k_singleSetupBarrierNUM = 1U;
		static constexpr UINT k_setDescriptorHeapNUM  = 1U;

		std::weak_ptr<RootSignature>     m_currentRootSignature = {};
		std::weak_ptr<PipelineStateBase> m_currentPipelineState = {};
	};
}