#pragma once

namespace FWK::Graphics
{
	class DescriptorHeapIndexAllocator
	{
	public:

		 DescriptorHeapIndexAllocator() = default;
		~DescriptorHeapIndexAllocator() = default;
	
		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create     ();

		nlohmann::json Serialize() const;

		void Release(const TypeAlias::DescriptorIndex a_index);

		TypeAlias::DescriptorIndex Allocate();

		void SetIndexCapacity(TypeAlias::DescriptorIndex a_set) { m_indexCapacity = a_set; }

		static constexpr auto GetVALDefaultIndexCapacity() { return k_defaultIndexCapacity; }

		auto GetVALIndexCapacity() const { return m_indexCapacity; }

	private:

		bool IsValidIndex(const TypeAlias::DescriptorIndex a_index) const;

		static constexpr TypeAlias::DescriptorIndex k_defaultIndexCapacity     = 10000U;
		static constexpr TypeAlias::DescriptorIndex k_initialNextAllocateIndex = 0U;

		std::vector<bool> m_isAllocatedList = {};

		std::queue<TypeAlias::DescriptorIndex> m_reusableIndexQueue = {};

		Converter::DescriptorHeapIndexAllocatorJsonConverter m_jsonConverter = {};

		TypeAlias::DescriptorIndex m_indexCapacity     = Constant::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_nextAllocateIndex = k_initialNextAllocateIndex; 
	};
}