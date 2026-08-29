#pragma once

namespace FWK::TypeAlias
{
	using ComponentEventNotifyStrategyUniqueFactory = GenericFactory<std::unique_ptr<ComponentEventNotifyStrategyBase>>;
}