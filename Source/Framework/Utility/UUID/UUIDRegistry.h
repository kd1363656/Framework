#pragma once

namespace FWK
{
    template <typename Type>
    class UUIDRegistry final
    {
    private:

        using UUIDMap = std::unordered_map<boost::uuids::uuid, Type>;

        static constexpr bool k_isWeakPTR = TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Weak;

    public:

         UUIDRegistry() = default;
        ~UUIDRegistry() = default;

        bool Add(const Type& a_type, const boost::uuids::uuid& a_uuid)
            requires k_isWeakPTR
        {
            const auto& l_type = a_type.lock();

            FWK_ASSERT_RETURN_VALUE_IF(!l_type, "登録対象が無効なため、UUIDMapへの登録に失敗しました。", false);

            // 既にUUIDを持っている場合
            // Deserializeなどで復元されたUUIDなので
            // 重複していたからと言って勝手に別UUIDへ変更しない
            if (!a_uuid.is_nil())
            {
                const bool l_isInserted = m_uuidMap.try_emplace(a_uuid, a_type).second;

                FWK_ASSERT_RETURN_VALUE_IF(!l_isInserted, "既に同じUUIDが登録されているため、UUIDMapへの登録に失敗しました。", false);

                return true;
            }

            return false;
        }

        bool Erase(const boost::uuids::uuid& a_uuid)
        {
            FWK_ASSERT_RETURN_VALUE_IF(a_uuid.is_nil(), "UUIDが無効値を指し示しており、UUIDMapからの削除に失敗しました。", false);

            const auto& l_itr = m_uuidMap.find(a_uuid);

            FWK_ASSERT_RETURN_VALUE_IF(l_itr == m_uuidMap.end(), "指定されたUUIDが登録されていないため、UUIDMapからの削除に失敗しました。", false);

            m_uuidMap.erase(l_itr);

            return true;
        }

        bool Contains(const boost::uuids::uuid& a_uuid) const 
        {
            return m_uuidMap.contains(a_uuid);
        }

        void Clear()
        {
            m_uuidMap.clear();
        }

        Type FindVALRegisteredType(const boost::uuids::uuid& a_uuid) const
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