#include "GameObjectBenchmarkComponent.h"
#include "../../../../Application/Application.h"

// ※ 注意 : 速さを比べるためだけのテスト用のコンポーネント(Documents/ImplementationPlan/00_Overview.md の「テスト用に作ったもの」)
//           比べ終わったら、このフォルダごと消し、vcxproj / filters / Framework.h / タグの定数からも消す
// 指定したモデルを、このGameObjectの位置から格子状にN体並べる
// 並べたGameObjectはシーンに入れず、このコンポーネントが持つ(シーンを保存しても混ざらず、このコンポーネントを消せば全部消える)
// シーンに入れないため、並べたGameObjectのUpdate / PostLateUpdateは、このコンポーネントが代わりに呼ぶ
FWK::GameObjectBenchmarkComponent::GameObjectBenchmarkComponent() :
    m_spawnedGameObjectList(),

    m_modelFilePath                              (),
    m_fetchSelfGameObjectTransformComponentHelper(),

    m_inspector(),

    m_spacing      (k_defaultSpacing),
    m_movingRatio  (k_defaultMovingRatio),
    m_elapsedSecond(k_initialElapsedSecond),

    m_spawnCount(k_defaultSpawnCount),

    m_isSkeletal(false),
    m_isAttached(false)
{
    m_modelFilePath.SetAllowedType(Enum::AssetFilePathType::Model);
}
FWK::GameObjectBenchmarkComponent::~GameObjectBenchmarkComponent() = default;

void FWK::GameObjectBenchmarkComponent::PostDeserialize()
{
    // 並べる位置の基準にするため、自身のTransformComponentをキャッシュする
    m_fetchSelfGameObjectTransformComponentHelper.PostDeserialize(GetREFOwner());
}

void FWK::GameObjectBenchmarkComponent::Update()
{
    const auto& l_application   = Application::GetInstance         ();
    const auto& l_fpsController = l_application.GetREFFPSController();

    m_elapsedSecond += l_fpsController.GetVALScaledDeltaTime();

    // 並べたGameObjectのUpdate(スケルタルならアニメーションの時間が進む)
    for (const auto& l_gameObject : m_spawnedGameObjectList)
    {
        l_gameObject->Update();
    }

    // 指定した割合のGameObjectを上下に揺らす
    // 行列が毎フレーム変わるオブジェクトの数を変えて、比べられるようにするため
    MoveSpawnedGameObjectList();
}
void FWK::GameObjectBenchmarkComponent::PostLateUpdate()
{
    // 並べたGameObjectの行列を最新にし、モデルのコンポーネントへ渡す
    for (const auto& l_gameObject : m_spawnedGameObjectList)
    {
        l_gameObject->PostLateUpdate();
    }
}

void FWK::GameObjectBenchmarkComponent::EditInspector()
{
    m_inspector.EditInspector(*this);
}

std::shared_ptr<FWK::GameObjectComponentBase> FWK::GameObjectBenchmarkComponent::Clone() const
{
    // テスト用なのでJSONには保存しない(設定は複製のときだけ写す)
    // 並べたGameObjectは複製しない
    auto l_clone = std::make_shared<GameObjectBenchmarkComponent>();

    auto& l_cloneModelFilePath = l_clone->GetMutableREFModelFilePath();

    l_cloneModelFilePath.SetAssetFilePathUUID(m_modelFilePath.GetREFAssetFilePathUUID());

    l_clone->SetSpacing    (m_spacing);
    l_clone->SetMovingRatio(m_movingRatio);
    l_clone->SetSpawnCount (m_spawnCount);
    l_clone->SetIsSkeletal (m_isSkeletal);

    return l_clone;
}

void FWK::GameObjectBenchmarkComponent::Attach()
{
    m_isAttached = true;

    // ※ 注意 : 本来ApplyIsInSceneはSceneだけが呼ぶ
    //           並べたGameObjectはシーンに入れないため、テスト用としてここで代わりに呼び、描画の登録をさせる
    for (const auto& l_gameObject : m_spawnedGameObjectList)
    {
        l_gameObject->ApplyIsInScene(true);
    }
}
void FWK::GameObjectBenchmarkComponent::Detach()
{
    m_isAttached = false;

    for (const auto& l_gameObject : m_spawnedGameObjectList)
    {
        l_gameObject->ApplyIsInScene(false);
    }
}

void FWK::GameObjectBenchmarkComponent::Spawn()
{
    // 前に並べたものを消してから並べ直す
    Clear();

    if (m_spawnCount < k_minSpawnCount) { return; }

    // 格子の1辺の数 = ceil(√N)
    // 例 : 100体なら10 × 10、128体なら12 × 12の中に128体
    const auto& l_spawnCount    = static_cast<std::size_t>(m_spawnCount);
    const auto& l_gridSideCount = static_cast<std::size_t>(std::ceil(std::sqrt(static_cast<float>(m_spawnCount))));

    m_spawnedGameObjectList.reserve(l_spawnCount);

    for (std::size_t l_spawnIndex = 0ULL; l_spawnIndex < l_spawnCount; ++l_spawnIndex)
    {
        const auto& l_gameObject = CreateGameObject(FetchVALSpawnPosition(l_spawnIndex, l_gridSideCount));

        if (!l_gameObject) { return; }

        m_spawnedGameObjectList.emplace_back(l_gameObject);
    }
}
void FWK::GameObjectBenchmarkComponent::Clear()
{
    // 描画の登録を外してから捨てる
    for (const auto& l_gameObject : m_spawnedGameObjectList)
    {
        l_gameObject->ApplyIsInScene(false);
    }

    m_spawnedGameObjectList.clear();

    m_elapsedSecond = k_initialElapsedSecond;
}

std::shared_ptr<FWK::GameObject> FWK::GameObjectBenchmarkComponent::CreateGameObject(const TypeAlias::Math::Vector3& a_position) const
{
    // GameObjectのINITはweak_from_thisを使うため、必ずmake_sharedで作る
    auto l_gameObject = std::make_shared<GameObject>();

    l_gameObject->INIT();

    // モデルのコンポーネントに、このコンポーネントと同じモデルを指定する
    auto  l_modelComponent     = std::make_shared<GameObjectModelComponent>     ();
    auto& l_modelFilePath      = l_modelComponent->GetMutableREFModelFilePath   ();
    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer  ();

    l_modelFilePath.SetAssetFilePathUUID(m_modelFilePath.GetREFAssetFilePathUUID());
    l_modelComponent->SetIsSkeletal     (m_isSkeletal);

    // シーンにいないGameObjectなので、ここではAttachは呼ばれない
    if (!l_componentContainer.AddComponent(l_modelComponent)) { return nullptr; }

    const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

    if (!l_transformComponent) { return nullptr; }

    l_transformComponent->ApplyTransformPosition(a_position);

    // モデルのコンポーネントのPostDeserializeで、Transformのキャッシュとモデルの読み込みが行われる
    // その後のPostLateUpdateで行列が計算され、モデルへ渡される
    l_gameObject->PostDeserialize();
    l_gameObject->PostLateUpdate ();

    // このコンポーネントがシーンにいるなら、すぐに描画の登録をさせる(Attachの説明を参照)
    if (m_isAttached)
    {
        l_gameObject->ApplyIsInScene(true);
    }

    return l_gameObject;
}

void FWK::GameObjectBenchmarkComponent::MoveSpawnedGameObjectList() const
{
    // 先頭から「全体の数 × 割合」体だけを動かす
    // 例 : 100体で割合0.1なら、先頭の10体が上下に揺れる(残りの90体は行列が変わらない)
    // 割合はインスペクターで0 ~ 1に制限しているが、念のため全体の数を超えないようにする
    const auto& l_spawnedCount  = m_spawnedGameObjectList.size();
    const auto  l_movingCount   = std::min(static_cast<std::size_t>(static_cast<float>(l_spawnedCount) * m_movingRatio), l_spawnedCount);
    const auto& l_gridSideCount = static_cast<std::size_t>(std::ceil(std::sqrt(static_cast<float>(l_spawnedCount))));

    for (std::size_t l_spawnIndex = 0ULL; l_spawnIndex < l_movingCount; ++l_spawnIndex)
    {
        const auto& l_transformComponent = m_spawnedGameObjectList[l_spawnIndex]->GetVALTransformComponent().lock();

        if (!l_transformComponent) { continue; }

        // 少しずつずらした波で揺らす(全員が同じ動きにならないようにする)
        auto l_position = FetchVALSpawnPosition(l_spawnIndex, l_gridSideCount);

        l_position.y += std::sin(m_elapsedSecond * k_movingSpeed + static_cast<float>(l_spawnIndex) * k_movingPhaseStep) * k_movingAmplitude;

        l_transformComponent->ApplyTransformPosition(l_position);
    }
}

FWK::TypeAlias::Math::Vector3 FWK::GameObjectBenchmarkComponent::FetchVALSpawnPosition(const std::size_t& a_spawnIndex, const std::size_t& a_gridSideCount) const
{
    // このGameObjectの位置を基準に、XZ平面へ格子状に並べる
    // 例 : 1辺10、間隔2なら、0番は(0, 0, 0)、11番は(2, 0, 2)
    TypeAlias::Math::Vector3 l_originPosition = {};

    if (const auto& l_transformComponent = m_fetchSelfGameObjectTransformComponentHelper.GetREFTransformComponent().lock();
        l_transformComponent)
    {
        const auto& l_transform = l_transformComponent->GetREFTransform();

        l_originPosition = l_transform.m_position;
    }

    const auto l_gridX = static_cast<float>(a_spawnIndex % a_gridSideCount);
    const auto l_gridZ = static_cast<float>(a_spawnIndex / a_gridSideCount);

    return l_originPosition + TypeAlias::Math::Vector3{ l_gridX * m_spacing, k_spawnPositionY, l_gridZ * m_spacing };
}