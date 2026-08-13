#include "PrefabInstanceNUMAllocator.h"

void FWK::PrefabInstanceNUMAllocator::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	// JsonConverterが使用中・未使用の状態を
	// m_isAllocatedListへ読み込む
	m_jsonConverter.Deserialize(a_rootJson, *this);

	// falseになっている番号を、
	// 再利用可能番号としてQueueへ登録する
	RebuildFreePrefabIDQueue();
}

nlohmann::json FWK::PrefabInstanceNUMAllocator::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

FWK::TypeAlias::PrefabSceneInstanceNUM FWK::PrefabInstanceNUMAllocator::Allocate()
{
	// 削除によって空いた番号が存在する場合は、
	// 新しい番号を作る前に際し利用する
	if (!m_freePrefabInstanceNUMQueue.empty())
	{
		const auto l_reusePrefabInstanceNUM = m_freePrefabInstanceNUMQueue.front();

		m_freePrefabInstanceNUMQueue.pop();

		FWK_ASSERT_RETURN_VALUE_IF(l_reusePrefabInstanceNUM >= static_cast<TypeAlias::PrefabSceneInstanceNUM>(m_isAllocatedList.size()), "再利用しようとしたPrefabInstanceNUMが有効範囲外となっている。", Constant::k_invalidPrefabInstanceNUM);
		FWK_ASSERT_RETURN_VALUE_IF(m_isAllocatedList[l_reusePrefabInstanceNUM],                                                          "使用中のPrefabInstanceNUMを再利用しようとしています。",         Constant::k_invalidPrefabInstanceNUM);

		m_isAllocatedList[l_reusePrefabInstanceNUM] = true;

		return l_reusePrefabInstanceNUM;
	}

	// vectorの要素数が次に新規発行する番号となる
	FWK_ASSERT_RETURN_VALUE_IF(m_isAllocatedList.size() >= static_cast<std::size_t>(Constant::k_invalidPrefabInstanceNUM), "PrefabInstanceNUMをこれ以上発行できません。", Constant::k_invalidPrefabInstanceNUM);

	const auto l_newPrefabInstanceNUM = static_cast<TypeAlias::PrefabSceneInstanceNUM>(m_isAllocatedList.size());

	//　新しい番号尾使用ちゅう状態として追加する
	m_isAllocatedList.emplace_back(true);

	return l_newPrefabInstanceNUM;
}

void FWK::PrefabInstanceNUMAllocator::Release(const TypeAlias::PrefabSceneInstanceNUM a_prefabInstanceNUM)
{
	FWK_ASSERT_RETURN_IF(a_prefabInstanceNUM >= static_cast<TypeAlias::PrefabSceneInstanceNUM>(m_isAllocatedList.size()), "解放しようとしたPrefabInstanceNUMが有効範囲外となっています。");
	FWK_ASSERT_RETURN_IF(!m_isAllocatedList[a_prefabInstanceNUM],                                                         "未使用のPrefabInstanceNUMを二重に解放しようとしています。");

	// 削除されたPrefabインスタンスの名前を未使用状態へ戻す
	m_isAllocatedList[a_prefabInstanceNUM] = false;

	// 次回のAllocate()で再利用できるようにする
	m_freePrefabInstanceNUMQueue.push(a_prefabInstanceNUM);
}

void FWK::PrefabInstanceNUMAllocator::RebuildFreePrefabIDQueue()
{
	// シーンに保存した際のプレハブIDの使用状況を復元する
	m_freePrefabInstanceNUMQueue = {};

	for (std::size_t l_i = 0U; l_i < m_isAllocatedList.size(); ++l_i)
	{
		// trueは現在使用中の番号
		if (m_isAllocatedList[l_i]) { continue; }

		// 現在のfor分のインデックスを使用可能IDとして保存
		m_freePrefabInstanceNUMQueue.push(static_cast<TypeAlias::PrefabSceneInstanceNUM>(l_i));
	}
}