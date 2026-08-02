#pragma once

namespace FWK::Utility
{
	// ポインターを格納する際に重複するアドレスを持つポインタを格納しないようにするためのクラス
	template <typename Type>
		requires Concept::IsSmartPTRConcept<Type>
	class SmartPointerVectorArray
	{
	public:

		struct ArrayElementData final
		{
			using ElementType = typename Type::element_type;

			Type m_type = {};

			const ElementType* m_typeAddress = nullptr;
		};

	private:

		using ElementType = typename Type::element_type;

		static constexpr bool k_isWeakPTR   = TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Weak;
		static constexpr bool k_isSharedPTR = TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Shared;
		static constexpr bool k_isUniquePTR = TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Unique;
		
	public:

		 SmartPointerVectorArray() = default;
		~SmartPointerVectorArray() = default;

		// shared_ptrからweak_ptrを生成して登録する
		void Add(const Type& a_type)
			requires k_isWeakPTR
		{
			const auto& l_type = a_type.lock();

			FWK_ASSERT_RETURN_IF(!l_type, "登録対象が無効なため、VectorArrayへの登録に失敗しました。");

			// シェーアードが保持している生ポインタを取得
			const auto* const l_typeAddress = l_type.get();

			ArrayElementData l_arrayElementData = {};

			// ポインタとそのポインタを格納
			l_arrayElementData.m_type        = a_type;
			l_arrayElementData.m_typeAddress = l_typeAddress;

			m_arrayElementDataList.emplace_back(std::move(l_arrayElementData));
			m_registeredAddressSet.emplace	   (l_typeAddress);
		}

		// shared_ptrの場合
		void Add(const Type& a_type)
			requires k_isSharedPTR
		{
			FWK_ASSERT_RETURN_IF(!a_type, "登録対象が無効なため、VectorArrayへの登録に失敗しました。");

			const auto* const l_typeAddress = a_type.get();

			ArrayElementData l_arrayElementData = {};

			// ポインタとそのポインタを格納
			l_arrayElementData.m_type        = a_type;
			l_arrayElementData.m_typeAddress = l_typeAddress;
			
			m_arrayElementDataList.emplace_back(std::move(l_arrayElementData));
			m_registeredAddressSet.emplace     (l_typeAddress);
		}

		// uniqueポインタから取得
		void Add(Type&& a_type)
			requires k_isUniquePTR
		{
			FWK_ASSERT_RETURN_IF(!a_type, "登録対象が無効なため、VectorArrayへの登録に失敗しました。");

			// ユニークが所持している生ポインタを取得
			const auto* const l_typeAddress = a_type.get();

			ArrayElementData l_arrayElementData = {};

			// ポインタとそのポインタを格納
			l_arrayElementData.m_type        = std::move(a_type);
			l_arrayElementData.m_typeAddress = l_typeAddress;

			m_arrayElementDataList.emplace_back(std::move(l_arrayElementData));
			m_registeredAddressSet.emplace	   (l_typeAddress);
		}

		void Clear()
		{
			m_arrayElementDataList.clear();
			m_registeredAddressSet.clear();
		}

		// weak_ptrの場合の削除処理
		void RemoveExpiredElements()
			requires k_isWeakPTR
		{
			std::erase_if(m_arrayElementDataList, [this](const ArrayElementData& a_arrayElementData)
			{
				if (!a_arrayElementData.m_type.expired()) { return false; }

				// このアドレスはオブジェクトへアクセスするために
				// 使用せず、unordered_setからキーを削除するためだけに使う。
				m_registeredAddressSet.erase(a_arrayElementData.m_typeAddress);

				return true;
			});
		}

		// unique_ptrの場合の削除処理
		void RemoveExpiredElements()
			requires k_isUniquePTR
		{
			std::erase_if(m_arrayElementDataList, [this](const ArrayElementData& a_arrayElementData)
			{
				if (a_arrayElementData.m_type) { return false; }

				m_registeredAddressSet.erase(a_arrayElementData.m_typeAddress);

				return true;
			});
		}

		void RemoveSameElement(const Type& a_type)
			requires k_isWeakPTR
		{
			const auto& l_removeTarget = a_type.lock();

			if (!l_removeTarget) { return; }

			std::erase_if(m_arrayElementDataList, [this, l_removeTarget](const ArrayElementData& a_arrayElementData)
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

			std::erase_if(m_arrayElementDataList, [this, a_type](const ArrayElementData& a_arrayElementData)
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

		const auto& GetREFArrayElementDataList() const { return m_arrayElementDataList; }

		auto& GetMutableREFArrayElementDataList() { return m_arrayElementDataList; }
		
	private:

		std::unordered_set<const ElementType*> m_registeredAddressSet = {};

		std::vector<ArrayElementData> m_arrayElementDataList = {};
	};
}