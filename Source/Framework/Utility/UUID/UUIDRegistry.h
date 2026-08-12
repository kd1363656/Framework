#pragma once

namespace FWK
{
	template <typename Type>
	class UUIDRegistry final
	{
	private:

		using UUIDMap = std::unordered_map<TypeAlias::UUID, Type>;

		static constexpr bool k_isWeakPTR = TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Weak;

	public:

		 UUIDRegistry() = default;
		~UUIDRegistry() = default;

		bool Add(const Type& a_type, TypeAlias::UUID& a_uuid)
			requires k_isWeakPTR
		{
			const auto& l_type = a_type.lock();

			FWK_ASSERT_RETURN_VALUE_IF(!l_type, "登録対象が無効なため、UUIDMapへの登録に失敗しました。", false);
			
			// 既にUUIDを持っている場合
			// Deserializeなどで復元されたUUIDなので
			// 重複していたからと言って勝手に別UUIDeへ変更しない
			if (!a_uuid.is_nil())
			{
				const bool l_isContains = m_uuidMap.try_emplace(a_uuid, a_type).second;

				FWK_ASSERT_RETURN_VALUE_IF(!l_isContains, "既に同じUUIDが登録されているため、UUIDMapへの登録に失敗しました。", false);

				return l_isContains;
			}

			// 新規GameObjectなど、
			// UUIDをまだ持っていない場合のみ新規発行する
			while (true)
			{
				a_uuid = UUIDManager::GetInstance().GenerateVALUUID();

				if (a_uuid.is_nil()) { continue; }

				if (m_uuidMap.try_emplace(a_uuid, a_type).second)
				{
					return true;
				}
			}
		}

		bool Erase(TypeAlias::UUID& a_uuid)
		{
			FWK_ASSERT_RETURN_VALUE_IF(a_uuid.is_nil(), "UUIDが無効値を指し示しており、UUIDMapからの削除に失敗しました。", false);

			const auto& l_itr = m_uuidMap.find(a_uuid);

			FWK_ASSERT_RETURN_VALUE_IF(l_itr == m_uuidMap.end(), "指定されたUUIDが登録されていないため、UUIDMapからの削除に失敗しました。", false);

			m_uuidMap.erase(l_itr);

			// UUIDを明示的に無効値として扱う
			a_uuid = {};

			return true;
		}

		void Clear()
		{
			m_uuidMap.clear();
		}

		Type FindVALRegisteredType(const TypeAlias::UUID& a_uuid) const
		{
			if (a_uuid.is_nil()) { return {}; }

			const auto l_itr = m_uuidMap.find(a_uuid);

			if (l_itr == m_uuidMap.end()) { return {}; }

			return l_itr->second;
		}

	private:

		UUIDMap m_uuidMap = {};
	};
}