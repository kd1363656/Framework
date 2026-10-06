#pragma once

namespace FWK::Editor
{
    class ReparentGameObjectCommand final : public ICommand
    {
    public:
 
         ReparentGameObjectCommand(const boost::uuids::uuid& a_gameObjectUUID, const boost::uuids::uuid& a_beforeParentUUID, const boost::uuids::uuid& a_afterParentUUID);
 
        ~ReparentGameObjectCommand() override;
 
        void Undo() override;
        void Redo() override;
 
    private:
 
        void ApplyParent(const boost::uuids::uuid& a_parentUUID);

        boost::uuids::uuid m_gameObjectUUID   = {};
        boost::uuids::uuid m_beforeParentUUID = {};
        boost::uuids::uuid m_afterParentUUID  = {};
    };
}