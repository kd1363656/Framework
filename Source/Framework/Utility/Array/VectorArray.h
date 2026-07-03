#pragma once

namespace FWK::Utility
{
	template <typename Type>
		requires Concept::IsSmartPTRConcept<Type>
	class VectorArray
	{
	private:

		
	public:

		 VectorArray() = default;
		~VectorArray() = default;



	private:

		std::unordered_set<const Type> m_registeredAddressSet = {};

		std::vector<Struct::ArrayElementData> m_arrayElementDataList = {};
	};
}