#include "RenderGraph.h"

void FWK::Graphics::RenderGraph::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Graphics::RenderGraph::BuildDefaultBackBufferGraph()
{
	m_passList.clear		       ();
	m_sortedPassIndexList.clear    ();
	m_resourceStateRecordList.clear();

	AddPass(std::make_unique<RenderGraphBackBufferClearPass>());
	AddPass(std::make_unique<RenderGraphBackBufferPresentPass>());
}
bool FWK::Graphics::RenderGraph::Compile()
{
	m_sortedPassIndexList.clear();

	const auto& l_passCount = m_passList.size();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_passList.empty(), "RenderGraphPassが登録されていないため、RenderGraphのCompileに失敗しました。", false);

	std::vector<std::vector<std::uint32_t>> l_edgeList	   = {};
	std::vector<std::uint32_t>			    l_inDegreeList = {};

	// パス数分作成
	l_edgeList.resize	 (l_passCount);
	l_inDegreeList.resize(l_passCount);

	// 依存関係を構築
	BuildDependency(l_edgeList, l_inDegreeList);

	std::queue<std::uint32_t> l_passQueue = {};

	for (std::uint32_t l_passIndex = 0U; l_passIndex < static_cast<std::uint32_t>(l_passCount); ++l_passIndex)
	{
		if (l_inDegreeList[l_passIndex] != k_noIncomingEdgeCount) { continue; }

		l_passQueue.emplace(l_passIndex);
	}

	while (!l_passQueue.empty())
	{
		const auto l_passIndex = l_passQueue.front();

		l_passQueue.pop();

		m_sortedPassIndexList.emplace_back(l_passIndex);

		for (const auto& l_nextPassIndex : l_edgeList[l_passIndex])
		{
			--l_inDegreeList[l_nextPassIndex];

			if (l_inDegreeList[l_nextPassIndex] != k_noIncomingEdgeCount) { continue; }

			l_passQueue.emplace(l_nextPassIndex);
		}
	}

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_sortedPassIndexList.size() != l_passCount, "RenderGraphPassの依存関係が循環しているため、RenderGraphのCompileに失敗しました。", false);

	return true;
}

void FWK::Graphics::RenderGraph::BeginFrame()
{
	m_resourceStateRecordList.clear();

	SetupCurrentResourceState(Enum::RenderGraphResourceType::BackBuffer, D3D12_RESOURCE_STATE_PRESENT);
}

void FWK::Graphics::RenderGraph::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer)
{
	FWK_ASSERT_RETURN_IF_FAILED(m_sortedPassIndexList.empty(), "RenderGraphがCompileされていないため、RenderGraphのExecuteに失敗しました。");

	for (const auto l_passIndex : m_sortedPassIndexList)
	{
		FWK_ASSERT_RETURN_IF_FAILED(l_passIndex >= m_passList.size(), "RenderGraphPassIndexが範囲外のため、RenderGraphのExecuteに失敗しました。");

		const auto& l_pass = m_passList[l_passIndex];

		FWK_ASSERT_RETURN_IF_FAILED(!l_pass, "RenderGraphPassが無効のため、RenderGraphのExecuteに失敗しました。");

		// Passが要求する状態へ実行前に自動遷移する
		TransitionPassResources(*l_pass, a_renderer);

		l_pass->Execute(a_resourceContext, a_renderer);
	}
}

void FWK::Graphics::RenderGraph::AddPass(std::unique_ptr<RenderGraphPassBase>&& a_pass)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_pass, "RenderGraphPassが無効のため、RenderGraphへの追加に失敗しました。");

	m_passList.emplace_back(std::move(a_pass));
}

nlohmann::json FWK::Graphics::RenderGraph::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::Graphics::RenderGraph::IsReadAccess(const Enum::RenderGraphResourceAccessType a_accessType) const
{
	return a_accessType == Enum::RenderGraphResourceAccessType::Read ||
		   a_accessType == Enum::RenderGraphResourceAccessType::ReadWrite;
}
bool FWK::Graphics::RenderGraph::IsWriteAccess(const Enum::RenderGraphResourceAccessType a_accessType) const
{
	return a_accessType == Enum::RenderGraphResourceAccessType::Write      ||
		   a_accessType == Enum::RenderGraphResourceAccessType::ReadWrite;
}
bool FWK::Graphics::RenderGraph::IsFinalStateAccess(const Enum::RenderGraphResourceAccessType a_accessType) const
{
	return a_accessType == Enum::RenderGraphResourceAccessType::Present;
}

void FWK::Graphics::RenderGraph::AddDependencyEdge(const std::uint32_t a_fromPassIndex, const std::uint32_t a_toPassIndex, std::vector<std::vector<std::uint32_t>>& a_edgeList, std::vector<std::uint32_t>& a_inDegreeList) const
{
	// 同じパス同士では依存関係を構築しない
	if (a_fromPassIndex == a_toPassIndex) { return; }

	for (const auto l_nextPassIndex : a_edgeList[a_fromPassIndex])
	{
		if (l_nextPassIndex == a_toPassIndex) { return; }
	}

	a_edgeList[a_fromPassIndex].emplace_back(a_toPassIndex);

	++a_inDegreeList[a_toPassIndex];
}

void FWK::Graphics::RenderGraph::BuildDependency(std::vector<std::vector<std::uint32_t>>& a_edgeList, std::vector<std::uint32_t>& a_inDegreeList) const
{
	std::vector<ResourceAccessPassRecord> l_resourceAccessPassRecordList = {};

	for (std::uint32_t l_passIndex = 0U; l_passIndex < static_cast<std::uint32_t>(m_passList.size()); ++l_passIndex)
	{
		const auto& l_pass = m_passList[l_passIndex];

		FWK_ASSERT_RETURN_IF_FAILED(!l_pass, "RenderGraphPassが無効のため、依存関係の構築に失敗しました。");

		for (const auto& l_resourceAccess : l_pass->GetREFResourceAccessList())
		{
			ResourceAccessPassRecord l_record = {};

			l_record.m_resourceType		  = l_resourceAccess.m_resourceType;
			l_record.m_passIndex		  = l_passIndex;
			l_record.m_accessOrder		  = static_cast<std::uint32_t>(l_resourceAccessPassRecordList.size());
			l_record.m_isRead			  = IsReadAccess			  (l_resourceAccess.m_accessType);
			l_record.m_isWrite			  = IsWriteAccess			  (l_resourceAccess.m_accessType);
			l_record.m_isFinalStateAccess = IsFinalStateAccess		  (l_resourceAccess.m_accessType);

			FWK_ASSERT_RETURN_IF_FAILED(l_resourceAccess.m_resourceType == Enum::RenderGraphResourceType::None,		 "RenderGraphResourceTypeがNoneのため、依存関係の構築に失敗しました。");
			FWK_ASSERT_RETURN_IF_FAILED(!l_record.m_isRead && !l_record.m_isWrite && !l_record.m_isFinalStateAccess, "RenderGraphResourceAccessTypeが不正なため、依存関係の構築に失敗しました。");

			l_resourceAccessPassRecordList.emplace_back(l_record);
		}
	}

	std::stable_sort(l_resourceAccessPassRecordList.begin(),
					 l_resourceAccessPassRecordList.end(),
					 [](const auto& a_left, const auto& a_right)
					 {
						if (a_left.m_resourceType != a_right.m_resourceType)
						{
							return static_cast<std::uint32_t>(a_left.m_resourceType) < static_cast<std::uint32_t>(a_right.m_resourceType);
						}
						
						return a_left.m_accessOrder < a_right.m_accessOrder;
					 });

	std::uint32_t l_groupBeginIndex = 0U;

	while (l_groupBeginIndex < static_cast<std::uint32_t>(l_resourceAccessPassRecordList.size()))
	{
		std::uint32_t l_groupEndIndex = l_groupBeginIndex;

		while (l_groupEndIndex < static_cast<std::uint32_t>(l_resourceAccessPassRecordList.size()) &&
			   l_resourceAccessPassRecordList[l_groupBeginIndex].m_resourceType == l_resourceAccessPassRecordList[l_groupEndIndex].m_resourceType)
		{
			++l_groupEndIndex;
		}

		// Presentのような「最終状態要求」は、そのResourceを触る通常Passの後ろに置く
		for (std::uint32_t l_recordIndex = l_groupBeginIndex; l_recordIndex < l_groupEndIndex; ++l_recordIndex)
		{
			const auto& l_finalRecord = l_resourceAccessPassRecordList[l_recordIndex];

			if (!l_finalRecord.m_isFinalStateAccess) { continue; }

			for (std::uint32_t l_otherRecordIndex = l_groupBeginIndex; l_otherRecordIndex < l_groupEndIndex; ++l_otherRecordIndex)
			{
				const auto& l_otherRecord = l_resourceAccessPassRecordList[l_otherRecordIndex];

				// Present同士を依存させる必要はない
				// Presentは「最後にこの状態へ戻したい」という要求であり、通常のRead/Write系のPassの後ろにだけ置く
				if (l_otherRecord.m_isFinalStateAccess) { continue; }

				AddDependencyEdge(l_otherRecord.m_passIndex, 
								  l_finalRecord.m_passIndex,
								  a_edgeList,
								  a_inDegreeList);
			}
		}

		std::vector<std::uint32_t> l_pendingReadPassIndexList = {};
		std::uint32_t			   l_lastWritePassIndex		  = k_invalidPassIndex;

		for (std::uint32_t l_recordIndex = l_groupBeginIndex; l_recordIndex < l_groupEndIndex; ++l_recordIndex)
		{
			const auto& l_record = l_resourceAccessPassRecordList[l_recordIndex];

			if (l_record.m_isFinalStateAccess) { continue; }

			if (l_record.m_isRead)
			{
				if (l_lastWritePassIndex != k_invalidPassIndex)
				{
					AddDependencyEdge(l_lastWritePassIndex, 
									  l_record.m_passIndex,
									  a_edgeList,
									  a_inDegreeList);
				}

				bool l_isAlreadyAdded = false;

				for (const auto l_pendingReadPassIndex : l_pendingReadPassIndexList)
				{
					if (l_pendingReadPassIndex != l_record.m_passIndex) { continue; }

					l_isAlreadyAdded = true;
					break;
				}

				if (!l_isAlreadyAdded)
				{
					l_pendingReadPassIndexList.emplace_back(l_record.m_passIndex);
				}
			}

			if (l_record.m_isWrite)
			{
				if (l_lastWritePassIndex != k_invalidPassIndex)
				{
					AddDependencyEdge(l_lastWritePassIndex,
									  l_record.m_passIndex,
									  a_edgeList,
									  a_inDegreeList);
				}

				for (const auto l_pendingReadPassIndex : l_pendingReadPassIndexList)
				{
					AddDependencyEdge(l_pendingReadPassIndex,
									  l_record.m_passIndex,
									  a_edgeList,
									  a_inDegreeList);
				}

				l_pendingReadPassIndexList.clear();

				l_lastWritePassIndex = l_record.m_passIndex;
			}
		}

		l_groupBeginIndex = l_groupEndIndex;
	}
}

void FWK::Graphics::RenderGraph::TransitionPassResources(const RenderGraphPassBase& a_pass, Renderer& a_renderer)
{
	for (const auto& l_resourceAccess : a_pass.GetREFResourceAccessList())
	{
		const auto l_requiresState = ConvertAccessTypeToResourceState(l_resourceAccess.m_accessType);

		if (l_resourceAccess.m_resourceType == Enum::RenderGraphResourceType::BackBuffer)
		{
			TransitionBackBufferResource(l_requiresState, a_renderer);
			continue;
		}

		FWK_ASSERT_RETURN_IF_FAILED(l_resourceAccess.m_resourceType == Enum::RenderGraphResourceType::None, "RenderGraphで未対応のResourceTypeが指定されています。");
	}
}
void FWK::Graphics::RenderGraph::TransitionBackBufferResource(const D3D12_RESOURCE_STATES& a_afterState, Renderer& a_renderer)
{
	const auto l_beforeState = FetchVALCurrentResourceState(Enum::RenderGraphResourceType::BackBuffer);

	if (l_beforeState == a_afterState) { return; }

	const auto& l_swapChain			= a_renderer.GetREFSwapChain		();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	const auto  l_backBufferIndex = l_swapChain.FetchVALCurrentBackBufferIndex();
	const auto& l_backBufferList  = l_swapChain.GetREFBackBufferList		  ();

	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferList.empty(),										 "バックバッファーリストが空になっており、バックバッファーリソースの遷移に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(l_backBufferIndex >= static_cast<UINT>(l_backBufferList.size()), "バックバッファーのインデックス範囲を超えており、バックバッファーリソースの遷移に失敗しました。");

	const auto& l_backBuffer = l_backBufferList[l_backBufferIndex];

	FWK_ASSERT_RETURN_IF_FAILED(!l_backBuffer.m_backBufferResource, "BackBufferResourceが無効のため、BackBufferの状態遷移に失敗しました。");

	l_directCommandList.TransitionResourceBarrier(l_backBuffer.m_backBufferResource, l_beforeState, a_afterState);

	SetupCurrentResourceState(Enum::RenderGraphResourceType::BackBuffer, a_afterState);
}

D3D12_RESOURCE_STATES FWK::Graphics::RenderGraph::ConvertAccessTypeToResourceState(const Enum::RenderGraphResourceAccessType a_accessType) const
{
	switch (a_accessType)
	{
		case Enum::RenderGraphResourceAccessType::Read:
		{
			return D3D12_RESOURCE_STATE_GENERIC_READ;
		}
		break;

		case Enum::RenderGraphResourceAccessType::Write:
		case Enum::RenderGraphResourceAccessType::ReadWrite:
		{
			return D3D12_RESOURCE_STATE_RENDER_TARGET;
		}
		break;

		case Enum::RenderGraphResourceAccessType::Present:
		{
			return D3D12_RESOURCE_STATE_PRESENT;
		}
		break;

		case Enum::RenderGraphResourceAccessType::None:
		default:
		{
			FWK_ASSERT_RETURN_VALUE("RenderGraphResourceAccessTypeが不正です。", D3D12_RESOURCE_STATE_COMMON);
		}
		break;
	}
}

void FWK::Graphics::RenderGraph::SetupCurrentResourceState(const Enum::RenderGraphResourceType a_resourceType, const D3D12_RESOURCE_STATES a_currentState)
{
	for (auto& l_resourceStateRecord : m_resourceStateRecordList)
	{
		if (l_resourceStateRecord.m_resourceType != a_resourceType) { continue; }

		l_resourceStateRecord.m_currentState = a_currentState;
		return;
	}

	ResourceStateRecord l_resourceStateRecord = {};

	l_resourceStateRecord.m_resourceType = a_resourceType;
	l_resourceStateRecord.m_currentState = a_currentState;

	m_resourceStateRecordList.emplace_back(l_resourceStateRecord);
}

D3D12_RESOURCE_STATES FWK::Graphics::RenderGraph::FetchVALCurrentResourceState(const Enum::RenderGraphResourceType a_resourceType) const
{
	for (const auto& l_resourceStateRecord : m_resourceStateRecordList)
	{
		if (l_resourceStateRecord.m_resourceType != a_resourceType) { continue; }

		return l_resourceStateRecord.m_currentState;
	}

	return D3D12_RESOURCE_STATE_COMMON;
}