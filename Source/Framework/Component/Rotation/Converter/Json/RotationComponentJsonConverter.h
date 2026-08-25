#pragma once

namespace FWK
{
	class RotationComponent;
}

namespace FWK::Converter
{
	class RotationComponentJsonConverter final
	{
	public:

		 RotationComponentJsonConverter() = default;
		~RotationComponentJsonConverter() = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson, RotationComponent& a_rotationComponent) const;
		
		nlohmann::json SerializePrefab(const RotationComponent& a_rotationComponent) const;
	};
}