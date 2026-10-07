#pragma once

namespace FWK::Editor
{
    class DestroyGameObjectCommand final : public ICommand
    {
    public:

         DestroyGameObjectCommand(std::vector<Struct::DestroyedGameObjectRecord>&& a_destroyedGameObjectRecordList);
        ~DestroyGameObjectCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        // 1回の削除操作で取り外したGameObjectの記録(複数選択時は複数)
        std::vector<Struct::DestroyedGameObjectRecord> m_destroyedGameObjectRecordList;
    };
}