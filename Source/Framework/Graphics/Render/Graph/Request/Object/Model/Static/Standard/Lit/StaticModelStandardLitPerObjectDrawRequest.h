#pragma once

namespace FWK::Graphics
{
	class StaticModelStandardLitPerObjectDrawRequest final : public StaticModelPerObjectDrawRequestBase
	{
	public:

		 StaticModelStandardLitPerObjectDrawRequest()			 = default;
		~StaticModelStandardLitPerObjectDrawRequest() override = default;

		FWK_DEFINE_TYPE_INFO(StaticModelStandardLitPerObjectDrawRequest, StaticModelPerObjectDrawRequestBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::StaticModelStandardLitPerObjectDrawRequest)