#pragma once

namespace FWK::Graphics
{
    class ModelMaterialBase;
}

namespace FWK::TypeAlias
{
    using ModelMaterialSharedFactory = GenericFactory<std::shared_ptr<Graphics::ModelMaterialBase>>;
}