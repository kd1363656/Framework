#pragma once

namespace FWK
{
	class RenderComponentBase : public FWK::ComponentBase
	{
	public:

		 RenderComponentBase()          = default;
		~RenderComponentBase() override = default;

	private:

		Utility::AssetFilePath m_assetFilePath = {};

		FWK_DEFINE_TYPE_INFO(RenderComponentBase, ComponentBase)
	};
}