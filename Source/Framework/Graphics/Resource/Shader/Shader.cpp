#include "Shader.h"

void FWK::Graphics::Shader::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_shaderJsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::Shader::CreateFromFile(const ShaderCompiler& a_shaderCompiler)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::CanLoadFilePath(m_filePath, k_lowerCSOExtension), "シェーダーファイルが読み込みに失敗しており、シェーダーの作成に失敗しました。", false);

	m_dxcBlob = a_shaderCompiler.LoadBinaryFromFile(m_filePath.wstring());

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_dxcBlob, "シェーダーバイトコードの作成に失敗しました。", false);

	return true;
}

nlohmann::json FWK::Graphics::Shader::Serialize() const
{
	return m_shaderJsonConverter.Serialize(*this);
}