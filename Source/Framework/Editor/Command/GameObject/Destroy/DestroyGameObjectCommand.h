#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Editor
{
    class DestroyGameObjectCommand final : public ICommand
    {
    public:

         DestroyGameObjectCommand(const std::vector<std::weak_ptr<GameObject>>& a_destroyGameObjectList);
        ~DestroyGameObjectCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        std::vector<GameObjectCommandRecord> m_recordList;
    };
}