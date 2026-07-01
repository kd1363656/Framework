#pragma once

namespace FWK::TypeAlias
{
	using DynamicBufferSharedFactory = GenericFactory<std::shared_ptr<Graphics::DynamicBufferUploaderBase>>;
}