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

		void SetCapacity(TypeAlias::DescriptorIndex a_set) { m_capacity = a_set; }

		static constexpr auto GetVALDefaultCapacity() { return k_defaultCapacity; }

		auto GetVALCapacity() const { return m_capacity; }

	private:

		bool IsInValidIndex(const TypeAlias::DescriptorIndex a_index) const;

		static constexpr TypeAlias::DescriptorIndex k_defaultCapacity  = 10000U;
		static constexpr TypeAlias::DescriptorIndex k_initialNextIndex = 0U;

		std::vector<bool> m_isAllocatedIndexList = {};

		std::queue<TypeAlias::DescriptorIndex> m_reusableIndexQueue = {};

		Converter::DescriptorHeapIndexAllocatorJsonConverter m_jsonConverter = {};

		TypeAlias::DescriptorIndex m_capacity  = DescriptorHeap::k_invalidDescriptorIndex;
		TypeAlias::DescriptorIndex m_nextIndex = k_initialNextIndex; 
	};
}