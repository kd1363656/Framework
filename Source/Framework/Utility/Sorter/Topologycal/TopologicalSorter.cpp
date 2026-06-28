#include "TopologicalSorter.h"

std::vector<std::size_t> FWK::Utility::TopologicalSorter::Sort(const std::vector<std::vector<std::size_t>>& a_dependencyList) const
{
	// dependencyListの要素数が、そのままノード数
	const auto& l_nodeCount = a_dependencyList.size();

	// 次ノードの入次数を数える配列
	// 入次数とは、そのノードより前に実行される必要があるノード数のこと
	// 例 : l_inDegreeList[2] == 1の場合、
	// 3番ノードの前に、まだ一つ実行しないといけないノードがあるということ
	std::vector<std::size_t> l_inDegreeList = {};

	// ノード数分入次数0の状態でリストの要素を作成
	l_inDegreeList.resize(l_nodeCount, k_emptyInDegree);

	// 依存関係リストから入次数を作る
	for (std::size_t l_nodeIndex = 0ULL; l_nodeIndex < l_nodeCount; ++l_nodeIndex)
	{
		for (const auto l_nextNodeIndex : a_dependencyList[l_nodeIndex])
		{
			FWK_ASSERT_RETURN_VALUE_IF(l_nextNodeIndex >= l_nodeCount, "トポロジカルソートの依存先Indexが範囲外です。", {});

			// l_nodeIndex -> l_nextNodeIndexという依存関係があるため、
			// l_nextNodeIndexはl_nodeIndexが終わるまで実行できない
			// そのため、l_nextNodeIndexの入次数を増やす
			++l_inDegreeList[l_nextNodeIndex];
		}
	}

	std::queue<std::size_t> l_visitQueue = {};

	// 入次数が0のノードをキューに入れる
	for (std::size_t l_nodeIndex = 0ULL; l_nodeIndex < l_nodeCount; ++l_nodeIndex)
	{
		if (l_inDegreeList[l_nodeIndex] != k_emptyInDegree) { continue; }

		l_visitQueue.push(l_nodeIndex);
	}

	// 並び替え済みのIndexリスト
	// ここにはノード本体ではなくIndexだけを入れる
	std::vector<std::size_t> l_sortedIndexList = {};

	l_sortedIndexList.reserve(l_nodeCount);

	// 入次数0のみを集めたキューが空になるまで実行
	while (!l_visitQueue.empty())
	{
		// 現在実行順に追加できるノードIndexを取り出す
		const auto l_currentNodeIndex = l_visitQueue.front();

		l_visitQueue.pop();

		// 入次数0のノードは、現在実行可能なので結果に追加する
		l_sortedIndexList.emplace_back(l_currentNodeIndex);

		// l_currentNodeIndexの後に実行するノードたちを見る
		for (const auto& l_nextNodeIndex : a_dependencyList[l_currentNodeIndex])
		{
			// 入次数が既に0の場合、ここでさらに減らすとstd::size_tがアンダーフローする
			FWK_ASSERT_RETURN_VALUE_IF(l_inDegreeList[l_nextNodeIndex] == k_emptyInDegree, "トポロジカルソートの入次数が不正です。", {});

			// l_currentNodeIndexが実行済み扱いになったので、
			// l_nextNodeIndexが持つべきノード数を一つ減らす
			--l_inDegreeList[l_nextNodeIndex];

			// まだ持つべきノードがあるなら、まだキューに入れない
			if (l_inDegreeList[l_nextNodeIndex] != k_emptyInDegree) { continue; }

			// 入り時数が0になったので、次に実行可能な候補に入れる。
			l_visitQueue.push(l_nextNodeIndex);
		}
	}

	// 全てのノードを結果に入れられなかった場合は循環依存。
	FWK_ASSERT_RETURN_VALUE_IF(l_sortedIndexList.size() != l_nodeCount, "トポロジカルソートに失敗しました。循環依存があります。", {});

	return l_sortedIndexList;
}