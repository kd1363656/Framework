#pragma once

namespace FWK::Graphics
{
	template<typename DrawRequestPerObjectType>
	class DrawRequestPerObjectList
	{
	private:

		struct DrawRequestPerObjectRecord final
		{
			std::weak_ptr<DrawRequestPerObjectType> m_drawRequestPerObject = {};

			const DrawRequestPerObjectType* m_drawRequestPerObjectAddress = nullptr;
		};

	public:

		 DrawRequestPerObjectList() = default;
		~DrawRequestPerObjectList() = default;

		void BeginFrame()
		{
			std::erase_if(m_drawRequestPerObjectRecordList, [this](const auto& a_drawRequestPerObjectRecord) 
			{
				if (!a_drawRequestPerObjectRecord.m_drawRequestPerObject.expired()) { return false; }

				m_registeredAddressSet.erase(a_drawRequestPerObjectRecord.m_drawRequestPerObjectAddress);

				return true;
			});
		}

		void AddDrawRequestPerObject(const std::shared_ptr<DrawRequestPerObjectType>& a_drawRequestPerObject)
		{
			FWK_ASSERT_RETURN_IF_FAILED(!a_drawRequestPerObject, "DrawRequestPerObjectが無効のため、DrawRequestPerObjectの登録に失敗しました。");

			const auto& l_drawRequestPerObjectAddress = a_drawRequestPerObject.get();

			// unordered_setにアドレスが存在するか確認するため、平均O(1)で重複確認ができる
			FWK_ASSERT_RETURN_IF_FAILED(m_registeredAddressSet.contains(l_drawRequestPerObjectAddress), "同じDrawRequestPerObjectを二重登録しようとしており、DrawRequestPerObjectの登録に失敗しました。");

			DrawRequestPerObjectRecord l_drawRequestPerObjectRecord = {};

			l_drawRequestPerObjectRecord.m_drawRequestPerObject		   = a_drawRequestPerObject;
			l_drawRequestPerObjectRecord.m_drawRequestPerObjectAddress = l_drawRequestPerObjectAddress;

			m_drawRequestPerObjectRecordList.emplace_back(l_drawRequestPerObjectRecord);
			m_registeredAddressSet.emplace				 (l_drawRequestPerObjectAddress);
		}

		bool ContainsDrawRequestPerObject(const std::shared_ptr<DrawRequestPerObjectType>& a_drawRequestPerObject) const
		{
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_drawRequestPerObject, "DrawRequestPerObjectが無効のため、DrawRequestPerObjectの登録確認に失敗しました。", false);

			const auto& l_drawRequestPerObjectAddress = a_drawRequestPerObject.get();

			return m_registeredAddressSet.contains(l_drawRequestPerObjectAddress);
		}

		bool RemoveDrawRequestPerObject(const std::shared_ptr<DrawRequestPerObjectType>& a_drawRequestPerObject)
		{
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_drawRequestPerObject, "DrawRequestPerObjectが無効のため、DrawRequestPerObjectの登録解除に失敗しました。", false);

			const auto& l_drawRequestPerObjectAddress = a_drawRequestPerObject.get();

			// そもそも登録されていない場合は削除不要
			if (!m_registeredAddressSet.contains(l_drawRequestPerObjectAddress)) { return false; }

			// 高速検索用のアドレス集合から削除する
			m_registeredAddressSet.erase(l_drawRequestPerObjectAddress);

			// 実際に描画時に走査するvectorからも削除する
			const auto l_removedCount = std::erase_if(m_drawRequestPerObjectRecordList, [l_drawRequestPerObjectAddress](const auto& a_drawRequestPerObjectRecord)
			{
				return a_drawRequestPerObjectRecord.m_drawRequestPerObjectAddress == l_drawRequestPerObjectAddress;
			});

			// 削除数が1以上である必要がある
			return l_removedCount > k_emptyElementCount;
		}

		const auto& GetREFDrawRequestPerObjectRecordList() const { return m_drawRequestPerObjectRecordList; }

	private:

		static  constexpr std::size_t k_emptyElementCount = 0U;

		std::unordered_set<const DrawRequestPerObjectType*> m_registeredAddressSet = {};

		std::vector<DrawRequestPerObjectRecord> m_drawRequestPerObjectRecordList = {};
	};
}