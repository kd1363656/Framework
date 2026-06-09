#pragma once

namespace FWK::Graphics
{
	class Shader;
}

namespace FWK::Converter
{
	class ShaderJsonConverter final
	{
	public:

		 ShaderJsonConverter() = default;
		~ShaderJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::Shader& a_shader) const;

		nlohmann::json Serialize(const Graphics::Shader& a_shader) const;

	private:

		static constexpr std::string_view k_filePathJsonKey               = "FilePath";
	    static constexpr std::string_view k_entryPointNameJsonKey         = "EntryPointName";
	    static constexpr std::string_view k_shaderModelVersionNameJsonKey = "ShaderModelVersionName";
	};
}