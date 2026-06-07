#pragma once

namespace FWK::Graphics
{
	class ResourceContext;
}

namespace FWK::Converter
{
	class ResourceContextJsonConverter final
	{
	public:

		 ResourceContextJsonConverter() = default;
		~ResourceContextJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::ResourceContext& a_resourceContext) const;

		nlohmann::json Serialize(const Graphics::ResourceContext& a_resourceContext) const;

	private:

		static constexpr std::string_view k_rtvDescriptorPoolJsonKey = "RTVDescriptorPool";
		static constexpr std::string_view k_srvDescriptorPoolJsonKey = "SRVDescriptorPool";
	};
}