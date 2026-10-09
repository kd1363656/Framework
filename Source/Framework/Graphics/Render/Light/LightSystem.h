#pragma once

namespace FWK::Graphics
{
    class LightSystem final
    {
    public:

         LightSystem() = default;
        ~LightSystem() = default;

        void Deserialize(const nlohmann::json& a_rootJson);

        nlohmann::json Serialize() const;

        void ApplyDefaultSettings();

        const auto& GetREFCbLightPass() const { return m_cbLightPass; }

        auto& GetMutableREFCbLightPass() { return m_cbLightPass; }

    private:

        void RegisterCBLightPass();

        std::shared_ptr<Struct::CBLightPass> m_cbLightPass = nullptr;

        Converter::LightSystemJsonConverter m_jsonConverter = {};
    };
}