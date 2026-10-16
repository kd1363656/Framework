#pragma once

namespace FWK::Editor
{
    class ReparentGameObjectCommand final : public ICommand
    {
    public:

         ReparentGameObjectCommand(const Struct::GameObjectPlacement& a_beforePlacement, const Struct::GameObjectPlacement& a_afterPlacement, const boost::uuids::uuid& a_gameObjectUUID);
        ~ReparentGameObjectCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        void ApplyPlacement(const Struct::GameObjectPlacement& a_placement) const;

        Struct::GameObjectPlacement m_beforePlacement;
        Struct::GameObjectPlacement m_afterPlacement;

        boost::uuids::uuid m_gameObjectUUID;
    };
}