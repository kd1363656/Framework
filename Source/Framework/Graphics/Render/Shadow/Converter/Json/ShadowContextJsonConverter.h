#pragma once

namespace FWK::Graphics
{
	class ShadowContext;
}

namespace FWK::Converter
{
	class ShadowContextJsonConverter
	{
	public:

		 ShadowContextJsonConverter() = default;
		~ShadowContextJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::ShadowContext& a_shadowContext) const;

		nlohmann::json Serialize(const Graphics::ShadowContext& a_shadowContext) const;

	private:

		static constexpr std::string_view k_cascadeShadowMapJsonKey = "CascadeShadowMap";
	};
}