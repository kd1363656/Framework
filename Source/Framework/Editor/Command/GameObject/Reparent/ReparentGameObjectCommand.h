#pragma once

namespace FWK::Editor
{
    class ReparentGameObjectCommand final : public ICommand
    {
    public:

         ReparentGameObjectCommand(const boost::uuids::uuid& a_gameObjectUUID, const Struct::ReparentGameObjectState& a_beforeState, const Struct::ReparentGameObjectState& a_afterState);

        ~ReparentGameObjectCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        void ApplyState(const Struct::ReparentGameObjectState& a_state);

        boost::uuids::uuid m_gameObjectUUID = {};

        Struct::ReparentGameObjectState m_beforeState = {};
        Struct::ReparentGameObjectState m_afterState  = {};
    };
}