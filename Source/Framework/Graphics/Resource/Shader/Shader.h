#pragma once

namespace FWK::Graphics
{
	class Shader final
	{
	public:

		 Shader() = default;
		~Shader() = default;
		
		void Deserialize   (const nlohmann::json& a_rootJson);
		bool CreateFromFile(const ShaderCompiler& a_shaderCompiler);

		nlohmann::json Serialize() const;

		void SetFilePath              (const std::filesystem::path& a_set) { m_filePath               = a_set; }
		void SetEntryPointName        (const std::string&			a_set) { m_entryPointName         = a_set; }
		void SetShaderModelVersionName(const std::string&			a_set) { m_shaderModelVersionName = a_set; }

		const auto& GetDXCBlob() const { return m_dxcBlob; }

		const auto& GetFilePath              () const { return m_filePath; }
		const auto& GetEntryPointName        () const { return m_entryPointName; }
		const auto& GetShaderModelVersionName() const { return m_shaderModelVersionName; }

	private:

		const std::filesystem::path k_lowerCSOExtension = ".cso";
		
		TypeAlias::ComPtr<IDxcBlob> m_dxcBlob = nullptr;

		Converter::ShaderJsonConverter m_shaderJsonConverter = {};

		std::filesystem::path m_filePath               = {};
		std::string           m_entryPointName         = {};
		std::string           m_shaderModelVersionName = {};
	};
}