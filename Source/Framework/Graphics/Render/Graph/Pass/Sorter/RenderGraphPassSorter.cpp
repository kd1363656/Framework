#include "RenderGraphPassSorter.h"

void FWK::Graphics::RenderGraphPassSorter::SortPassList(std::vector<std::unique_ptr<RenderGraphPassBase>>& a_passList) const
{
	// 現在のパス総数を取得する。
	const auto& l_passCount = a_passList.size();

	// パス数が2未満なら、並び替える必要がない。
	if (l_passCount < k_minPassCountToResolveExecutionOrder) { return; }

	std::vector<std::vector<std::size_t>> l_passDependencyList = {};

	l_passDependencyList.resize(l_passCount);

	// Pass同士を総当たりで比較して依存関係を作る。
	// l_beforePassIndexより後ろのPassだけ尾を見ることで、同じ組み合わせを二回比較しない
	for (std::size_t l_beforePassIndex = 0ULL; l_beforePassIndex < l_passCount; ++l_beforePassIndex)
	{
		if (!a_passList[l_beforePassIndex]) { continue; }

		for (std::size_t l_afterPassIndex = l_beforePassIndex + k_nextPassIndexOffset; l_afterPassIndex < l_passCount; ++l_afterPassIndex)
		{
			if (!a_passList[l_afterPassIndex]) { continue; }

			// ExecutionLayerから依存関係を作る
			AddPassExecutionLayerDependencyEdge(a_passList,
												l_beforePassIndex,
												l_afterPassIndex,
												l_passDependencyList);

			// ResourceAccessから依存関係を作る
			AddPassResourceDependencyEdge(a_passList,
										  l_beforePassIndex,
										  l_afterPassIndex,
										  l_passDependencyList);
		}
	}

	const Utility::TopologicalSorter l_topologicalSorter = {};

	// 汎用トポロジカルソートクラスに依存関係リストを渡して、
	// 実行順Indexリストを作る。
	const auto& l_sortedPassIndexList = l_topologicalSorter.Sort(l_passDependencyList);

	FWK_ASSERT_RETURN_IF_FAILED(l_sortedPassIndexList.size() != l_passCount, "RenderGraphPassの実行順解決に失敗しました。");

	// 解決した順番に従って、新しいPassListを作る
	std::vector<std::unique_ptr<RenderGraphPassBase>> l_sortedPassList = {};

	l_sortedPassList.reserve(l_passCount);

	for (const auto l_sortedPassIndex : l_sortedPassIndexList)
	{
		FWK_ASSERT_RETURN_IF_FAILED(l_sortedPassIndex >= a_passList.size(), "RenderGraphPassの並び替えIndexが範囲外となっており、RenderGraphPassの実行順解決に失敗しました。");

		l_sortedPassList.emplace_back(std::move(a_passList[l_sortedPassIndex]));
	}

	// 並び替え済みのPassListに差し替える
	a_passList = std::move(l_sortedPassList);
}

void FWK::Graphics::RenderGraphPassSorter::AddPassExecutionLayerDependencyEdge(const std::vector<std::unique_ptr<RenderGraphPassBase>>& a_passList, 
																			   const std::size_t&									    a_beforePassIndex, 
																			   const std::size_t&										a_afterPassIndex, 
																					 std::vector<std::vector<std::size_t>>&				a_passDependencyList) const
{
	FWK_ASSERT_RETURN_IF_FAILED(a_beforePassIndex >= a_passList.size() ||
								a_afterPassIndex  >= a_passList.size(),
								"Before,AfterのPassIndexが範囲外となっており、ExecutionLayer依存関係の作成に失敗しました。");

	const auto& l_beforePass = a_passList[a_beforePassIndex];
	const auto& l_afterPass  = a_passList[a_afterPassIndex];

	FWK_ASSERT_RETURN_IF_FAILED(!l_beforePass, "BeforePassが無効となっており、ExecutionLayer依存関係の作成に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_afterPass,  "AfterPassが無効となっており、ExecutionLayer依存関係の作成に失敗しました。");

	const auto l_beforeExecutionLayer = l_beforePass->GetVALExecutionLayer();
	const auto l_afterExecutionLayer  = l_afterPass->GetVALExecutionLayer ();

	FWK_ASSERT_RETURN_IF_FAILED(l_beforeExecutionLayer == Enum::RenderGraphPassExecutionLayer::Invalid ||
								l_beforeExecutionLayer == Enum::RenderGraphPassExecutionLayer::Count,
								"BeforePassのRenderGraphPassExecutionLayerが無効となっており、ExecutionLayer依存関係の作成に失敗しました。");

	FWK_ASSERT_RETURN_IF_FAILED(l_afterExecutionLayer == Enum::RenderGraphPassExecutionLayer::Invalid ||
								l_afterExecutionLayer == Enum::RenderGraphPassExecutionLayer::Count,
								"AfterPassのRenderGraphPassExecutionLayerが無効です、ExecutionLayer依存関係の作成に失敗しました。");

	// 同じExecutionLayer同士は、Layerだけでは依存辺を作らない
	if (l_beforeExecutionLayer == l_afterExecutionLayer) { return; }

	// 基本は、Json上で前にあるPassを先に実行する依存辺にする
	auto l_dependencyBeforePassIndex = a_beforePassIndex;
	auto l_dependencyAfterPassIndex  = a_afterPassIndex;

	// beforeの方が前のレイヤーなら、そのままbefore->afterにする
	if (l_afterExecutionLayer < l_beforeExecutionLayer)
	{
		l_dependencyBeforePassIndex = a_afterPassIndex;
		l_dependencyAfterPassIndex  = a_beforePassIndex;
	}

	AddPassDependencyEdge(l_dependencyBeforePassIndex, l_dependencyAfterPassIndex, a_passDependencyList);
}
void FWK::Graphics::RenderGraphPassSorter::AddPassResourceDependencyEdge(const std::vector<std::unique_ptr<RenderGraphPassBase>>& a_passList, 
																	     const std::size_t&									      a_beforePassIndex, 
																		 const std::size_t&										  a_afterPassIndex,
																			   std::vector<std::vector<std::size_t>>&			  a_passDependencyList) const
{
	FWK_ASSERT_RETURN_IF_FAILED(a_beforePassIndex >= a_passList.size() ||
								a_afterPassIndex  >= a_passList.size(),
								"Before,AfterのPassIndexが範囲外となっており、ResourceAccess依存関係の作成に失敗しました。");

	const auto& l_beforePass = a_passList[a_beforePassIndex];
	const auto& l_afterPass  = a_passList[a_afterPassIndex];

	FWK_ASSERT_RETURN_IF_FAILED(!l_beforePass, "BeforePassが無効となっており、ResourceAccess依存関係の作成に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_afterPass,  "AfterPassが無効となっており、ResourceAccess依存関係の作成に失敗しました。");

	const auto& l_beforeResourceAccessList = l_beforePass->GetREFResourceAccessList();
	const auto& l_afterResourceAccessList  = l_afterPass->GetREFResourceAccessList ();
	
	for (const auto& l_beforeResourceAccess : l_beforeResourceAccessList)
	{
		for (const auto& l_afterResourceAccess : l_afterResourceAccessList)
		{
			// 違うRenderGraphResource対象なら依存関係は作らない
			if (!IsSameRenderGraphResource(l_beforeResourceAccess, l_afterResourceAccess)) { continue; }

			const auto l_isBeforeRead  = IsReadResourceAccess (l_beforeResourceAccess);
			const auto l_isBeforeWrite = IsWriteResourceAccess(l_beforeResourceAccess);

			const auto l_isAfterRead  = IsReadResourceAccess (l_afterResourceAccess);
			const auto l_isAfterWrite = IsWriteResourceAccess(l_afterResourceAccess);

			// beforeが書いてafterが読む場合。
			// afterはbeforeの書き込み結果を読む必要があるため、
			// before -> afterの依存関係を作る
			if (l_isBeforeWrite && 
				l_isAfterRead)
			{
				AddPassDependencyEdge(a_beforePassIndex, a_afterPassIndex, a_passDependencyList);

				continue;
			}

			// beforeが読んでafterが書く場合
			// after -> beforeの依存関係を作る
			if (l_isBeforeRead && 
				l_isAfterWrite)
			{
				AddPassDependencyEdge(a_afterPassIndex, a_beforePassIndex, a_passDependencyList);

				continue;
			}

			// 両方が同じリソースを書く場合
			// Json順を維持
			if (l_isBeforeWrite && 
				l_isAfterWrite  &&
				IsSamePassExecutionLayer(*l_beforePass, *l_afterPass))
			{
				AddPassDependencyEdge(a_beforePassIndex, a_afterPassIndex, a_passDependencyList);

				continue;
			}
		}
	}
}
void FWK::Graphics::RenderGraphPassSorter::AddPassDependencyEdge(const std::size_t& a_beforePassIndex, const std::size_t& a_afterPassIndex, std::vector<std::vector<std::size_t>>& a_passDependencyList) const
{
	// 自分自身への依存は意味がないため追加しない
	if (a_beforePassIndex == a_afterPassIndex) { return; }

	FWK_ASSERT_RETURN_IF_FAILED(a_beforePassIndex >= a_passDependencyList.size() ||
								a_afterPassIndex  >= a_passDependencyList.size(),
								"RenderGraphPassの依存Indexが範囲外となっており、依存関係の作成に失敗しました。");

	// 同じ依存辺をに順位追加しない。
	// ResourceAccessの組み合わせによっては、
	// 同じPassの依存が複数見つかる可能性がある
	for (const auto l_nextPassIndex : a_passDependencyList[a_beforePassIndex])
	{
		if (l_nextPassIndex != a_afterPassIndex) { continue; }

		return;
	}

	// before -> afterの依存辺を追加する
	// これは「beforeの後にafterを実行する必要がある」という意味
	a_passDependencyList[a_beforePassIndex].emplace_back(a_afterPassIndex);
}

bool FWK::Graphics::RenderGraphPassSorter::IsSamePassExecutionLayer(const RenderGraphPassBase& a_lhs, const RenderGraphPassBase& a_rhs) const
{
	return a_lhs.GetVALExecutionLayer() == a_rhs.GetVALExecutionLayer();
}
bool FWK::Graphics::RenderGraphPassSorter::IsSameRenderGraphResource(const Struct::RenderGraphResourceAccess& a_lhs, const Struct::RenderGraphResourceAccess& a_rhs) const
{
	// BackBufferは専用判定にする
	if (a_lhs.m_isBackBuffer || 
		a_rhs.m_isBackBuffer)
	{
		return a_lhs.m_isBackBuffer &&
			   a_rhs.m_isBackBuffer;
	}

	// RenderTargetLTypeがNone以外ならRenderTarget同士として比較する
	if (a_lhs.m_renderTargetType != Enum::RenderGraphRenderTargetType::None ||
		a_rhs.m_renderTargetType != Enum::RenderGraphRenderTargetType::None)
	{
		return a_lhs.m_renderTargetType == a_rhs.m_renderTargetType;
	}

	if (a_lhs.m_depthStencilType != Enum::RenderGraphDepthStencilType::None ||
		a_rhs.m_depthStencilType != Enum::RenderGraphDepthStencilType::None)
	{
		return a_lhs.m_depthStencilType == a_rhs.m_depthStencilType;
	}

	return false;
}
bool FWK::Graphics::RenderGraphPassSorter::IsReadResourceAccess(const Struct::RenderGraphResourceAccess& a_resourceAccess) const
{
	return a_resourceAccess.m_accessType == Enum::RenderGraphAccessType::Read;
}
bool FWK::Graphics::RenderGraphPassSorter::IsWriteResourceAccess(const Struct::RenderGraphResourceAccess& a_resourceAccess) const
{
	return a_resourceAccess.m_accessType == Enum::RenderGraphAccessType::Write;
}