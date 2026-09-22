#pragma once

namespace FWK::Utility
{
    // ポインターを格納する際に重複するアドレスを持つポインタを格納しないようにするためのクラス
    template <typename Type>
        requires Concept::IsSmartPTRConcept<Type> &&
                 (TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Weak ||
                  TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Shared)
    class SmartPointerVectorList final
    {
    public:

        struct ArrayElementData final
        {
            static constexpr std::uintptr_t k_initialTypeAddress = 0ULL;

            Type m_type = {};

            std::uintptr_t m_typeAddress = k_initialTypeAddress;
        };

    private:

        static constexpr bool k_isWeakPTR   = TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Weak;
        static constexpr bool k_isSharedPTR = TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Shared;

    public:

         SmartPointerVectorList() = default;
        ~SmartPointerVectorList() = default;

        // shared_ptrからweak_ptrを生成して登録する
        void Add(const Type& a_type)
            requires k_isWeakPTR
        {
            const auto& l_type = a_type.lock();

            FWK_ASSERT_RETURN_IF(!l_type, "登録対象が無効なため、VectorArrayへの登録に失敗しました。");

            // weak_ptrの場合、以前登録したオブジェクトが既に破棄されている可能性がある
            // 古いアドレス値がSetに残ったままだと、新しいオブジェクトが偶然同じアドレスを再利用した際に
            // 重複と誤判定する可能性があるため、先に削除する
            RemoveExpiredElements();

            // シェーアードが保持している生ポインタを取得
            const auto& l_typeAddress = reinterpret_cast<std::uintptr_t>(l_type.get());

            if (!m_registeredAddressSet.emplace(l_typeAddress).second) { return; }

            ArrayElementData l_arrayElementData = {};

            // ポインタとそのポインタを格納
            l_arrayElementData.m_type        = a_type;
            l_arrayElementData.m_typeAddress = l_typeAddress;

            m_elementDataList.emplace_back(std::move(l_arrayElementData));
        }

        // shared_ptrの場合
        void Add(const Type& a_type)
            requires k_isSharedPTR
        {
            FWK_ASSERT_RETURN_IF(!a_type, "登録対象が無効なため、VectorArrayへの登録に失敗しました。");

            const auto& l_typeAddress = reinterpret_cast<std::uintptr_t>(a_type.get());

            if (!m_registeredAddressSet.emplace(l_typeAddress).second) { return; }

            ArrayElementData l_arrayElementData = {};

            // ポインタとそのポインタを格納
            l_arrayElementData.m_type        = a_type;
            l_arrayElementData.m_typeAddress = l_typeAddress;

            m_elementDataList.emplace_back(std::move(l_arrayElementData));
        }

        void Clear()
        {
            m_elementDataList.clear     ();
            m_registeredAddressSet.clear();
        }

        bool Contains(const Type& a_type)
            requires k_isWeakPTR
        {
            // weak_ptrから一時的にshared_ptrを取得する
            // 既に破棄されているオブジェクトなら登録されているとは判定しない
            const auto& l_type = a_type.lock();

            if (!l_type) { return false; }

            // weak_ptrの場合は、既に破棄された要素のアドレスが
            // unordered_set側に残っている可能性があるため先に掃除する
            RemoveExpiredElements();

            // SmartPointerVectorListは内部でアドレスをキーとして
            // 重複を管理しているため、そのSetを利用して後続に検索する
            const auto& l_typeAddress = reinterpret_cast<std::uintptr_t>(l_type.get());

            return m_registeredAddressSet.contains(l_typeAddress);
        }

        bool Contains(const Type& a_type)
            requires k_isSharedPTR
        {
            if (!a_type) { return false; }

            const auto& l_typeAddress = reinterpret_cast<std::uintptr_t>(a_type.get());

            return m_registeredAddressSet.contains(l_typeAddress);
        }

        // weak_ptrの場合の削除処理
        void RemoveExpiredElements()
            requires k_isWeakPTR
        {
            std::erase_if(m_elementDataList, [this](const ArrayElementData& a_arrayElementData)
            {
                if (!a_arrayElementData.m_type.expired()) { return false; }

                // このアドレスはオブジェクトへアクセスするために
                // 使用せず、unordered_setからキーを削除するためだけに使う。
                m_registeredAddressSet.erase(a_arrayElementData.m_typeAddress);

                return true;
            });
        }

        void RemoveSameElement(const Type& a_type)
            requires k_isWeakPTR
        {
            const auto& l_removeTarget = a_type.lock();

            if (!l_removeTarget) { return; }

            std::erase_if(m_elementDataList, [this, l_removeTarget](const ArrayElementData& a_arrayElementData)
            {
                const auto& l_registeredType = a_arrayElementData.m_type.lock();

                // 登録対象が既に破棄されている場合は
                // Listと登録済みアドレスSetの両方から削除する
                if (!l_registeredType)
                {
                    m_registeredAddressSet.erase(a_arrayElementData.m_typeAddress);

                    return true;
                }

                // 削除対象と異なるオブジェクトなら削除しない
                if (l_registeredType != l_removeTarget) { return false; }

                // 同じオブジェクトなので、登録済みアドレスSetからも削除する
                m_registeredAddressSet.erase(a_arrayElementData.m_typeAddress);

                return true;
            });
        }

        void RemoveSameElement(const Type& a_type)
            requires k_isSharedPTR
        {
            if (!a_type) { return; }

            std::erase_if(m_elementDataList, [this, a_type](const ArrayElementData& a_arrayElementData)
            {
                const auto& l_registeredType = a_arrayElementData.m_type;

                // 登録対象が既に破棄されている場合は
                // Listと登録済みアドレスSetの両方から削除する
                if (!l_registeredType)
                {
                    m_registeredAddressSet.erase(a_arrayElementData.m_typeAddress);

                    return true;
                }

                // 削除対象と異なるオブジェクトなら削除しない
                if (l_registeredType != a_type) { return false; }

                // 同じオブジェクトなので、登録済みアドレスSetからも削除する
                m_registeredAddressSet.erase(a_arrayElementData.m_typeAddress);

                return true;
            });
        }

        const auto& GetREFElementDataList() const { return m_elementDataList; }

        auto& GetMutableREFElementDataList() { return m_elementDataList; }

    private:

        std::unordered_set<std::uintptr_t> m_registeredAddressSet = {};

        std::vector<ArrayElementData> m_elementDataList = {};
    };
}