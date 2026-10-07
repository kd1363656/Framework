#pragma once

namespace FWK::Editor
{
    class ChangeSelectionCommand final : public ICommand
    {
    public:

         ChangeSelectionCommand(const boost::uuids::uuid&               a_beforeAnchorUUID,
                                const boost::uuids::uuid&               a_afterAnchorUUID,
                                const bool                              a_beforeIsSceneSelected,
                                const bool                              a_afterIsSceneSelected,
                                      std::vector<boost::uuids::uuid>&& a_beforeUUIDList,
                                      std::vector<boost::uuids::uuid>&& a_afterUUIDList);

        ~ChangeSelectionCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        std::vector<boost::uuids::uuid> m_beforeUUIDList;
        std::vector<boost::uuids::uuid> m_afterUUIDList;

        boost::uuids::uuid m_beforeAnchorUUID;
        boost::uuids::uuid m_afterAnchorUUID;

        bool m_beforeIsSceneSelected;
        bool m_afterIsSceneSelected;
    };
}