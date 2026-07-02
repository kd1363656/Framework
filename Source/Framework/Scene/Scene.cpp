#include "Scene.h"

void FWK::Scene::INIT()
{
	m_camera                         = std::make_shared<Graphics::Camera>								    ();
	m_charaModel                     = std::make_shared<Graphics::StaticModel>							    ();
	m_groundModel                    = std::make_shared<Graphics::StaticModel>							    ();
	m_charaModelStandardDrawRequest  = std::make_shared<Struct::StaticModelStandardPerObjectDrawRequestData>();
	m_groundModelStandardDrawRequest = std::make_shared<Struct::StaticModelStandardPerObjectDrawRequestData>();
	
	const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	const auto& l_renderer        = l_graphicsManager.GetREFRenderer      ();
	const auto& l_renderGraph     = l_renderer.GetREFRenderGraph		  ();

	// モデル
	m_charaModel->Load ("Asset/Model/Antike.fbx");
	m_groundModel->Load("Asset/Model/Terrain/Terrain.fbx");

	// 本来はUpdateなどで更新する
	m_charaModelStandardDrawRequest->m_staticModelRecord           = m_charaModel->GetREFStaticModelRecord();
	m_charaModelStandardDrawRequest->m_worldMatrix				   = TypeAlias::Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(0.0F));
	m_charaModelStandardDrawRequest->m_worldMaxScale               = 1.0F;
	m_charaModelStandardDrawRequest->m_worldInverseTransposeMatrix = m_charaModelStandardDrawRequest->m_worldMatrix.Transpose();

	m_groundModelStandardDrawRequest->m_staticModelRecord           = m_groundModel->GetREFStaticModelRecord();
	m_groundModelStandardDrawRequest->m_worldMaxScale               = 1.0F;
	m_groundModelStandardDrawRequest->m_worldInverseTransposeMatrix = m_groundModelStandardDrawRequest->m_worldMatrix.Transpose();

	const auto& l_staticModelStandardPerObjectDrawRequest = l_renderGraph.FindVALDrawRequestPerObject<Graphics::StaticModelStandardLitPerObjectDrawRequest>().lock();
	
	if (!l_staticModelStandardPerObjectDrawRequest) { return; }

	l_staticModelStandardPerObjectDrawRequest->AddDrawRequest(m_charaModelStandardDrawRequest);
	l_staticModelStandardPerObjectDrawRequest->AddDrawRequest(m_groundModelStandardDrawRequest);

	const auto& l_viewport = l_renderer.GetREFRenderArea().GetREFViewport();

	const auto l_aspectRatio = l_viewport.Width / l_viewport.Height;

	// カメラ
	m_camera->Setup(TypeAlias::Math::Matrix::CreateTranslation(0.0F, 1.0F, -1.15F), l_aspectRatio);

	// ライト
	m_lightSystem.ApplyDefaultSettings();

	//=========================================================
	// JoltPhysics 当たり判定テスト
	//=========================================================
	// まだTransformComponentは使わない。
	// 今回はScene側で直接座標を渡し、
	// StaticBoxの床・壁とDynamicCapsuleの衝突を確認する。
	auto& l_physicsManager = Physics::PhysicsManager::GetInstance();
	
	// 床Collider。
	// 見た目のTerrainとはまだ一致させない。
	// まずは「平らな床に乗るか」だけ確認する。
	const TypeAlias::Math::Vector3& l_floorWorldPosition = { 0.0F, -0.5F, 0.0F };
	const TypeAlias::Math::Vector3& l_floorHalfExtent    = { 5.0F,  0.5F, 5.0F };
	
	m_staticFloorBodyHandle = l_physicsManager.CreateStaticBoxBody(l_floorWorldPosition,
																   l_floorHalfExtent);
	
	// 壁Collider。
	// X方向に移動したときに、Capsuleが壁へ入り込まず止まるか確認するための壁。
	const TypeAlias::Math::Vector3& l_wallWorldPosition = { 2.0F, 1.0F, 0.0F };
	const TypeAlias::Math::Vector3& l_wallHalfExtent    = { 0.25F, 1.0F, 5.0F };
	
	m_staticWallBodyHandle = l_physicsManager.CreateStaticBoxBody(l_wallWorldPosition,
																  l_wallHalfExtent);
	
	// プレイヤー仮Body。
	// 人型キャラクターの当たり判定として、まずはDynamicCapsuleを使う。
	// Boxだと角が引っかかりやすく、Sphereだと人型として低すぎるため。
	const TypeAlias::Math::Vector3& l_capsuleWorldPosition = { 0.0F, 3.0F, 0.0F };
	
	const float l_capsuleHalfHeightOfCylinder = 0.75F;
	const float l_capsuleRadius               = 0.35F;
	
	m_dynamicCapsuleBodyHandle = l_physicsManager.CreateDynamicCapsuleBody(l_capsuleWorldPosition,
																		   l_capsuleHalfHeightOfCylinder,
																		   l_capsuleRadius);
}
void FWK::Scene::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) 
	{
		FWK_ADD_LOG("jsonの中身が空となっておりシーンのデシリアライズ処理に失敗しました。");
		return; 
	}

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Scene::PostDeserialize() const
{
	auto& l_physicsManager = Physics::PhysicsManager::GetInstance();

	// StaticBodyをまとめて追加した後なので、BroadPhaseを最適化しておく。
	// 毎フレーム呼ぶものではなく、ステージ読み込み後などに呼ぶ。
	l_physicsManager.OptimizeBroadPhase();
}

void FWK::Scene::EarlyUpdate()
{

}
void FWK::Scene::Update()
{
	// テスト実装カメラ(絶対に後で消す)
	static TypeAlias::Math::Vector3 l_cameraPos = { 0.0F, 1.0F, -1.15F };
	static float					l_rot		= 0.0F;

	if (GetAsyncKeyState(VK_SHIFT))
	{
		if (GetAsyncKeyState('W'))
		{
			l_cameraPos.y += 0.1F;
		}
		else if (GetAsyncKeyState('S'))
		{
			l_cameraPos.y -= 0.1F;
		}
	}
	else if (GetAsyncKeyState('W'))
	{
		l_cameraPos.z += 0.1F;
	}
	else if (GetAsyncKeyState('S'))
	{
		l_cameraPos.z -= 0.1F;
	}

	if (GetAsyncKeyState('A'))
	{
		l_cameraPos.x -= 0.1F;
	}
	else if (GetAsyncKeyState('D'))
	{
		l_cameraPos.x += 0.1F;
	}

	if (GetAsyncKeyState('Q'))
	{
		l_rot -= 1.0F;
	}
	else if (GetAsyncKeyState('E'))
	{
		l_rot += 1.0F;
	}

	if (GetAsyncKeyState('1'))
	{
		m_charaModel				    = nullptr;
		m_charaModelStandardDrawRequest = nullptr;
	}

	m_camera->ApplyCameraMatrix(TypeAlias::Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(l_rot)) * TypeAlias::Math::Matrix::CreateTranslation(l_cameraPos));

	//=========================================================
	// DynamicCapsule操作テスト
	//=========================================================
	// 描画モデルを直接動かすのではなく、JoltのBodyに速度を設定する。
	// 実際にどこまで移動できるか、壁に当たったときに止まるかは、
	// PhysicsManager::Update() 内でJoltが計算する。
	if (m_dynamicCapsuleBodyHandle.m_isValid)
	{
		auto& l_physicsManager = Physics::PhysicsManager::GetInstance();
	
		static constexpr float k_playerMoveSpeed = 3.0F;
		static constexpr float k_moveZero        = 0.0F;
	
		TypeAlias::Math::Vector3 l_nextVelocity = { k_moveZero, k_moveZero, k_moveZero };
	
		if (GetAsyncKeyState(VK_UP))
		{
			l_nextVelocity.z += k_playerMoveSpeed;
		}
		else if (GetAsyncKeyState(VK_DOWN))
		{
			l_nextVelocity.z -= k_playerMoveSpeed;
		}
	
		if (GetAsyncKeyState(VK_LEFT))
		{
			l_nextVelocity.x -= k_playerMoveSpeed;
		}
		else if (GetAsyncKeyState(VK_RIGHT))
		{
			l_nextVelocity.x += k_playerMoveSpeed;
		}
	
		// 現在のY速度を取得する。
		// 横移動を設定するときにY速度まで0にすると、重力落下が消えてしまう。
		// そのため、X/Zは入力で上書きし、Yだけ現在の物理速度を残す。
		const auto l_currentVelocity = l_physicsManager.FetchVALBodyLinearVelocity(m_dynamicCapsuleBodyHandle);
	
		l_nextVelocity.y = l_currentVelocity.y;
	
		// Bodyへ速度を設定する。
		// ここで位置を直接動かすのではなく、速度を渡すのが重要。
		l_physicsManager.ApplyBodyLinearVelocity(m_dynamicCapsuleBodyHandle,
												 l_nextVelocity);
	
		// Gキーを押している間だけ重力OFF。
		// 離したら重力ON。
		const bool l_isGravityEnabled = !GetAsyncKeyState('G');
	
		l_physicsManager.ApplyBodyGravityEnabled(m_dynamicCapsuleBodyHandle,
												 l_isGravityEnabled);
	
		// 物理Bodyの現在位置を描画モデルへ反映する。
		// この時点ではTransformComponentは使わず、DrawRequestのworldMatrixへ直接入れる。
		if (m_charaModelStandardDrawRequest)
		{
			const auto l_bodyWorldPosition = l_physicsManager.FetchVALBodyWorldPosition(m_dynamicCapsuleBodyHandle);
	
			m_charaModelStandardDrawRequest->m_worldMatrix =
				TypeAlias::Math::Matrix::CreateTranslation(l_bodyWorldPosition);
	
			m_charaModelStandardDrawRequest->m_worldInverseTransposeMatrix =
				m_charaModelStandardDrawRequest->m_worldMatrix.Transpose();
		}
	}
}
void FWK::Scene::LateUpdate() const
{

}
void FWK::Scene::FixMatrix()
{

}

nlohmann::json FWK::Scene::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}