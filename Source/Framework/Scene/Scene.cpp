#include "Scene.h"

#include "../../Application/Application.h"

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
	m_charaModelStandardDrawRequest->m_worldMaxScale               = Utility::CalculateWorldMaxScale(m_charaModelStandardDrawRequest->m_worldMatrix);
	m_charaModelStandardDrawRequest->m_worldInverseTransposeMatrix = m_charaModelStandardDrawRequest->m_worldMatrix.Invert().Transpose();

	m_groundModelStandardDrawRequest->m_staticModelRecord           = m_groundModel->GetREFStaticModelRecord();
	m_groundModelStandardDrawRequest->m_worldMaxScale               = Utility::CalculateWorldMaxScale(m_groundModelStandardDrawRequest->m_worldMatrix);
	m_groundModelStandardDrawRequest->m_worldInverseTransposeMatrix = m_groundModelStandardDrawRequest->m_worldMatrix.Invert().Transpose();

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

	// Physics
	auto l_boxBody     = std::make_unique<Physics::PhysicsStaticBoxBody>    ();
	auto l_sphererBody = std::make_unique<Physics::PhysicsStaticSphereBody> ();
	auto l_capsuleBody = std::make_unique<Physics::PhysicsStaticCapsuleBody>();

	l_boxBody->m_halfExtent = { 5.0F,  0.5F, 5.0F };

	l_boxBody->CreateBody(TypeAlias::Math::Quaternion::Identity, { 0.0F, -0.5F, 0.0F }, true);

	l_sphererBody->CreateBody(TypeAlias::Math::Quaternion::Identity, { -2.0F, 0.75F, 0.0F }, true);

	const float l_capsuleHalfHeightOfCylinder = 0.75F;
	const float l_capsuleRadius               = 0.5F;
	const float l_capsuleHalfHeight           = l_capsuleHalfHeightOfCylinder + l_capsuleRadius;

	l_capsuleBody->m_halfHeightOfCylinder = l_capsuleHalfHeightOfCylinder;
	l_capsuleBody->m_radius			      = l_capsuleRadius;

	l_capsuleBody->CreateBody(TypeAlias::Math::Quaternion::Identity, { 2.0F, l_capsuleHalfHeight, 0.0F }, false);

	m_staticBoxBody     = std::move(l_boxBody);
	m_staticSphereBody  = std::move(l_sphererBody);
	m_staticCapsuleBody = std::move(l_capsuleBody);

	auto l_characterVirtual = std::make_unique<Physics::PhysicsCharacterVirtualAffectedByGravity>();

	if (!l_characterVirtual->CreateCharacterVirtual(TypeAlias::Math::Quaternion::Identity, { 0.0F, 2.0F, 0.0F })) { return; }

	m_characterVirtual = std::move(l_characterVirtual);

	m_characterModelRotationYRadians = 0.0F;
	m_wasJumpKeyDown = false;

	if (!m_characterVirtual) { return; }
	if (!m_charaModelStandardDrawRequest) { return; }

	const auto l_characterWorldPosition = m_characterVirtual->FetchVALWorldPosition();

	m_charaModelStandardDrawRequest->m_worldMatrix                 = TypeAlias::Math::Matrix::CreateRotationY(m_characterModelRotationYRadians) * TypeAlias::Math::Matrix::CreateTranslation(l_characterWorldPosition);
	m_charaModelStandardDrawRequest->m_worldInverseTransposeMatrix = m_charaModelStandardDrawRequest->m_worldMatrix.Transpose();
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
	std::erase_if(m_gameObjectList, [this](const auto& a_gameObjectData) 
	{
		if (!a_gameObjectData.m_gameObject) { return false; }

		m_registeredGameObjectSet.erase(a_gameObjectData.m_gameObjectAddress);

		return true;
	});
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

	// テスト
	const auto& l_application = Application::GetInstance();
	const auto  l_deltaTime = l_application.GetREFFFPSController().GetVALScaledDeltaTime();

	if (!m_characterVirtual) { return; }

	TypeAlias::Math::Vector3 l_moveDirection = {};

	if (GetAsyncKeyState(VK_UP))
	{
		l_moveDirection.z += 1.0F;
	}

	if (GetAsyncKeyState(VK_DOWN))
	{
		l_moveDirection.z -= 1.0F;
	}

	if (GetAsyncKeyState(VK_LEFT))
	{
		l_moveDirection.x -= 1.0F;
	}

	if (GetAsyncKeyState(VK_RIGHT))
	{
		l_moveDirection.x += 1.0F;
	}

	if (l_moveDirection.LengthSquared() > 0.0F)
	{
		// 斜め移動時に速度が速くならないよう、方向ベクトルの長さを1にする。
		l_moveDirection.Normalize();

		// Antikeの見た目だけを移動方向へ向ける。
		// CapsuleはY軸周りに回転しても形が変わらないため、
		// CharacterVirtual側の回転は変更しない。
		m_characterModelRotationYRadians = std::atan2(l_moveDirection.x, l_moveDirection.z) + 0.0F;
	}

	const bool l_isJumpKeyDown = GetAsyncKeyState(VK_SPACE) != 0;

	Struct::PhysicsCharacterVirtualUpdateData l_updateData = {};

	l_updateData.m_desiredVelocity = l_moveDirection * 3.0F;

	// Spaceを押した瞬間だけジャンプ申請を送る。
	// 押し続けている間、毎フレーム申請しないようにする。
	l_updateData.m_isJumpRequested = l_isJumpKeyDown &&
									!m_wasJumpKeyDown;

	m_wasJumpKeyDown = l_isJumpKeyDown;

	m_characterVirtual->Update(l_updateData, l_deltaTime);

	// 衝突判定後のCharacterVirtual座標をAntikeへ反映する。
	if (!m_characterVirtual) { return; }
	if (!m_charaModelStandardDrawRequest) { return; }

	const auto l_characterWorldPosition = m_characterVirtual->FetchVALWorldPosition();

	m_charaModelStandardDrawRequest->m_worldMatrix                 = TypeAlias::Math::Matrix::CreateRotationY(m_characterModelRotationYRadians) * TypeAlias::Math::Matrix::CreateTranslation(l_characterWorldPosition);
	m_charaModelStandardDrawRequest->m_worldInverseTransposeMatrix = m_charaModelStandardDrawRequest->m_worldMatrix.Invert().Transpose();

	m_characterVirtual->DrawDebug(JPH::ColorArg{ 255U, 255U, 255U, 255U });
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

void FWK::Scene::AddGameObject(const std::shared_ptr<GameObject>& a_gameObject)
{
	if (!a_gameObject) 
	{
		FWK_ADD_LOG("GameObjectクラスが無効となっており、追加処理に失敗しました。");
		return; 
	}

	// 既に登録されているアドレスを持つゲームオブジェクトなら追加しない
	if (m_registeredGameObjectSet.contains(a_gameObject.get())) { return; }

	m_gameObjectList.emplace_back    (a_gameObject, a_gameObject.get());
	m_registeredGameObjectSet.emplace(a_gameObject.get());
}