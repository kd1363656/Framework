#pragma once

namespace FWK
{
	template <typename Type>
	class UUIDRegistry final
	{
	private:

		struct Hasher final
		{
			std::size_t operator()(const UUID& a_uuid) const
			{
				RPC_STATUS l_status = {};

				// UuidHashはUUID*を要求するため、
				// const_castせずローカル変数へコピーして渡す
				UUID l_uuid = a_uuid;

				const auto l_hash = UuidHash(&l_uuid, &l_status);

				FWK_ASSERT_RETURN_VALUE_IF(l_status == RPC_S_OK, "ハッシュ化に失敗しました。", l_hash);

				return l_hash;
			}
		};

		struct Equal final
		{
			bool operator()(const UUID& a_lhs, const UUID& a_rhs) const
			{
				return IsEqualGUID(a_lhs, a_rhs) != FALSE;
			}
		};

		using UUIDMap = std::unordered_map<UUID, Type, Hasher, Equal>;

		static constexpr bool k_isWeakPTR = TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Weak;

	public:

		 UUIDRegistry() = default;
		~UUIDRegistry() = default;

		bool Add(const Type& a_type, UUID& a_uuid)
			requires k_isWeakPTR
		{
			const auto& l_type = a_type.lock();

			FWK_ASSERT_RETURN_VALUE_IF(!l_type,                    "登録対象が無効なため、UUIDMapへの登録に失敗しました。",                               false);
			FWK_ASSERT_RETURN_VALUE_IF(m_uuidMap.contains(a_uuid), "既に登録されているUUIDのものを登録しようとしており、UUIDMapへの登録に失敗しました。", false);
			
			// 追加する際にUUIDがGUID_NULLか、同じUUID値を登録してしまうことを防ぐためにここでUUIDを生成する
			// 被った値を生成する可能性も考慮して、被らない値を生成するまでループする
			while (a_uuid == GUID_NULL ||
				   m_uuidMap.contains(a_uuid))
			{
				Utility::GenerateUUID(a_uuid);
			}

			m_uuidMap.try_emplace(a_uuid, a_type);
			
			return true;
		}

		bool Erase(UUID& a_uuid)
		{
			FWK_ASSERT_RETURN_VALUE_IF(IsEqualGUID(a_uuid, GUID_NULL) != FALSE, "UUIDが無効値を指し示しており、UUIDMapからの削除に失敗しました。", false);

			const auto& l_itr = m_uuidMap.find(a_uuid);

			FWK_ASSERT_RETURN_VALUE_IF(l_itr == m_uuidMap.end(), "指定されたUUIDが登録されていないため、UUIDMapからの削除に失敗しました。", false);

			m_uuidMap.erase(l_itr);

			// UUIDを明示的に無効値として扱う
			a_uuid = GUID_NULL;

			return true;
		}

	private:

		UUIDMap m_uuidMap = {};
	};
}