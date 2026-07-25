#pragma once

namespace FWK::Graphics
{
	class ShadowContext final
	{
	public:

		 ShadowContext() = default;
		~ShadowContext() = default;

		ShadowContext(const ShadowContext&)           = delete;
		ShadowContext(      ShadowContext&&) noexcept = default;

		ShadowContext& operator=(const ShadowContext&)           = delete;
		ShadowContext& operator=(      ShadowContext&&) noexcept = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		bool Create(const Device&                             a_device, 
			        const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
			              TypeAlias::DSVDescriptorPool&       a_dsvDescriptorPool,
			              TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

		nlohmann::json Serialize() const;

		const auto& GetREFCascadeShadowMap() const { return m_cascadeShadowMap; }

		auto& GetMutableREFCascadeShadowMap() { return m_cascadeShadowMap; }

	private:

		CascadeShadowMap m_cascadeShadowMap = {};

		Converter::ShadowContextJsonConverter m_jsonConverter = {};
	};
}