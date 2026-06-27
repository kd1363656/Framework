#pragma once

namespace FWK::TypeAlias
{
	using PipelineStateSharedFactory = GenericFactory<std::shared_ptr<Graphics::PipelineStateBase>>;
}