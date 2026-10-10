#pragma once

namespace FWK::Graphics
{
    class ModelRenderTableINFORegistry final : public Utility::SingletonBase<ModelRenderTableINFORegistry>
    {
    private:

        using ModelRenderTableINFONameMap = std::unordered_map<std::string_view, const Struct::ModelRenderTableINFO* const, Struct::StringHash, std::equal_to<>>;

        friend class SingletonBase<ModelRenderTableINFORegistry>;

         ModelRenderTableINFORegistry()          = default;
        ~ModelRenderTableINFORegistry() override = default;

    public:

        void Register(const Struct::ModelRenderTableINFO& a_tableINFO);

        const auto& GetREFTableINFONameMap() const { return m_tableINFONameMap; }

    private:

        ModelRenderTableINFONameMap m_tableINFONameMap = {};
    };
}