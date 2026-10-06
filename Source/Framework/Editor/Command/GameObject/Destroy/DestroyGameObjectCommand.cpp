#include "DestroyGameObjectCommand.h"

FWK::Editor::DestroyGameObjectCommand::DestroyGameObjectCommand(std::vector<Struct::DestroyedGameObjectRecord>&& a_destroyedGameObjectRecordList) :
    m_destroyedGameObjectRecordList(std::move(a_destroyedGameObjectRecordList))
{}
FWK::Editor::DestroyGameObjectCommand::~DestroyGameObjectCommand() = default;

void FWK::Editor::DestroyGameObjectCommand::Undo()
{
}
void FWK::Editor::DestroyGameObjectCommand::Redo()
{
}