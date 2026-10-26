#include "TransformGameObjectCommand.h"

FWK::Editor::TransformGameObjectCommand::TransformGameObjectCommand(std::vector<Struct::TransformGameObjectRecord>&& a_recordList) :
    m_recordList(std::move(a_recordList))
{}
FWK::Editor::TransformGameObjectCommand::~TransformGameObjectCommand() = default;

void FWK::Editor::TransformGameObjectCommand::Undo()
{
    ApplyTransform(k_applyBeforeTransform);
}
void FWK::Editor::TransformGameObjectCommand::Redo()
{
    ApplyTransform(k_applyAfterTransform);
}

void FWK::Editor::TransformGameObjectCommand::ApplyTransform(const bool a_isBefore) const
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    for (const auto& l_record : m_recordList)
    {
        const auto& l_gameObject = l_scene->FindVALGameObject(l_record.m_sceneInstanceUUID).lock();

        // 削除されたGameObjectは戻せないので飛ばす
        if (!l_gameObject) { continue; }

        const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

        if (!l_transformComponent) { continue; }

        auto& l_transform = l_transformComponent->GetMutableREFTransform();

        l_transform = a_isBefore ? l_record.m_beforeTransform : l_record.m_afterTransform;

        // 行列はPostLateUpdateで作り直される(子の行列も合わせて作り直される)
        l_transformComponent->SetShouldUpdateMatrixDirty(true);
    }
}
