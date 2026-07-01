#pragma once

namespace FWK
{
	class SceneManager;
}

namespace FWK::Converter
{
	class SceneManagerJsonConverter
	{
	public:

		 SceneManagerJsonConverter() = default;
		~SceneManagerJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, SceneManager& a_sceneManager) const;

		nlohmann::json Serialize(const SceneManager& a_sceneManager) const;
	};
}