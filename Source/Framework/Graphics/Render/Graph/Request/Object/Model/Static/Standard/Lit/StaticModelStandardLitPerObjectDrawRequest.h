#pragma once

namespace FWK::Graphics
{
	class StaticModelStandardLitPerObjectDrawRequest final : public StaticModelStandardPerObjectDrawRequestBase
	{
	public:

		 StaticModelStandardLitPerObjectDrawRequest()			 = default;
		~StaticModelStandardLitPerObjectDrawRequest() override = default;

		FWK_DEFINE_TYPE_INFO(StaticModelStandardLitPerObjectDrawRequest, StaticModelStandardPerObjectDrawRequestBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::StaticModelStandardLitPerObjectDrawRequest)