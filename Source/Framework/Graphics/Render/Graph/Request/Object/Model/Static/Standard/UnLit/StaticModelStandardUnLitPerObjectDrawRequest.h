#pragma once

namespace FWK::Graphics
{
	class StaticModelStandardUnLitPerObjectDrawRequest final : public StaticModelStandardPerObjectDrawRequestBase
	{
	public:

		 StaticModelStandardUnLitPerObjectDrawRequest()			 = default;
		~StaticModelStandardUnLitPerObjectDrawRequest() override = default;

		FWK_DEFINE_TYPE_INFO(StaticModelStandardUnLitPerObjectDrawRequest, StaticModelStandardPerObjectDrawRequestBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::StaticModelStandardUnLitPerObjectDrawRequest)