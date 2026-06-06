#pragma once

namespace FWK::Graphics
{
	class Renderer;
}

namespace FWK::Graphics
{
	// レンダーパスの依存関係を調べ実行順序を決める、
	// またリソースの状態遷移が必要なら状態遷移を行う。
	class RenderGraph
	{
	public:

		 RenderGraph() = default;
		~RenderGraph() = default;

		void BeginFrame(const ResourceContext& a_resourceContext, const Renderer& a_renderer) const;

		void EndFrame(const Renderer& a_renderer) const;

	private:

		void ClearBackBuffer(const ResourceContext& a_resourceContext, const Renderer& a_renderer) const;

		void TransitionBackBufferResource(const DirectCommandList&    a_directCommandList, 
									      const D3D12_RESOURCE_STATES a_beforeState,
										  const D3D12_RESOURCE_STATES a_afterState, 
										  const Struct::BackBuffer&	  a_backBuffer) const;

		static constexpr TypeAlias::Math::Color k_backBufferClearColor =
		{
			1.0F,
			0.80F,
			1.0F,
			1.0F
		};
	};
}