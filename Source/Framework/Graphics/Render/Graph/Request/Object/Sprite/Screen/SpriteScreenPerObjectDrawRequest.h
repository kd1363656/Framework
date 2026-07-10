#pragma once

namespace FWK::Graphics
{
	class Renderer;
}

namespace FWK::Graphics
{
	class SpriteScreenPerObjectDrawRequest final : public DrawRequestPerObjectBase
	{
	public:

		struct DrawRequestData final
		{
			std::weak_ptr<Graphics::TextureRecord> m_textureRecord = {};

			TypeAlias::Math::Color m_color = Constant::k_whiteColor;

			TypeAlias::Math::Vector2 m_position = TypeAlias::Math::Vector2::Zero;
			TypeAlias::Math::Vector2 m_scale    = TypeAlias::Math::Vector2::One;
			TypeAlias::Math::Vector2 m_pivot    = Constant::k_defaultSpritePivot;

			Struct::SpriteRECT m_sourceRECT = {};
		};

	public:

		 SpriteScreenPerObjectDrawRequest()          = default;
		~SpriteScreenPerObjectDrawRequest() override = default;

		void BeginFrame() override;

		void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

		void AddDrawRequestPerObject(const std::shared_ptr<DrawRequestData>& a_drawRequestData);

	private:

		Utility::VectorArray<std::weak_ptr<DrawRequestData>> m_drawRequestDataList = {};

		FWK_DEFINE_TYPE_INFO(SpriteScreenPerObjectDrawRequest, DrawRequestPerObjectBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::SpriteScreenPerObjectDrawRequest)