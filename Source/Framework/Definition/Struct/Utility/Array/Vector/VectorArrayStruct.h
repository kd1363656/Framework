#pragma once

namespace FWK::Struct
{
	// スマートポインタ(std::shared_ptr<T>, std::weak_ptr<T>, std::unique_ptr<T>のメンバしか使えない)
	template <typename Type>
		requires Concept::IsSmartPTRConcept<Type>
	struct ArrayElementData final
	{
		using ElementType = typename Type::element_type;

		Type m_type = {};

		const ElementType* m_typeAddress = nullptr;
	};
}