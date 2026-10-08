#include "PasteCutGameObjectCommand.h"

FWK::Editor::PasteCutGameObjectCommand::PasteCutGameObjectCommand(const std::vector<std::weak_ptr<GameObject>>& a_pastedGameObjectList, std::vector<Struct::DestroyedGameObjectRecord>&& a_destroyedGameObjectRecordList) :
    m_createGameObjectCommand (a_pastedGameObjectList),
    m_destroyGameObjectCommand(std::move(a_destroyedGameObjectRecordList))
{}
FWK::Editor::PasteCutGameObjectCommand::~PasteCutGameObjectCommand() = default;

void FWK::Editor::PasteCutGameObjectCommand::Undo()
{
    // 貼り付けで生成した複製を取り外してから、Cutした元のGameObjectをシーンへ戻す
    // 複製を取り外してから元のゲームオブジェクトを戻す
    m_createGameObjectCommand.Undo ();
    m_destroyGameObjectCommand.Undo();
}
void FWK::Editor::PasteCutGameObjectCommand::Redo()
{
    // Cutした元のGameObjectを取り外してから、貼り付けた複製をシーンへ戻す
    // 元のゲームオブジェクトを取り外してから複製を戻す
    m_destroyGameObjectCommand.Redo();
    m_createGameObjectCommand.Redo ();
}