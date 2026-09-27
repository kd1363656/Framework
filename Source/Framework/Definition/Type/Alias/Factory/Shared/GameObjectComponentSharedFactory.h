#pragma once

namespace FWK::TypeAlias
{
    using GameObjectComponentSharedFactory = GenericFactory<std::shared_ptr<GameObjectComponentBase>>;
}