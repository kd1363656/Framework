#pragma once

namespace FWK
{
	class Scene;
}

namespace FWK::Converter
{
	class SceneJsonConverter
	{
	public:

		 SceneJsonConverter() = default;
		~SceneJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Scene& a_scene) const;

		nlohmann::json Serialize(const Scene& a_scene) const;
	};
}