#pragma once

namespace FWK
{
	class GameObject final
	{
	public:

		 GameObject() = default;
		~GameObject() = default;

	private:

		UUID m_uuid = GUID_NULL;

		std::string m_selfName   = {};
		std::string m_prefabName = {};

		bool m_isDestroyed = false;
	};
}