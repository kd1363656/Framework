#pragma once

namespace FWK::Graphics
{
	class ResourceContext final
	{
	public:
		
		 ResourceContext() = default;
		~ResourceContext() = default;

		void Deserialize    (const nlohmann::json& a_rootJson);
		bool PostDeserialize(const Device& a_device);

		nlohmann::json Serialize() const;

		const auto& GetREFRTVDescriptorPool() const { return m_rtvDescriptorPool; }

		auto& GetMutableREFRTVDescriptorPool() { return m_rtvDescriptorPool; }

	private:

		TypeAlias::RTVDescriptorPool m_rtvDescriptorPool = {};

		Converter::ResourceContextJsonConverter m_jsonConverter = {};
	};
}