#pragma once

namespace FWK
{
	class Scene final
	{
	public:

		 Scene() = default;
		~Scene() = default;

		void INIT  ();
		void Update();

	private:

		Graphics::LightSystem m_lightSystem = {};

		std::shared_ptr<Graphics::Camera>									 m_camera						  = nullptr;
		std::shared_ptr<Graphics::StaticModel>								 m_staticModel					  = nullptr;
		std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData> m_staticModelStandardDrawRequest = nullptr;
	};
}