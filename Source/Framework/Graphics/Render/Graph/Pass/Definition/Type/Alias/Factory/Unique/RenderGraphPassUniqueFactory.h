#pragma once

namespace FWK::TypeAlias
{
	using RenderGraphPassUniqueFactory = GenericFactory<std::unique_ptr<Graphics::RenderGraphPassBase>>;
}