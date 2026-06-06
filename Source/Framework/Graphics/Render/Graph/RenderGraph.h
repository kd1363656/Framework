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

	private:


	};
}