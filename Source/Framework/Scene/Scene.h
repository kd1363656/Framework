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

		std::shared_ptr<Graphics::Texture> m_texture = nullptr;

		std::shared_ptr<Struct::SpriteScreenPerObjectDrawRequestData> m_spriteDrawRequestData = nullptr;
	};
}