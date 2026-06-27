#pragma once

namespace FWK::Utility
{
	inline void DeserializeOptionalShader(const nlohmann::json& a_rootJson, std::shared_ptr<Graphics::Shader>& a_shader)
	{
		if (a_rootJson.is_null()) { return; }

		// jsonで保存されていたということは使う予定のシェーダーなのでインスタンス化
		// されていなければインスタンス化する
		if (!a_shader)
		{
			a_shader = std::make_shared<Graphics::Shader>();
		}

		a_shader->Deserialize(a_rootJson);
	}
}