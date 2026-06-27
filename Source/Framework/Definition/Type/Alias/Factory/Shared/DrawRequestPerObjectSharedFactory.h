#pragma once

namespace FWK::TypeAlias
{
	using DrawRequestPerObjectSharedFactory = GenericFactory<std::shared_ptr<Graphics::DrawRequestPerObjectBase>>;
}