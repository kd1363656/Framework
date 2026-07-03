#pragma once

namespace FWK::Struct
{
	template <typename Type>
		requires Concept::IsSmartPTRConcept<Type>
	struct ArrayElementData final
	{
		Type m_type = {};

		const Type* a_typeAddress = nullptr;
	};
}