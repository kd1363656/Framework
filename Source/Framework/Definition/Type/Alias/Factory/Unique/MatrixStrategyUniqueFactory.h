#pragma once

namespace FWK::TypeAlias
{
	using MatrixStrategyUniqueFactory = GenericFactory<std::unique_ptr<MatrixStrategyBase>>;
}