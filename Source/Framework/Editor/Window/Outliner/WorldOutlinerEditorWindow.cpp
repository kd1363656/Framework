#include "WorldOutlinerEditorWindow.h"

void FWK::Editor::WorldOutlinerEditorWindow::Draw(EditorManager& a_editorManager)
{
    // Outliner用ImGuiウィンドウを開始
    if (!ImGui::Begin(k_editorName.data()))
    {
        ImGui::End();

        return;
    }

    ReportActiveWindowIfMouseClicked(a_editorManager);

    Utility::IMGUIDelayedTooltip(k_thisWindowExplanationLabel);

    // SceneManagerが現在所有しているSceneをweak_ptrから取得する
    // lock()したshared_ptrはこのDraw()の間だけSceneの生存を保証する
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene)
    {
        // TextDisabled()は通常のText()より薄い色で文字を描画する
        // Scene未読み込みはエラーではないため、警告色ではなく補助表示にする
        ImGui::TextDisabled(k_noCurrentSceneLabel.data());

        // Sceneが存在しなくても
        // Outlinerの空白クリックによる選択解除は許可する
        ProcessEmptyAreaClick();

        ImGui::End();

        return;
    }

    DrawSceneNode(*l_scene);

    // Treeの描画が終了した後に残った領域を
    // 空白クリック領域として使用する
    ProcessEmptyAreaClick();

    ImGui::End();
}

void FWK::Editor::WorldOutlinerEditorWindow::DrawSceneNode(const Scene& a_scene)
{
    // Scene直下へ描画可能なRootGameObjectがある場合はTreeNode
    // 1つも存在しない場合はLeafとして描画する
    const bool l_hasDrawableRootGameObject = HasDrawableRootGameObject(a_scene);

    ImGuiTreeNodeFlags l_treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth;

    if (l_hasDrawableRootGameObject)
    {
        // 矢印部分をクリックした場合にSceneを開閉できるようにする
        l_treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
    }
    else
    {
        // Childを持たないSceneはLeafとして描画する
        // NoTreePushOnOpenを指定したLeafは内部でIndentをPushしないため
        // TreePop()も必要ない
        l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf |
                           ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    const auto& l_sceneName       = a_scene.GetREFSceneName();
    const bool  l_isSceneNodeOpen = ImGui::TreeNodeEx      (l_sceneName.empty() ? k_emptySceneLabel.data() : l_sceneName.c_str(), l_treeNodeFlags);

    // RootGameObjectが存在しなSceneはLeafなので
    // これ以上描画するものはない
    if (!l_hasDrawableRootGameObject) { return; }

    // Sceneノードが閉じられている場合、
    // ChildとなるGameObjectを描画する必要はない
    if (!l_isSceneNodeOpen) { return; }

    const auto& l_gameObjectList = a_scene.GetREFGameObjectList();

    for (const auto& l_gameObject : l_gameObjectList)
    {
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }

        // Scene直下にはParentを持たないRootGameObjectだけを描画する
        // Parentを持っているGameObjectは、
        // 親GameObjectのDrawGameObjectNode()から再帰的に描画する
        const auto& l_hierarchy = l_gameObject->GetREFHierarchy();

        if (const auto& l_parentGameObject = l_hierarchy.GetREFParent();
            !l_parentGameObject.expired())
        {
            continue;
        }

        DrawGameObjectNode(l_gameObject);
    }

    // TreeNodeEx()によってScene階層へ入っているため
    // TreePop()で一階層戻す
    ImGui::TreePop();
}
void FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNode(const std::weak_ptr<GameObject>& a_gameObject)
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject ||
        l_gameObject->GetVALIsDestroyed())
    {
        return;
    }

    // 有効なChildが存在する場合だけTreeNode、
    // Childを持たない場合はLeafとして描画する
    const bool l_hasDrawableChildGameObject = HasDrawableChildGameObject(*l_gameObject);

    ImGuiTreeNodeFlags l_treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth;

    if (l_hasDrawableChildGameObject)
    {
        l_treeNodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow;
    }
    else
    {
        l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf |
                           ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    const auto& l_gameObjectName = l_gameObject->GetREFName();

    // 同じ名前のGameObjectが複数存在すると
    // ImGuiは同じIDのItemとして扱ってしまう
    // PushID()でGameObjectのアドレスを一時的なIDとして利用し、
    // 同名GameObjectでも別のTreeNodeとして識別できるようにする
    ImGui::PushID(l_gameObject.get());

    const bool l_isGameObjectNodeOpen = ImGui::TreeNodeEx(l_gameObjectName.c_str(), l_treeNodeFlags);

    // Childを持っていて、なおかつTreeNodeが開いている場合だけ
    // Childを再帰描画する
    if (l_hasDrawableChildGameObject &&
        l_isGameObjectNodeOpen)
    {
        const auto& l_childSmartPointerVectorList = l_gameObject->GetREFHierarchy().GetREFChildSmartPointerVectorList();
        const auto& l_childDataList               = l_childSmartPointerVectorList.GetREFElementDataList              ();

        for (const auto& l_childData : l_childDataList)
        {
            const auto& l_childGameObject = l_childData.m_type.lock();

            if (!l_childGameObject ||
                l_childGameObject->GetVALIsDestroyed())
            {
                continue;
            }

            DrawGameObjectNode(l_childGameObject);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}

void FWK::Editor::WorldOutlinerEditorWindow::ProcessEmptyAreaClick() const
{
    // Tree描画後に残っているWindow内の領域サイズを取得する
    const auto& l_availableContentRegion = ImGui::GetContentRegionAvail();

    // 描画できる領域がない場合はInvisibleButtonを作らない
    if (l_availableContentRegion.x <= k_minEmptyAreaSize ||
        l_availableContentRegion.y <= k_minEmptyAreaSize)
    {
        return;
    }

    // InvisibleButton()は見た目を何も描画せず
    // Mouse入力を受け取るItemだけを作成する
    // TreeNodeより下に残った領域へ配置することで
    // Outlinerの空白部分をクリック下を正確に判定できる
    ImGui::InvisibleButton(k_emptyAreaLabel.data(), l_availableContentRegion);
}

bool FWK::Editor::WorldOutlinerEditorWindow::HasDrawableRootGameObject(const Scene& a_scene) const
{
    const auto& l_gameObjectList = a_scene.GetREFGameObjectList();

    // SceneがTreeNodeなのかLeafなのかをTreeNodeEx()より前に
    // 決める必要があるため、描画可能Rootが一つ存在するかだけ確認する
    // 一つ見つかった時点でreturnするため
    // 必要以上にリスト全体を捜査しない
    return std::ranges::any_of(l_gameObjectList,
                               [](const auto& a_gameObject)
                               {
                                    if (!a_gameObject ||
                                        a_gameObject->GetVALIsDestroyed())
                                    {
                                        return false;
                                    }

                                    return a_gameObject->GetREFHierarchy().GetREFParent().expired();
                               });
}
bool FWK::Editor::WorldOutlinerEditorWindow::HasDrawableChildGameObject(const GameObject& a_gameObject) const
{
    const auto& l_childSmartPointerVectorList = a_gameObject.GetREFHierarchy().GetREFChildSmartPointerVectorList();
    const auto& l_childDataList               = l_childSmartPointerVectorList.GetREFElementDataList             ();

    // TreeNodeEx()を呼ぶより前にLeafかどうかを判断するため、
    // 描画可能Childが一つ存在するかだけ確認する
    return std::ranges::any_of(l_childDataList,
                               [](const auto& a_childData)
                               {
                                     const auto& l_childGameObject = a_childData.m_type.lock();
                               
                                     return l_childGameObject &&
                                            !l_childGameObject->GetVALIsDestroyed();
                               });
}