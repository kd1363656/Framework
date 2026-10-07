#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Editor
{
    class PasteCutGameObjectCommand final : public ICommand
    {
    public:

         PasteCutGameObjectCommand(const std::vector<std::weak_ptr<GameObject>>& a_pastedGameObjectList, std::vector<Struct::DestroyedGameObjectRecord>&& a_destroyedGameObjectRecordList);
        ~PasteCutGameObjectCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        CreateGameObjectCommand  m_createGameObjectCommand;
        DestroyGameObjectCommand m_destroyGameObjectCommand;
    };
}