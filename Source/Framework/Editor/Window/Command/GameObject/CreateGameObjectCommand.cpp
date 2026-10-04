#include "CreateGameObjectCommand.h"

FWK::Editor::CreateGameObjectCommand::CreateGameObjectCommand(std::vector<std::weak_ptr<GameObject>>&& a_createdGameObjectList, boost::uuids::uuid a_parentUUID) :
    m_parentUUID(std::move(a_parentUUID))
{
    // Undoでシーンから取り外しても実体が消えないよう
    // shared_ptrで保持する
    m_createdGameObjectList.reserve(a_createdGameObjectList.size());

    for (const auto& l_createdWeak : a_createdGameObjectList)
    {
        if (const auto& l_created = l_createdWeak.lock();
            l_created)
        {
            m_createdGameObjectList.emplace_back(l_created);
        }
    }
}
FWK::Editor::CreateGameObjectCommand::~CreateGameObjectCommand() = default;

void FWK::Editor::CreateGameObjectCommand::Undo()
{
    
}

void FWK::Editor::CreateGameObjectCommand::Redo()
{

}