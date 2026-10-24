#include "Scene.h"
#include "../../Application/Application.h"

void FWK::Scene::INIT()
{
    m_gameObjectList.clear              ();
    m_pendingAddGameObjectList.clear    ();
    m_gameObjectExecutionLevelList.clear();

    m_gameObjectUUIDRegistry.Clear();

    m_gameObjectPrefabSystem.INIT();

    m_lightSystem.ApplyDefaultSettings();

    m_name.clear();

    m_nextSceneUUID = {};
}

void FWK::Scene::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Jsonの中身が空となっておりシーンのデシリアライズ処理に失敗しました。");
        return;
    }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Scene::PostDeserialize()
{
    // ※負荷テスト専用 : モデルを500体並べて描画する
    SetupPerformanceTest();

    if (m_gameObjectExecutionLevelList.empty()) { return; }

    // ルートGameObjectのみを対象にする
    // 子孫は各GameObjectのHierarchy経由のPostDeserializeに任せる
    // (全ノードを回すと再帰済みの子が二重実行される)
    const auto& l_rootGameObjectLevel = m_gameObjectExecutionLevelList.front();

    for (const auto& l_gameObjectWeak : l_rootGameObjectLevel)
    {
        const auto& l_gameObject = l_gameObjectWeak.lock();

        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }

        l_gameObject->PostDeserialize();
    }

    auto& l_physicsManager = Physics::PhysicsManager::GetInstance();

    // StaticBodyをまとめて追加した後なので、BroadPhaseを最適化しておく。
    // 毎フレーム呼ぶものではなく、ステージ読み込み後などに呼ぶ。
    l_physicsManager.OptimizeBroadPhase();
}

void FWK::Scene::EarlyUpdate() 
{
    // 削除要求があったゲームオブジェクトがあればここで削除
    RemoveDestroyedGameObjects();

    // 追加要求があったゲームオブジェクトがあればここで追加
    AddPendingGameObjects();

    // ※負荷テスト専用 : ウィンドウサイズが変わったらカメラの縦横比を合わせ、アニメーションの再生時刻を進める
    UpdatePerformanceTestCameraAspectRatio();
    UpdatePerformanceTestAnimationTime    ();

    for (const auto& l_gameObjectExecutionLevel : m_gameObjectExecutionLevelList)
    {
        for (const auto& l_gameObjectWeak : l_gameObjectExecutionLevel)
        {
            const auto& l_gameObject = l_gameObjectWeak.lock();

            if (!l_gameObject ||
                l_gameObject->GetVALIsDestroyed())
            {
                continue;
            }

            l_gameObject->EarlyUpdate();
        }
    }
}
void FWK::Scene::Update() const
{
    for (const auto& l_gameObjectExecutionLevel : m_gameObjectExecutionLevelList)
    {
        for (const auto& l_gameObjectWeak : l_gameObjectExecutionLevel)
        {
            const auto& l_gameObject = l_gameObjectWeak.lock();

            if (!l_gameObject ||
                l_gameObject->GetVALIsDestroyed())
            {
                continue;
            }

            l_gameObject->Update();
        }
    }
}
void FWK::Scene::LateUpdate() const
{
    for (const auto& l_gameObjectExecutionLevel : m_gameObjectExecutionLevelList)
    {
        for (const auto& l_gameObjectWeak : l_gameObjectExecutionLevel)
        {
            const auto& l_gameObject = l_gameObjectWeak.lock();

            if (!l_gameObject ||
                l_gameObject->GetVALIsDestroyed())
            {
                continue;
            }

            l_gameObject->LateUpdate();
        }
    }
}
void FWK::Scene::PostLateUpdate() const
{
    for (const auto& l_gameObjectExecutionLevel : m_gameObjectExecutionLevelList)
    {
        for (const auto& l_gameObjectWeak : l_gameObjectExecutionLevel)
        {
            const auto& l_gameObject = l_gameObjectWeak.lock();

            if (!l_gameObject ||
                l_gameObject->GetVALIsDestroyed())
            {
                continue;
            }

            l_gameObject->PostLateUpdate();
        }
    }
}

nlohmann::json FWK::Scene::Serialize()
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Scene::RebuildGameObjectExecutionLevelList()
{
    m_gameObjectExecutionLevelList.clear();

    // UUIDRegistryには既に登録済みなので、
    // m_gameObjectList内の順番にGameObjectの現在改装を求める
    for (const auto& l_gameObject : m_gameObjectList)
    {
        if (!l_gameObject ||
            l_gameObject->GetVALIsDestroyed())
        {
            continue;
        }

        const auto& l_executionLevel = CalculateGameObjectExecutionLevel(l_gameObject);

        // 階層を調べてから適した階層に追加
        AddGameObjectToExecutionLevelList(l_gameObject, l_executionLevel);
    }
}

void FWK::Scene::AddGameObject(const std::shared_ptr<GameObject>& a_gameObject)
{
    if (!a_gameObject)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectクラスが無効となっており、ゲームオブジェクトの追加処理に失敗しました。");
        return;
    }

    // SceneInstanceUUIDがnilの場合は新規発行する
    if (a_gameObject->GetREFSceneInstanceUUID().is_nil())
    {
        auto& l_uuidManager = Utility::UUIDManager::GetInstance();
 
        a_gameObject->SetSceneInstanceUUID(l_uuidManager.GenerateVALUUID());
    }
    
    // GameObject自身を識別するSceneInstanceUUIDをUUIDRegistryへ登録する
    // 新規GameObjectでUUIDがnilの場合はUUIDRegistry内で新規発行する
    // Deserialize済みで既にUUIDを持っている場合は、そのUUIDを維持したまま
    // Registry内で重複していないことを確認して登録する
    FWK_ASSERT_RETURN_IF(!m_gameObjectUUIDRegistry.Add(a_gameObject, a_gameObject->GetREFSceneInstanceUUID()),"GameObjectのSceneInstanceUUID登録に失敗したため、GameObjectをSceneへ追加できませんでした。");

    // 計算済みの階層へ直接追加する
    m_pendingAddGameObjectList.emplace_back(a_gameObject);
}

void FWK::Scene::RemoveGameObject(const std::weak_ptr<GameObject>& a_gameObject)
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }

    // 破棄フラグを立てずにシーン管理から外す
    // 実体は呼び出し側が保持し続けるため再追加も可能
    if (const auto& l_sceneInstanceUUID = l_gameObject->GetREFSceneInstanceUUID();
        !l_sceneInstanceUUID.is_nil())
    {
        m_gameObjectUUIDRegistry.Erase(l_sceneInstanceUUID);
    }

    std::erase(m_gameObjectList,           l_gameObject);
    std::erase(m_pendingAddGameObjectList, l_gameObject);
 
    // 実行階層リストからも外す
    for (auto& l_gameObjectExecutionLevel : m_gameObjectExecutionLevelList)
    {
        std::erase_if(l_gameObjectExecutionLevel,
                      [&l_gameObject](const auto& a_gameObjectWeak)
                      {
                          return a_gameObjectWeak.lock() == l_gameObject;
                      });
    }
 
    // 親がいるなら親子関係も解除する
    auto& l_hierarchy = l_gameObject->GetMutableREFHierarchy();

    l_hierarchy.ClearParent();
}

std::weak_ptr<FWK::GameObject> FWK::Scene::FindVALGameObject(const boost::uuids::uuid& a_sceneInstanceUUID) const
{
    return m_gameObjectUUIDRegistry.FindVALRegisteredType(a_sceneInstanceUUID);
}

std::filesystem::path FWK::Scene::FetchVALNextLoadSceneFilePath() const
{
    // 次のに移行するシーンの名前が空なら移行しない
    if (m_nextSceneUUID.is_nil()) { return {}; }

    const auto& l_application           = Application::GetInstance                        ();
    const auto& l_assetFilePathRegistry = l_application.GetREFAssetFilePathRegistry       ();
    const auto* l_assetFilePathData     = l_assetFilePathRegistry.FindPTRAssetFilePathData(m_nextSceneUUID);

    if (!l_assetFilePathData)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "次のシーンへの遷移用のUUIDが無効です、SceneManagerのマップ内部を確認してください。");

        return {};
    }

    // 次のシーンのファイルパスが空なら移行しない
    if (l_assetFilePathData->m_assetFilePath.empty() ||
        l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Scene)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "次のシーンへの情報が無効です、SceneManagerのマップ内部を確認してください。");

        return {};
    }

    // AssetFilePathDataから次に読み込むファイルのパスをreturn
    return l_assetFilePathData->m_assetFilePath;
}

std::string FWK::Scene::FetchVALNextSceneName() const
{
    // 次のに移行するシーンの名前が空なら移行しない
    if (m_nextSceneUUID.is_nil()) { return {}; }

    const auto* l_nextScene = m_sceneChanger.FetchPTRNexScene(m_nextSceneUUID);

    if (!l_nextScene) { return {}; }

    return l_nextScene->GetREFName();
}

void FWK::Scene::AddGameObjectToExecutionLevelList(const std::weak_ptr<GameObject>& a_gameObject, const std::size_t& a_executionLevel)
{
    if (a_gameObject.expired()) { return; }

    // 指定された実行階層までリストを追加
    while (m_gameObjectExecutionLevelList.size() <= a_executionLevel)
    {
        m_gameObjectExecutionLevelList.emplace_back();
    }

    m_gameObjectExecutionLevelList[a_executionLevel].emplace_back(a_gameObject);
}

void FWK::Scene::RemoveDestroyedGameObjects()
{
    // Scene所有リストから削除
    std::erase_if(m_gameObjectList, 
                 [this](const auto& a_gameObject)
                 {
                     if (!a_gameObject) { return true; }
                 
                     a_gameObject->GetREFSceneInstanceUUID();

                     if (a_gameObject->GetVALIsDestroyed())
                     {
                         // UUIDの登録解除に失敗しても(=もともと登録されていなかった)、
                         // 破棄済みのGameObjectはリストから取り除く
                         // ここでfalseを返すと、破棄済みGameObjectがリストに永久に残り続けるため
                         // 失敗は報告だけ行い、下のreturnでtrue(削除する)を返す
                         if (!m_gameObjectUUIDRegistry.Erase(a_gameObject->GetREFSceneInstanceUUID()))
                         {
                             FWK_ASSERT_REPORT("削除対象GameObjectのUUID登録解除に失敗しました。");
                         }
                     }
                 
                     return a_gameObject->GetVALIsDestroyed();
                 });

    // 階層実行リストから削除
    for (auto& l_gameObjectExecutionLevel : m_gameObjectExecutionLevelList)
    {
        std::erase_if(l_gameObjectExecutionLevel, 
                      [](const auto& a_gameObjectWeak)
                      {
                          const auto& l_gameObject = a_gameObjectWeak.lock();
                      
                          // weak_ptrの参照先がなくなっている場合も実行リストから削除する
                          return !l_gameObject;
                      });
    }

    // 最後尾が空にならずそれより上の階層が空になる現象などありえないため
    // pop_backを使用する。
    // 後方に残った空階層を削除
    // 階層が0,1,2,3とあった時に急に2を含む要素が
    // 消えてしまっても問題がないようにpop_backで後ろから削除
    while (!m_gameObjectExecutionLevelList.empty() &&
           m_gameObjectExecutionLevelList.back().empty())
    {
        m_gameObjectExecutionLevelList.pop_back();
    }
}
void FWK::Scene::AddPendingGameObjects()
{
    // GameObjectがGameObjectを生成するトリガーだった場合でも大丈夫なように
    // 遅延追加方式を取る
    for (const auto& l_gameObject : m_pendingAddGameObjectList)
    {
        m_gameObjectList.emplace_back(l_gameObject);

        // 親GameObjectは、子GameObjectより先に
        // Sceneへ登録されなければならない
        const auto& l_executionLevel = CalculateGameObjectExecutionLevel(l_gameObject);

        // 親子階層の深さによってレイヤーを分ける親が先に実行され子が後に実行されるべきという思想の元
        // このような形の処理を取っている
        AddGameObjectToExecutionLevelList(l_gameObject, l_executionLevel);
    }

    // すべて追加し終われば追加予約リストをクリア
    m_pendingAddGameObjectList.clear();
}

std::size_t FWK::Scene::CalculateGameObjectExecutionLevel(const std::weak_ptr<GameObject>& a_gameObject) const
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return k_initialExecutionLevel; }

    // 親を辿って階層の深さを実行レベルとする
    // AddGameObjectは親→子の順に呼ばれるため、
    // この時点の親鎖はすべて登録済み
    std::size_t l_executionLevel = k_initialExecutionLevel;

    const auto& l_hierarchy = l_gameObject->GetREFHierarchy();
          auto  l_parent    = l_hierarchy.GetREFParent     ().lock();

    // 親がいない = ルートゲームオブジェクトなので
    // ルートが見つかるまでl_executionLevelを加算する
    while (l_parent)
    {
        ++l_executionLevel;

        const auto& l_parentHierarchy = l_parent->GetREFHierarchy();

        l_parent = l_parentHierarchy.GetREFParent().lock();
    }
    
    return l_executionLevel;
}

void FWK::Scene::SetupPerformanceTest()
{
    // 前回の負荷テストの描画申請データが残っていれば破棄する
    // (描画申請側はweak_ptrで参照しているため、ここで破棄すれば自動的に描画対象から外れる)
    m_performanceTestDrawRequestDataList.clear        ();
    m_performanceTestSkeletalDrawRequestDataList.clear();

    // StaticModelとSkeletalAnimationModelのキャッシュは拡張子(.staticModel / .skeletalModel)で別ファイルになったが、
    // 負荷テストではモデルの種類ごとの負荷を測りたいので、一度に読み込むのはどちらか片方だけにする
    switch (k_performanceTestModelKind)
    {
    case PerformanceTestModelKind::Static:
        SetupPerformanceTestStaticModel();
        break;

    case PerformanceTestModelKind::SkeletalGPUSkinning:
        SetupPerformanceTestSkeletalModel();
        break;
    }
}

void FWK::Scene::SetupPerformanceTestStaticModel()
{
    FWK_ASSERT_RETURN_IF(!m_performanceTestStaticModel.Load(k_performanceTestModelFilePath), "負荷テスト用モデルの読み込みに失敗しました。");

    const auto& l_staticModelRecord = m_performanceTestStaticModel.GetREFStaticModelRecord().lock();

    FWK_ASSERT_RETURN_IF(!l_staticModelRecord, "負荷テスト用モデルのStaticModelRecordが無効です。");

    // モデルの大きさから、モデル同士の間隔とカメラの位置を決める
    const auto l_layout = CalculatePerformanceTestLayout(l_staticModelRecord->GetREFModelData().m_modelMeshList);

    FWK_ASSERT_RETURN_IF(l_layout.m_meshletCount == 0ULL || l_layout.m_spacing <= 0.0F, "負荷テスト用モデルに頂点が存在しません。");

          auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance ();
          auto& l_renderer        = l_graphicsManager.GetMutableREFRenderer();
    const auto& l_renderGraph     = l_renderer.GetREFRenderGraph           ();

    // 通常描画(ライティングあり)と影描画の両方へ同じ描画申請データを登録する
    const auto& l_litDrawRequest    = l_renderGraph.FindVALDrawRequestPerObject<Graphics::StaticModelStandardLitPerObjectDrawRequest>  ().lock();
    const auto& l_shadowDrawRequest = l_renderGraph.FindVALDrawRequestPerObject<Graphics::StaticModelCascadeShadowPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_litDrawRequest,    "StaticModelStandardLitPerObjectDrawRequestが見つからないため、負荷テストを開始できません。");
    FWK_ASSERT_RETURN_IF(!l_shadowDrawRequest, "StaticModelCascadeShadowPerObjectDrawRequestが見つからないため、負荷テストを開始できません。");

    m_performanceTestDrawRequestDataList.reserve(k_performanceTestInstanceCountX * k_performanceTestInstanceCountZ);

    for (std::size_t l_indexZ = 0ULL; l_indexZ < k_performanceTestInstanceCountZ; ++l_indexZ)
    {
        for (std::size_t l_indexX = 0ULL; l_indexX < k_performanceTestInstanceCountX; ++l_indexX)
        {
            auto l_drawRequestData = std::make_shared<Struct::StaticModelPerObjectDrawRequestData>();

            l_drawRequestData->m_staticModelRecord           = l_staticModelRecord;
            l_drawRequestData->m_worldMatrix                 = TypeAlias::Math::Matrix::CreateTranslation(CalculatePerformanceTestPosition(l_layout, l_indexX, l_indexZ));
            l_drawRequestData->m_worldInverseTransposeMatrix = l_drawRequestData->m_worldMatrix.Invert().Transpose();
            l_drawRequestData->m_worldMaxScale               = k_performanceTestWorldMaxScale;

            l_litDrawRequest->AddDrawRequest   (l_drawRequestData);
            l_shadowDrawRequest->AddDrawRequest(l_drawRequestData);

            m_performanceTestDrawRequestDataList.emplace_back(l_drawRequestData);
        }
    }

    // オクルージョンカリングの確認用 : 並びの手前に巨大なモデルを1体置き、後ろのモデルを隠す
    // (通常の計測では使わないため、k_isPerformanceTestOccluderEnabledがtrueのときだけ置く)
    if (k_isPerformanceTestOccluderEnabled)
    {
        const auto l_occluderScaleMatrix       = TypeAlias::Math::Matrix::CreateScale(k_performanceTestOccluderScale);
        const auto l_occluderTranslationMatrix = TypeAlias::Math::Matrix::CreateTranslation(0.0F, -l_layout.m_boundsMIN.y * k_performanceTestOccluderScale, -l_layout.m_gridDepth * k_half);

        auto l_drawRequestData = std::make_shared<Struct::StaticModelPerObjectDrawRequestData>();

        l_drawRequestData->m_staticModelRecord           = l_staticModelRecord;
        l_drawRequestData->m_worldMatrix                 = l_occluderScaleMatrix * l_occluderTranslationMatrix;
        l_drawRequestData->m_worldInverseTransposeMatrix = l_drawRequestData->m_worldMatrix.Invert().Transpose();
        l_drawRequestData->m_worldMaxScale               = k_performanceTestOccluderScale;

        l_litDrawRequest->AddDrawRequest   (l_drawRequestData);
        l_shadowDrawRequest->AddDrawRequest(l_drawRequestData);

        m_performanceTestDrawRequestDataList.emplace_back(l_drawRequestData);
    }

    SetupPerformanceTestCamera  (l_layout);
    AddPerformanceTestReportNote(l_layout);
}

void FWK::Scene::SetupPerformanceTestSkeletalModel()
{
    FWK_ASSERT_RETURN_IF(!m_performanceTestSkeletalAnimationModel.Load(k_performanceTestModelFilePath), "負荷テスト用SkeletalAnimationModelの読み込みに失敗しました。");

    const auto& l_skeletalAnimationModelRecord = m_performanceTestSkeletalAnimationModel.GetREFSkeletalAnimationModelRecord().lock();

    FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelRecord, "負荷テスト用モデルのSkeletalAnimationModelRecordが無効です。");

    auto& l_modelData = l_skeletalAnimationModelRecord->GetMutableREFModelData();

    // テスト用FBXにモーションが入っていない場合は、確認用のモーションを作って追加する
    // ※ アニメーションのベイクは最初に描画されるときに行うため、ここで追加すればベイクに含まれる
    if (l_modelData.m_motionSequenceList.empty())
    {
        AddPerformanceTestProceduralMotion(l_modelData);
    }

    const auto l_layout = CalculatePerformanceTestLayout(l_modelData.m_modelMeshList);

    FWK_ASSERT_RETURN_IF(l_layout.m_meshletCount == 0ULL || l_layout.m_spacing <= 0.0F, "負荷テスト用モデルに頂点が存在しません。");

          auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance ();
          auto& l_renderer        = l_graphicsManager.GetMutableREFRenderer();
    const auto& l_renderGraph     = l_renderer.GetREFRenderGraph           ();

    const auto& l_litDrawRequest    = l_renderGraph.FindVALDrawRequestPerObject<Graphics::SkeletalAnimationModelGPUSkinnedLitPerObjectDrawRequest>          ().lock();
    const auto& l_shadowDrawRequest = l_renderGraph.FindVALDrawRequestPerObject<Graphics::SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_litDrawRequest,    "SkeletalAnimationModelGPUSkinnedLitPerObjectDrawRequestが見つからないため、負荷テストを開始できません。");
    FWK_ASSERT_RETURN_IF(!l_shadowDrawRequest, "SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequestが見つからないため、負荷テストを開始できません。");

    m_performanceTestSkeletalDrawRequestDataList.reserve(k_performanceTestInstanceCountX * k_performanceTestInstanceCountZ);

    for (std::size_t l_indexZ = 0ULL; l_indexZ < k_performanceTestInstanceCountZ; ++l_indexZ)
    {
        for (std::size_t l_indexX = 0ULL; l_indexX < k_performanceTestInstanceCountX; ++l_indexX)
        {
            auto l_drawRequestData = std::make_shared<Struct::SkeletalAnimationModelGPUSkinnedDrawRequestData>();

            const auto l_instanceIndex = m_performanceTestSkeletalDrawRequestDataList.size();

            l_drawRequestData->m_skeletalAnimationModelRecord = l_skeletalAnimationModelRecord;
            l_drawRequestData->m_worldMatrix                  = TypeAlias::Math::Matrix::CreateTranslation(CalculatePerformanceTestPosition(l_layout, l_indexX, l_indexZ));
            l_drawRequestData->m_worldInverseTransposeMatrix  = l_drawRequestData->m_worldMatrix.Invert().Transpose();

            // モデルごとに再生時刻をずらし、全員が同じ姿勢にならないようにする
            // (std::fmod(割られる数, 割る数) : 浮動小数点の余りを求める)
            l_drawRequestData->m_animationTimeSecond = std::fmod(static_cast<float>(l_instanceIndex) * k_performanceTestAnimationTimeOffsetSecond, k_performanceTestMotionDurationSecond);

            l_litDrawRequest->AddDrawRequest   (l_drawRequestData);
            l_shadowDrawRequest->AddDrawRequest(l_drawRequestData);

            m_performanceTestSkeletalDrawRequestDataList.emplace_back(l_drawRequestData);
        }
    }

    SetupPerformanceTestCamera  (l_layout);
    AddPerformanceTestReportNote(l_layout);

    auto& l_frameProfiler = l_renderer.GetMutableREFFrameProfiler();

    l_frameProfiler.AddReportNote(std::format("SkeletalBoneCount        : {}", l_modelData.m_boneList.size()));
    l_frameProfiler.AddReportNote(std::format("SkeletalMotionCount      : {}", l_modelData.m_motionSequenceList.size()));
}

void FWK::Scene::SetupPerformanceTestCamera(const PerformanceTestLayout& a_layout)
{
    // カメラは並びの手前・斜め上から全体を見下ろす位置に置く
    // 手前のモデルは大きく、奥のモデルは小さく映るため、LODなどの効果を確認しやすい
    const TypeAlias::Math::Vector3 l_cameraPosition =
    {
        0.0F,
        a_layout.m_boundsSize.y * k_performanceTestCameraHeightScale,
       -a_layout.m_gridDepth * k_half - a_layout.m_spacing * k_performanceTestCameraBackOffsetCount
    };

    const TypeAlias::Math::Vector3 l_cameraLookAtPosition =
    {
        0.0F,
        a_layout.m_boundsSize.y * k_performanceTestCameraLookAtHeightScale,
        0.0F
    };

    // XMMatrixLookAtLH(カメラの位置、注視点、カメラの上方向);
    // 戻り値は「ワールド座標 → カメラから見た座標」へ変換するビュー行列なので、
    // 逆行列にしてカメラ自身の姿勢と位置を表す行列にする
    const TypeAlias::Math::Matrix l_viewMatrix   = DirectX::XMMatrixLookAtLH(l_cameraPosition, l_cameraLookAtPosition, TypeAlias::Math::Vector3::Up);
    const TypeAlias::Math::Matrix l_cameraMatrix = l_viewMatrix.Invert();

    // 一番遠いモデル(奥の角)まで映るようにFarClipを決める
    const TypeAlias::Math::Vector3 l_farthestPosition =
    {
        a_layout.m_gridWidth * k_half,
        0.0F,
        a_layout.m_gridDepth * k_half
    };

    const float l_farClip  = TypeAlias::Math::Vector3::Distance(l_cameraPosition, l_farthestPosition) * k_performanceTestCameraFarClipScale;
    const float l_nearClip = a_layout.m_boundsSize.y * k_performanceTestCameraNearClipScale;

          auto& l_application = Application::GetInstance  ();
    const auto& l_window      = l_application.GetREFWindow();

    m_performanceTestCamera.Setup(l_cameraMatrix,
                                  l_window.GetVALAspectRatio(),
                                  Constant::k_cameraDefaultFOVYDegree,
                                  l_farClip,
                                  l_nearClip);

    // シーンファイルのライトの向きが0の場合でも影とライティングが正しく計算されるように、既定のライトを使う
    m_lightSystem.ApplyDefaultSettings();

    auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance ();
    auto& l_renderer        = l_graphicsManager.GetMutableREFRenderer();
    auto& l_frameProfiler   = l_renderer.GetMutableREFFrameProfiler  ();

    l_frameProfiler.AddReportNote(std::format("CameraNearClip / FarClip : {:.3f} / {:.3f}", l_nearClip, l_farClip));
}

void FWK::Scene::AddPerformanceTestReportNote(const PerformanceTestLayout& a_layout) const
{
    // レポートへ負荷テストの条件を書き出す
          auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance ();
          auto& l_renderer        = l_graphicsManager.GetMutableREFRenderer();
          auto& l_frameProfiler   = l_renderer.GetMutableREFFrameProfiler  ();
    const auto  l_instanceCount   = m_performanceTestDrawRequestDataList.size() + m_performanceTestSkeletalDrawRequestDataList.size();

    const auto l_modelKindName = k_performanceTestModelKind == PerformanceTestModelKind::Static ? "Static" : "SkeletalGPUSkinning";

    l_frameProfiler.AddReportNote(std::format("PerformanceTestModel     : {}", k_performanceTestModelFilePath.string()));
    l_frameProfiler.AddReportNote(std::format("PerformanceTestModelKind : {}", l_modelKindName));
    l_frameProfiler.AddReportNote(std::format("ModelMeshCount           : {}", a_layout.m_meshCount));
    l_frameProfiler.AddReportNote(std::format("ModelTriangleCount       : {}", a_layout.m_triangleCount));
    l_frameProfiler.AddReportNote(std::format("ModelMeshletCount        : {}", a_layout.m_meshletCount));
    l_frameProfiler.AddReportNote(std::format("ModelBoundsSize          : ({:.3f}, {:.3f}, {:.3f})",
                                              a_layout.m_boundsSize.x,
                                              a_layout.m_boundsSize.y,
                                              a_layout.m_boundsSize.z));
    l_frameProfiler.AddReportNote(std::format("InstanceCount            : {}", l_instanceCount));
    l_frameProfiler.AddReportNote(std::format("TotalTriangleCount       : {}", a_layout.m_triangleCount * l_instanceCount));

    for (std::size_t l_lodIndex = 0ULL; l_lodIndex < a_layout.m_lodTriangleCountList.size(); ++l_lodIndex)
    {
        l_frameProfiler.AddReportNote(std::format("ModelTriangleCount LOD{}   : {}", l_lodIndex, a_layout.m_lodTriangleCountList[l_lodIndex]));
    }
}

FWK::TypeAlias::Math::Vector3 FWK::Scene::CalculatePerformanceTestPosition(const PerformanceTestLayout& a_layout, const std::size_t a_indexX, const std::size_t a_indexZ) const
{
    // 並び全体の中心が原点に来るように配置し、モデルの足元(最も低い頂点)を地面(Y=0)に合わせる
    return
    {
        static_cast<float>(a_indexX) * a_layout.m_spacing - a_layout.m_gridWidth * k_half,
       -a_layout.m_boundsMIN.y,
        static_cast<float>(a_indexZ) * a_layout.m_spacing - a_layout.m_gridDepth * k_half
    };
}

void FWK::Scene::AddPerformanceTestProceduralMotion(Graphics::SkeletalAnimationModelRecord::ModelData& a_modelData) const
{
    using ModelRecord = Graphics::SkeletalAnimationModelRecord;

    ModelRecord::ModelMotionSequence l_motionSequence = {};

    l_motionSequence.m_motionName     = L"PerformanceTestProceduralMotion";
    l_motionSequence.m_durationSecond = k_performanceTestMotionDurationSecond;
    l_motionSequence.m_frameRate      = ModelRecord::k_defaultAnimationFrameRate;

    // PoseEvaluatorは「再生時刻 × FrameRate」番目のKeyFrameを読むため、KeyFrameはFrameRateの間隔で並べる
    // 最後の時刻(= モーションの長さ)も含めるため+1する
    const auto l_keyFrameCount = static_cast<std::size_t>(std::floor(k_performanceTestMotionDurationSecond * l_motionSequence.m_frameRate)) + 1ULL;

    l_motionSequence.m_boneMotionTrackList.reserve(a_modelData.m_boneList.size());

    for (std::size_t l_boneIndex = 0ULL; l_boneIndex < a_modelData.m_boneList.size(); ++l_boneIndex)
    {
        // バインドポーズ(元の姿勢)の拡大率、回転、位置を取り出す
        // Decompose(拡大率, 回転, 位置) : 行列を3つの要素に分解する(分解できない行列ならfalse)
        TypeAlias::Math::Vector3    l_bindScale       = TypeAlias::Math::Vector3::One;
        TypeAlias::Math::Quaternion l_bindRotation    = TypeAlias::Math::Quaternion::Identity;
        TypeAlias::Math::Vector3    l_bindTranslation = TypeAlias::Math::Vector3::Zero;

        if (!a_modelData.m_boneList[l_boneIndex].m_bindPoseLocalMatrix.Decompose(l_bindScale, l_bindRotation, l_bindTranslation)) { continue; }

        ModelRecord::ModelBoneMotionTrack l_boneMotionTrack = {};

        l_boneMotionTrack.m_boneIndex = static_cast<std::uint32_t>(l_boneIndex);
        l_boneMotionTrack.m_keyFrameList.reserve(l_keyFrameCount);

        for (std::size_t l_keyFrameIndex = 0ULL; l_keyFrameIndex < l_keyFrameCount; ++l_keyFrameIndex)
        {
            const float l_timeSecond = std::min(static_cast<float>(l_keyFrameIndex) / l_motionSequence.m_frameRate, k_performanceTestMotionDurationSecond);

            // モーションの長さで1周する正弦波で、ボーンごとにタイミングをずらして揺らす
            // (最初と最後のKeyFrameが同じ角度になるため、ループ再生しても繋ぎ目が出ない)
            const float l_phase = DirectX::XM_2PI * l_timeSecond / k_performanceTestMotionDurationSecond + static_cast<float>(l_boneIndex) * k_performanceTestMotionBonePhaseRadian;
            const float l_angle = std::sin(l_phase) * k_performanceTestMotionAmplitudeRadian;

            // CreateFromAxisAngle(回転軸, 角度) : 軸周りに回転するQuaternionを作る
            // 「揺れの回転 → 元の回転」の順に掛け、ボーン自身の向きを基準に揺らす
            const auto l_swingRotation = TypeAlias::Math::Quaternion::CreateFromAxisAngle(TypeAlias::Math::Vector3::UnitX, l_angle);

            ModelRecord::ModelKeyFrame l_keyFrame = {};

            l_keyFrame.m_scale       = l_bindScale;
            l_keyFrame.m_rotation    = l_swingRotation * l_bindRotation;
            l_keyFrame.m_translation = l_bindTranslation;
            l_keyFrame.m_timeSecond  = l_timeSecond;

            l_boneMotionTrack.m_keyFrameList.emplace_back(l_keyFrame);
        }

        l_motionSequence.m_boneMotionTrackList.emplace_back(std::move(l_boneMotionTrack));
    }

    a_modelData.m_motionSequenceList.emplace_back(std::move(l_motionSequence));
}

void FWK::Scene::UpdatePerformanceTestCameraAspectRatio()
{
          auto& l_application   = Application::GetInstance    ();
    const auto& l_window        = l_application.GetREFWindow  ();
    const auto& l_resizeRequest = l_window.GetREFResizeRequest();

    if (!l_resizeRequest.m_isRequested ||
         l_resizeRequest.m_isMinimized)
    {
        return;
    }

    m_performanceTestCamera.ApplyPerspectiveAspectRatio(l_window.GetVALAspectRatio());
}

void FWK::Scene::UpdatePerformanceTestAnimationTime()
{
    if (m_performanceTestSkeletalDrawRequestDataList.empty()) { return; }

          auto& l_application   = Application::GetInstance             ();
    const auto& l_fpsController = l_application.GetREFFPSController    ();
    const float l_deltaTime     = l_fpsController.GetVALScaledDeltaTime();

    // 再生時刻を進め、モーションの長さを超えたら先頭へ戻す(ループ再生)
    // 実際に何コマ目を使うかは、描画申請側がベイク済みのコマ数から決める
    for (const auto& l_drawRequestData : m_performanceTestSkeletalDrawRequestDataList)
    {
        l_drawRequestData->m_animationTimeSecond = std::fmod(l_drawRequestData->m_animationTimeSecond + l_deltaTime, k_performanceTestMotionDurationSecond);
    }
}