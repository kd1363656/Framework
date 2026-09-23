#pragma once

namespace FWK
{
    class ModelComponentBase;
}

namespace FWK
{
    class ModelComponentBaseInspector final
    {
    public:

         ModelComponentBaseInspector() = default;
        ~ModelComponentBaseInspector() = default;

        void EditInspector(const ModelComponentBase& a_modelComponentBase);
    };
}