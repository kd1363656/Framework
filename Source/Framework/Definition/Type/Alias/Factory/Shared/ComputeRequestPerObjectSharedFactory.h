#pragma once

namespace FWK::TypeAlias
{
	using ComputeRequestPerObjectSharedFactory = GenericFactory<std::shared_ptr<Graphics::ComputeRequestPerObjectBase>>;
}