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

		std::shared_ptr<Graphics::Texture> m_textureOne = nullptr;
		std::shared_ptr<Graphics::Texture> m_textureTwo = nullptr;

		std::shared_ptr<Struct::SpriteScreenPerObjectDrawRequestData> m_spriteDrawRequestDataOne = nullptr;
		std::shared_ptr<Struct::SpriteScreenPerObjectDrawRequestData> m_spriteDrawRequestDataTwo = nullptr;
	};
}