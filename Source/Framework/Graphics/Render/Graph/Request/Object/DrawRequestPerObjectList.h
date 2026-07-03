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
			FWK_ASSERT_RETURN_IF(!a_drawRequestPerObject, "DrawRequestPerObjectが無効のため、DrawRequestPerObjectの登録に失敗しました。");

			const auto& l_drawRequestPerObjectAddress = a_drawRequestPerObject.get();

			// unordered_setにアドレスが存在するか確認するため、平均O(1)で重複確認ができる
			FWK_ASSERT_RETURN_IF(m_registeredAddressSet.contains(l_drawRequestPerObjectAddress), "同じDrawRequestPerObjectを二重登録しようとしており、DrawRequestPerObjectの登録に失敗しました。");

			DrawRequestPerObjectRecord l_drawRequestPerObjectRecord = {};

			l_drawRequestPerObjectRecord.m_drawRequestPerObject		   = a_drawRequestPerObject;
			l_drawRequestPerObjectRecord.m_drawRequestPerObjectAddress = l_drawRequestPerObjectAddress;

			m_drawRequestPerObjectRecordList.emplace_back(l_drawRequestPerObjectRecord);
			m_registeredAddressSet.emplace				 (l_drawRequestPerObjectAddress);
		}

		bool ContainsDrawRequestPerObject(const std::shared_ptr<DrawRequestPerObjectType>& a_drawRequestPerObject) const
		{
			FWK_ASSERT_RETURN_VALUE_IF(!a_drawRequestPerObject, "DrawRequestPerObjectが無効のため、DrawRequestPerObjectの登録確認に失敗しました。", false);

			const auto& l_drawRequestPerObjectAddress = a_drawRequestPerObject.get();

			return m_registeredAddressSet.contains(l_drawRequestPerObjectAddress);
		}

		const auto& GetREFDrawRequestPerObjectRecordList() const { return m_drawRequestPerObjectRecordList; }

	private:

		static constexpr std::size_t k_emptyElementCount = 0U;

		std::unordered_set<const DrawRequestPerObjectType*> m_registeredAddressSet = {};

		std::vector<DrawRequestPerObjectRecord> m_drawRequestPerObjectRecordList = {};
	};
}