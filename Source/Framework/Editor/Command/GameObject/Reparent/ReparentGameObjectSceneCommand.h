#pragma once

namespace FWK::Editor
{
    class ReparentGameObjectSceneCommand final : public SceneCommandBase
    {
    public:

         ReparentGameObjectSceneCommand(const boost::uuids::uuid& a_gameObjectUUID, const Struct::GameObjectPlacement& a_beforePlacement, const Struct::GameObjectPlacement& a_afterPlacement);
        ~ReparentGameObjectSceneCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        void ApplyPlacement(const Struct::GameObjectPlacement& a_placement) const;

        boost::uuids::uuid m_gameObjectUUID;

        Struct::GameObjectPlacement m_beforePlacement;
        Struct::GameObjectPlacement m_afterPlacement;
    };
}