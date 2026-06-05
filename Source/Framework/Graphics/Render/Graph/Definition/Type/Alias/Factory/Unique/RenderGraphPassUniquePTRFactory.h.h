#pragma once

namespace FWK::TypeAlias
{
	using RenderGraphPassUniquePTRFactory = GenericFactory<std::unique_ptr<Graphics::RenderGraphPassBase>>;
}