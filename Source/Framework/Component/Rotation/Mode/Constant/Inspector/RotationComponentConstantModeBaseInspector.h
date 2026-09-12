#pragma once

namespace FWK
{
    class RotationComponentConstantModeBase;
}

namespace FWK
{
    class RotationComponentConstantModeBaseInspector final
    {
    public:

         RotationComponentConstantModeBaseInspector() = default;
        ~RotationComponentConstantModeBaseInspector() = default;

        void EditInspector(RotationComponentConstantModeBase& a_rotationComponentConstantModeBase);

    private:

        static constexpr std::string_view k_rotationSpeedLabel = "回転スピード";
    };
}