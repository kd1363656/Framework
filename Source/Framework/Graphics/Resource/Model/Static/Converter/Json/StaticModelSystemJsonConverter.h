#pragma once

namespace FWK::Graphics
{
	class StaticModelSystem;
}

namespace FWK::Converter
{
	class StaticModelSystemJsonConverter final
	{
	public:

		 StaticModelSystemJsonConverter() = default;
		~StaticModelSystemJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::StaticModelSystem& a_staticModelSystem) const;

		nlohmann::json Serialize(const Graphics::StaticModelSystem& a_staticModelSystem) const;

	private:

		static constexpr std::string_view k_modelStorageJsonKey = "ModelStorage";
	};
}