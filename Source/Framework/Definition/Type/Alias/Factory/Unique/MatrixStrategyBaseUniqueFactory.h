#pragma once

namespace FWK::TypeAlias
{
	using MatrixStrategyFactory = GenericFactory<std::unique_ptr<MatrixStrategyBase>>;
}