#pragma once

namespace FWK::TypeAlias
{
	using ConstantBufferSharedFactory = GenericFactory<std::shared_ptr<Graphics::ConstantBufferUploaderBase>>;
}