#pragma once

namespace FWK::Editor
{
    class EditGameObjectTransformComponentCommand final : public ICommand
    {
    public:

         EditGameObjectTransformComponentCommand(const nlohmann::json& a_beforeJson, const nlohmann::json& a_afterJson, const boost::uuids::uuid& a_gameObjectUUID);
        ~EditGameObjectTransformComponentCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        void ApplyJson(const nlohmann::json& a_json) const;

        nlohmann::json m_beforeJson;
        nlohmann::json m_afterJson;

        boost::uuids::uuid m_gameObjectUUID;
    };
}