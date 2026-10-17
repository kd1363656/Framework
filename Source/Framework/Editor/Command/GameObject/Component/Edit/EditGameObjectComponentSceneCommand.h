#pragma once

namespace FWK::Editor
{
    class EditGameObjectComponentSceneCommand final : public SceneCommandBase
    {
    public:

         EditGameObjectComponentSceneCommand(const nlohmann::json&     a_beforeJson,
                                             const nlohmann::json&     a_afterJson,
                                             const boost::uuids::uuid& a_gameObjectUUID,
                                             const boost::uuids::uuid& a_componentUUID);
        ~EditGameObjectComponentSceneCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        void ApplyJson(const nlohmann::json& a_json) const;

        nlohmann::json m_beforeJson;
        nlohmann::json m_afterJson;

        boost::uuids::uuid m_gameObjectUUID;
        boost::uuids::uuid m_componentUUID;
    };
}