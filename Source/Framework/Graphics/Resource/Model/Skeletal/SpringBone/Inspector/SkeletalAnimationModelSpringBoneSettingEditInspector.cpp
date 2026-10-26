#include "SkeletalAnimationModelSpringBoneSettingEditInspector.h"

bool FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditInspector::EditInspector(SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting, const std::weak_ptr<SkeletalAnimationModelRecord>& a_ownerRecord)
{
    const auto& l_ownerRecord = a_ownerRecord.lock();

    if (!l_ownerRecord)
    {
        ImGui::TextUnformatted(k_noModelText.data());

        return false;
    }

    UpdateBoneNameList(l_ownerRecord);

    // 編集していない間に、Undo/Redoや読み込みで設定が変わった場合は、Undo用の「編集前の設定」を取り直す
    if (!m_hasPendingEdit &&
        m_committedRevision != a_springBoneSetting.GetVALRevision())
    {
        m_committedJson     = a_springBoneSetting.Serialize    ();
        m_committedRevision = a_springBoneSetting.GetVALRevision();
    }

    DrawSaveButton(a_springBoneSetting, *l_ownerRecord);

    bool l_isChanged = false;

    l_isChanged = EditGlobalSetting      (a_springBoneSetting) || l_isChanged;
    l_isChanged = EditChainSettingList   (a_springBoneSetting) || l_isChanged;
    l_isChanged = EditColliderSettingList(a_springBoneSetting) || l_isChanged;

    // 値が変わったことを知らせ、揺らす計算側にすぐ反映させる
    if (l_isChanged)
    {
        a_springBoneSetting.MarkChanged();

        m_hasPendingEdit = true;
    }

    // ドラッグ中は1回の編集が続いているため、どの部品も操作されていない状態になってからUndo/Redoへ積む
    // (ドラッグの1フレームごとに積むと、Undoを何十回も押さないと戻らなくなる)
    if (m_hasPendingEdit &&
        !ImGui::IsAnyItemActive())
    {
        CommitUndoCommand(a_springBoneSetting, a_ownerRecord);
    }

    return l_isChanged;
}

void FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditInspector::UpdateBoneNameList(const std::shared_ptr<SkeletalAnimationModelRecord>& a_ownerRecord)
{
    const auto& l_cachedOwnerRecord = m_boneNameListOwnerRecord.lock();

    // 同じモデルなら、前のフレームに作った一覧をそのまま使う(毎フレームの文字列変換を避ける)
    if (l_cachedOwnerRecord == a_ownerRecord) { return; }

    const auto& l_modelData = a_ownerRecord->GetREFModelData();

    m_boneNameList.clear ();
    m_boneWNameList.clear();

    m_boneNameList.reserve (l_modelData.m_boneList.size());
    m_boneWNameList.reserve(l_modelData.m_boneList.size());

    // ImGuiはUTF-8の文字列で表示するため、ボーンの名前(wstring)を変換しておく
    for (const auto& l_bone : l_modelData.m_boneList)
    {
        m_boneNameList.emplace_back (Utility::WStringToString(l_bone.m_boneName));
        m_boneWNameList.emplace_back(l_bone.m_boneName);
    }

    m_boneNameListOwnerRecord = a_ownerRecord;

    // 別のモデルに切り替わったため、Undo用の編集前の設定も取り直す
    m_committedRevision = k_invalidRevision;
    m_hasPendingEdit    = false;
}

bool FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditInspector::EditGlobalSetting(SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting) const
{
    if (!ImGui::CollapsingHeader(k_globalHeaderLabel.data(), ImGuiTreeNodeFlags_DefaultOpen)) { return false; }

    bool l_isChanged = false;

    // ImGui::DragScalarは最小値と最大値をポインタで受け取るため、定数のアドレスを渡す
    if (auto l_prewarmIterationCount = a_springBoneSetting.GetVALPrewarmIterationCount();
        ImGui::DragScalar(k_prewarmLabel.data(),
                          ImGuiDataType_U32,
                          &l_prewarmIterationCount,
                          k_prewarmDragSpeed,
                          &k_prewarmIterationCountMIN,
                          &k_prewarmIterationCountMAX))
    {
        a_springBoneSetting.SetPrewarmIterationCount(l_prewarmIterationCount);

        l_isChanged = true;
    }

    Utility::IMGUIDelayedTooltip(k_prewarmTooltip);

    return l_isChanged;
}

bool FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditInspector::EditChainSettingList(SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting) const
{
    if (!ImGui::CollapsingHeader(k_chainHeaderLabel.data(), ImGuiTreeNodeFlags_DefaultOpen)) { return false; }

    auto& l_chainSettingList = a_springBoneSetting.GetMutableREFChainSettingList();

    bool l_isChanged = false;

    // 繰り返しの途中で消すと番号がずれるため、消すグループは覚えておき最後に消す
    auto l_removeChainSettingIndex = k_invalidListIndex;

    for (std::size_t l_chainSettingIndex = 0ULL; l_chainSettingIndex < l_chainSettingList.size(); ++l_chainSettingIndex)
    {
        auto& l_chainSetting = l_chainSettingList[l_chainSettingIndex];

        // 同じラベルの部品が複数並んでもImGuiが区別できるように、グループの番号をIDとして積む
        ImGui::PushID(static_cast<int>(l_chainSettingIndex));

        const auto& l_headerLabel = FetchVALChainHeaderLabel(l_chainSetting, l_chainSettingIndex);

        if (ImGui::TreeNode(l_headerLabel.c_str()))
        {
            l_isChanged = EditChainSetting(l_chainSetting) || l_isChanged;

            if (ImGui::Button(k_removeButtonLabel.data()))
            {
                l_removeChainSettingIndex = l_chainSettingIndex;
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    if (l_removeChainSettingIndex != k_invalidListIndex)
    {
        l_chainSettingList.erase(l_chainSettingList.begin() + static_cast<std::ptrdiff_t>(l_removeChainSettingIndex));

        l_isChanged = true;
    }

    if (ImGui::Button(k_addChainButtonLabel.data()))
    {
        l_chainSettingList.emplace_back();

        l_isChanged = true;
    }

    return l_isChanged;
}
bool FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditInspector::EditColliderSettingList(SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting) const
{
    if (!ImGui::CollapsingHeader(k_colliderHeaderLabel.data(), ImGuiTreeNodeFlags_DefaultOpen)) { return false; }

    auto& l_colliderSettingList = a_springBoneSetting.GetMutableREFColliderSettingList();

    bool l_isChanged = false;

    auto l_removeColliderSettingIndex = k_invalidListIndex;

    for (std::size_t l_colliderSettingIndex = 0ULL; l_colliderSettingIndex < l_colliderSettingList.size(); ++l_colliderSettingIndex)
    {
        auto& l_colliderSetting = l_colliderSettingList[l_colliderSettingIndex];

        ImGui::PushID(static_cast<int>(l_colliderSettingIndex));

        ImGui::Separator();

        l_isChanged = EditColliderSetting(l_colliderSetting) || l_isChanged;

        if (ImGui::Button(k_removeButtonLabel.data()))
        {
            l_removeColliderSettingIndex = l_colliderSettingIndex;
        }

        ImGui::PopID();
    }

    if (l_removeColliderSettingIndex != k_invalidListIndex)
    {
        l_colliderSettingList.erase(l_colliderSettingList.begin() + static_cast<std::ptrdiff_t>(l_removeColliderSettingIndex));

        l_isChanged = true;
    }

    if (ImGui::Button(k_addColliderButtonLabel.data()))
    {
        l_colliderSettingList.emplace_back();

        l_isChanged = true;
    }

    return l_isChanged;
}

bool FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditInspector::EditChainSetting(Struct::ModelSpringBoneChainSetting& a_chainSetting) const
{
    bool l_isChanged = false;

    l_isChanged = ImGui::Checkbox(k_isEnabledLabel.data(), &a_chainSetting.m_isEnabled) || l_isChanged;

    l_isChanged = EditRootBoneNameList(a_chainSetting.m_rootBoneNameList) || l_isChanged;

    l_isChanged = ImGui::DragFloat(k_stiffnessLabel.data(),
                                   &a_chainSetting.m_stiffness,
                                   k_stiffnessDragSpeed,
                                   k_stiffnessMIN,
                                   k_stiffnessMAX) || l_isChanged;

    Utility::IMGUIDelayedTooltip(k_stiffnessTooltip);

    l_isChanged = ImGui::DragFloat(k_dragLabel.data(),
                                   &a_chainSetting.m_drag,
                                   k_dragDragSpeed,
                                   k_dragMIN,
                                   k_dragMAX) || l_isChanged;

    Utility::IMGUIDelayedTooltip(k_dragTooltip);

    l_isChanged = ImGui::DragFloat(k_gravityPowerLabel.data(),
                                   &a_chainSetting.m_gravityPower,
                                   k_gravityPowerDragSpeed,
                                   k_gravityPowerMIN,
                                   k_gravityPowerMAX) || l_isChanged;

    Utility::IMGUIDelayedTooltip(k_gravityPowerTooltip);

    l_isChanged = ImGui::DragFloat3(k_gravityDirectionLabel.data(),
                                    &a_chainSetting.m_gravityDirection.x,
                                    k_gravityDirectionDragSpeed,
                                    k_gravityDirectionMIN,
                                    k_gravityDirectionMAX) || l_isChanged;

    l_isChanged = ImGui::DragFloat(k_hitRadiusLabel.data(),
                                   &a_chainSetting.m_hitRadius,
                                   k_lengthDragSpeed,
                                   k_lengthMIN,
                                   k_lengthMAX) || l_isChanged;

    Utility::IMGUIDelayedTooltip(k_hitRadiusTooltip);

    l_isChanged = ImGui::DragFloat(k_maxAngularVelocityLabel.data(),
                                   &a_chainSetting.m_maxAngularVelocityDegree,
                                   k_maxAngularVelocityDragSpeed,
                                   k_maxAngularVelocityMIN,
                                   k_maxAngularVelocityMAX) || l_isChanged;

    Utility::IMGUIDelayedTooltip(k_maxAngularVelocityTooltip);

    l_isChanged = ImGui::DragFloat(k_endLengthLabel.data(),
                                   &a_chainSetting.m_endLength,
                                   k_lengthDragSpeed,
                                   k_lengthMIN,
                                   k_lengthMAX) || l_isChanged;

    Utility::IMGUIDelayedTooltip(k_endLengthTooltip);

    l_isChanged = ImGui::Checkbox(k_isAdjacentChainLinkedLabel.data(), &a_chainSetting.m_isAdjacentChainLinked) || l_isChanged;

    Utility::IMGUIDelayedTooltip(k_isAdjacentChainLinkedTooltip);

    // つなぐ設定のときだけ、つなぎ方の設定を出す
    if (a_chainSetting.m_isAdjacentChainLinked)
    {
        l_isChanged = ImGui::Checkbox(k_isAdjacentChainLoopedLabel.data(), &a_chainSetting.m_isAdjacentChainLooped) || l_isChanged;

        Utility::IMGUIDelayedTooltip(k_isAdjacentChainLoopedTooltip);

        l_isChanged = ImGui::DragFloat(k_adjacentLinkStiffnessLabel.data(),
                                       &a_chainSetting.m_adjacentLinkStiffness,
                                       k_adjacentLinkStiffnessDragSpeed,
                                       k_adjacentLinkStiffnessMIN,
                                       k_adjacentLinkStiffnessMAX) || l_isChanged;

        Utility::IMGUIDelayedTooltip(k_adjacentLinkStiffnessTooltip);
    }

    return l_isChanged;
}
bool FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditInspector::EditRootBoneNameList(std::vector<std::wstring>& a_rootBoneNameList) const
{
    bool l_isChanged = false;

    auto l_removeRootBoneIndex = k_invalidListIndex;

    for (std::size_t l_rootBoneIndex = 0ULL; l_rootBoneIndex < a_rootBoneNameList.size(); ++l_rootBoneIndex)
    {
        ImGui::PushID(static_cast<int>(l_rootBoneIndex));

        l_isChanged = SelectBoneName(k_rootBoneLabel, a_rootBoneNameList[l_rootBoneIndex]) || l_isChanged;

        ImGui::SameLine();

        if (ImGui::SmallButton(k_removeButtonLabel.data()))
        {
            l_removeRootBoneIndex = l_rootBoneIndex;
        }

        ImGui::PopID();
    }

    if (l_removeRootBoneIndex != k_invalidListIndex)
    {
        a_rootBoneNameList.erase(a_rootBoneNameList.begin() + static_cast<std::ptrdiff_t>(l_removeRootBoneIndex));

        l_isChanged = true;
    }

    // 名前が空の根元を追加し、選ぶのはコンボボックスで行う(空の間は揺らさない)
    if (ImGui::SmallButton(k_addRootButtonLabel.data()))
    {
        a_rootBoneNameList.emplace_back();

        l_isChanged = true;
    }

    return l_isChanged;
}
bool FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditInspector::EditColliderSetting(Struct::ModelSpringBoneColliderSetting& a_colliderSetting) const
{
    bool l_isChanged = false;

    l_isChanged = SelectBoneName(k_colliderBoneLabel, a_colliderSetting.m_boneName) || l_isChanged;

    // ImGui::Comboは選んだ番号をintで受け取るため、Enumを番号に変えてから渡す
    if (int l_shapeIndex = static_cast<int>(a_colliderSetting.m_shape);
        ImGui::Combo(k_colliderShapeLabel.data(),
                     &l_shapeIndex,
                     k_colliderShapeNameList.data(),
                     static_cast<int>(k_colliderShapeNameList.size())))
    {
        a_colliderSetting.m_shape = static_cast<Enum::ModelSpringBoneColliderShape>(l_shapeIndex);

        l_isChanged = true;
    }

    l_isChanged = ImGui::DragFloat3(k_colliderOffsetLabel.data(), &a_colliderSetting.m_offset.x, k_offsetDragSpeed) || l_isChanged;

    // カプセルのときだけ終点を出す
    if (a_colliderSetting.m_shape == Enum::ModelSpringBoneColliderShape::Capsule)
    {
        l_isChanged = ImGui::DragFloat3(k_colliderTailOffsetLabel.data(), &a_colliderSetting.m_tailOffset.x, k_offsetDragSpeed) || l_isChanged;
    }

    l_isChanged = ImGui::DragFloat(k_colliderRadiusLabel.data(),
                                   &a_colliderSetting.m_radius,
                                   k_lengthDragSpeed,
                                   k_lengthMIN,
                                   k_lengthMAX) || l_isChanged;

    return l_isChanged;
}

bool FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditInspector::SelectBoneName(const std::string_view& a_label, std::wstring& a_boneName) const
{
    // 今選んでいるボーンの番号を探す(見つからない = 未選択、又はこのモデルに無い名前)
    const auto& l_itr = std::ranges::find(m_boneWNameList, a_boneName);

    const auto l_selectedBoneIndex = l_itr == m_boneWNameList.end() ? k_invalidListIndex : static_cast<std::size_t>(std::distance(m_boneWNameList.begin(), l_itr));

    const auto* l_previewText = l_selectedBoneIndex == k_invalidListIndex ? k_unselectedBoneText.data() : m_boneNameList[l_selectedBoneIndex].c_str();

    if (!ImGui::BeginCombo(a_label.data(), l_previewText, ImGuiComboFlags_HeightLarge)) { return false; }

    bool l_isChanged = false;

    for (std::size_t l_boneIndex = 0ULL; l_boneIndex < m_boneNameList.size(); ++l_boneIndex)
    {
        const bool l_isSelected = l_boneIndex == l_selectedBoneIndex;

        if (ImGui::Selectable(m_boneNameList[l_boneIndex].c_str(), l_isSelected))
        {
            a_boneName = m_boneWNameList[l_boneIndex];

            l_isChanged = true;
        }

        // 開いたときに、選んでいるボーンの位置までスクロールさせる
        if (l_isSelected) { ImGui::SetItemDefaultFocus(); }
    }

    ImGui::EndCombo();

    return l_isChanged;
}

void FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditInspector::CommitUndoCommand(SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting, const std::weak_ptr<SkeletalAnimationModelRecord>& a_ownerRecord)
{
    auto l_afterJson  = a_springBoneSetting.Serialize();
    auto l_beforeJson = m_committedJson;

    m_committedJson     = l_afterJson;
    m_committedRevision = a_springBoneSetting.GetVALRevision();
    m_hasPendingEdit    = false;

    // 値を動かしたが元の値に戻した場合は、Undoしても何も変わらないため積まない
    if (l_beforeJson == l_afterJson) { return; }

    auto& l_editorManager  = Editor::EditorManager::GetInstance      ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<SkeletalAnimationModelSpringBoneSettingEditCommand>(a_ownerRecord, std::move(l_beforeJson), std::move(l_afterJson));
}

void FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditInspector::DrawSaveButton(SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting, const SkeletalAnimationModelRecord& a_ownerRecord) const
{
    if (ImGui::Button(k_saveButtonLabel.data()))
    {
        // .springファイルはFBXと同じ場所に、拡張子だけを変えて保存する
        const std::filesystem::path l_modelFilePath = a_ownerRecord.GetREFFilePath();

        FWK_ASSERT_RETURN_IF(!a_springBoneSetting.SaveBinaryData(l_modelFilePath), "揺れものの設定(.spring)の保存に失敗しました。");
    }

    if (!a_springBoneSetting.GetVALIsUnsaved()) { return; }

    ImGui::SameLine();

    ImGui::TextUnformatted(k_unsavedText.data());
}

std::string FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditInspector::FetchVALChainHeaderLabel(const Struct::ModelSpringBoneChainSetting& a_chainSetting, const std::size_t a_chainSettingIndex) const
{
    // 見出しには、最初の根元のボーンの名前を出す(どのボーンのグループか分かるように)
    // ImGuiは「###」より後ろだけをIDに使うため、名前を変えても開いた状態のまま保たれる
    const auto& l_firstRootBoneName = a_chainSetting.m_rootBoneNameList.empty() ? std::string{ k_unselectedBoneText } : Utility::WStringToString(a_chainSetting.m_rootBoneNameList.front());

    return std::format(k_chainHeaderFormat, a_chainSettingIndex, l_firstRootBoneName);
}