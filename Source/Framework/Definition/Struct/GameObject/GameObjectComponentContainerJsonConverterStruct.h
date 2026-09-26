#pragma once

namespace FWK::Struct
{
    struct GameObjectComponentContainerData final
    {
        std::shared_ptr<ComponentBase> m_component = nullptr;

        bool m_isSerializeSkip = false;
    };
}