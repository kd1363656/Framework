#include "Scene.h"

#include "../../Application/Application.h"

void FWK::Scene::INIT()
{
	m_camera                    = std::make_shared<Graphics::Camera>								      ();
	m_characterModel            = std::make_shared<Graphics::SkeletalAnimationModel>                      ();
	m_characterAnimationPlayer  = std::make_shared<Graphics::SkeletalAnimationPlayer>                     ();
	m_characterModelDrawRequest = std::make_shared<Struct::SkeletalAnimationModelPerObjectDrawRequestData>();
	m_groundModel               = std::make_shared<Graphics::StaticModel>							      ();
	m_groundModelDrawRequest    = std::make_shared<Struct::StaticModelPerObjectDrawRequestData>           ();
	
	      auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	const auto& l_renderer        = l_graphicsManager.GetREFRenderer      ();
	const auto& l_renderGraph     = l_renderer.GetREFRenderGraph		  ();

	// モデル
	m_characterModel->Load("Asset/Model/Actor/Antike/Antike.fbx");
	m_groundModel->Load   ("Asset/Model/Terrain/Terrain.fbx");
	
	m_characterAnimationPlayer->Create(*m_characterModel);
	
	m_characterAnimationPlayer->PlayMotion(0U, true, Graphics::SkeletalAnimationPlayer::Animation::k_defaultPlaybackSpeed);

	const auto& l_skeletalAnimationPerObjectComputeRequest = l_renderGraph.FindVALComputeRequestPerObject<Graphics::SkeletalAnimationPerObjectComputeRequest>().lock();

	FWK_ASSERT_RETURN_IF(!l_skeletalAnimationPerObjectComputeRequest, "SkeletalAnimationPerObjectComputeRequestを取得できないため、CharacterAnimationPlayerのCompute申請に失敗しました。");

	l_skeletalAnimationPerObjectComputeRequest->AddComputeRequest(m_characterAnimationPlayer);

	m_groundModelDrawRequest->m_staticModelRecord           = m_groundModel->GetREFStaticModelRecord();
	m_groundModelDrawRequest->m_worldMaxScale               = Utility::CalculateWorldMaxScale(m_groundModelDrawRequest->m_worldMatrix);
	m_groundModelDrawRequest->m_worldInverseTransposeMatrix = m_groundModelDrawRequest->m_worldMatrix.Invert().Transpose();

	const auto& l_skeletalAnimationModelStandardLitPerObjectDrawRequest = l_renderGraph.FindVALDrawRequestPerObject<Graphics::SkeletalAnimationModelStandardLitPerObjectDrawRequest>().lock();
	const auto& l_skeletalAnimationModelShadowPerObjectDrawRequest      = l_renderGraph.FindVALDrawRequestPerObject<Graphics::SkeletalAnimationModelShadowPerObjectDrawRequest>     ().lock();

	FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelStandardLitPerObjectDrawRequest, "SkeletalAnimationModelStandardLitPerObjectDrawRequestを取得できないため、Character Modelを描画できません。");
	FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelShadowPerObjectDrawRequest,      "SkeletalAnimationModelShadowPerObjectDrawRequestを取得できないため、Character ModelのShadow描画申請を追加できません。");

	m_characterModelDrawRequest->m_skeletalAnimationPlayer = m_characterAnimationPlayer;

	l_skeletalAnimationModelStandardLitPerObjectDrawRequest->AddDrawRequest(m_characterModelDrawRequest);
	l_skeletalAnimationModelShadowPerObjectDrawRequest->AddDrawRequest     (m_characterModelDrawRequest);

	const auto& l_staticModelStandardLitPerObjectDrawRequest = l_renderGraph.FindVALDrawRequestPerObject<Graphics::StaticModelStandardLitPerObjectDrawRequest>().lock();
	const auto& l_staticModelShadowPerObjectDrawRequest      = l_renderGraph.FindVALDrawRequestPerObject<Graphics::StaticModelShadowPerObjectDrawRequest>     ().lock();
	
	FWK_ASSERT_RETURN_IF(!l_staticModelStandardLitPerObjectDrawRequest, "StaticModelStandardLitPerObjectDrawRequestを取得できないため、Ground Modelを描画できません。");
	FWK_ASSERT_RETURN_IF(!l_staticModelShadowPerObjectDrawRequest,      "StaticModelShadowPerObjectDrawRequestを取得できないため、Ground ModelのShadow描画申請を追加できません。");

	l_staticModelStandardLitPerObjectDrawRequest->AddDrawRequest(m_groundModelDrawRequest);
	l_staticModelShadowPerObjectDrawRequest->AddDrawRequest     (m_groundModelDrawRequest);

	const auto& l_viewport = l_renderer.GetREFScreenRenderArea().GetREFViewport();

	const auto l_aspectRatio = l_viewport.Width / l_viewport.Height;

	// カメラ
	m_camera->Setup(TypeAlias::Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(45.0F)) * TypeAlias::Math::Matrix::CreateTranslation(0.0F, 1.0F, -1.15F), l_aspectRatio);

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
	
	const auto  l_characterWorldPosition  = m_characterVirtual->FetchVALWorldPosition();
	const auto& l_groundStaticModelRecord = m_groundModel->GetREFStaticModelRecord().lock();

	FWK_ASSERT_RETURN_IF(!m_characterModelDrawRequest, "Character Model用DrawRequestDataが無効なため、初期World Matrixを設定できません。");

	const auto l_characterWorldMatrix = TypeAlias::Math::Matrix::CreateRotationY(m_characterModelRotationYRadians) * TypeAlias::Math::Matrix::CreateTranslation(l_characterWorldPosition);

	m_characterModelDrawRequest->m_worldMatrix                 = l_characterWorldMatrix;
	m_characterModelDrawRequest->m_worldInverseTransposeMatrix = l_characterWorldMatrix.Invert().Transpose();

	FWK_ASSERT_RETURN_IF(!l_groundStaticModelRecord, "Ground用StaticModelRecordが無効なため、StaticMeshBodyの作成に失敗しました。");

	auto l_meshBody = std::make_unique<Physics::PhysicsStaticMeshBody>();

	if (!l_meshBody->CreateBody(l_groundStaticModelRecord->GetREFModelData(), true, m_groundModelDrawRequest->m_worldMatrix)) { return; }

	m_staticMeshBody = std::move(l_meshBody);
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
	std::erase_if(m_gameObjectList, [this](const auto& a_gameObject) 
	{
		if (!a_gameObject) { return true; }

		// 削除予定ならGUIDRegistryからも削除
		if (a_gameObject->GetVALIsDestroyed())
		{
			m_gameObjectUUIDRegistry.Erase(a_gameObject->GetMutableREFUUID());

			return true;
		}
		
		return false;
	});

	for (const auto& l_gameObject : m_gameObjectList)
	{
		if (!l_gameObject) { continue; }

		l_gameObject->EarlyUpdate();
	}
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

	m_camera->ApplyCameraMatrix(TypeAlias::Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(45.0F)) * TypeAlias::Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(l_rot)) * TypeAlias::Math::Matrix::CreateTranslation(l_cameraPos));

	// テスト
	const auto& l_application = Application::GetInstance();
	const auto  l_deltaTime = l_application.GetREFFFPSController().GetVALScaledDeltaTime();

	m_characterAnimationPlayer->AdvanceTime(l_deltaTime);

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

	// キーを押した瞬間だけBlendを開始するため、
	// 前Frameの入力状態を保持する。
	static bool l_wasFirstMotionKeyDown  = false;
	static bool l_wasSecondMotionKeyDown = false;
	
	const bool l_isFirstMotionKeyDown  = GetAsyncKeyState('1') != 0;
	const bool l_isSecondMotionKeyDown = GetAsyncKeyState('2') != 0;
	
	// 「1」を押すとMotion ZeroへBlendする。
	// 現在のAntikeではIDLE_COMBATに該当する。
	if (l_isFirstMotionKeyDown   &&
		!l_wasFirstMotionKeyDown && !m_characterAnimationPlayer->GetVALIsBlending())
	{
		const auto& l_currentAnimation = m_characterAnimationPlayer->GetREFAnimation();
	
		// すでにMotion Zeroを再生中なら、
		// 同じMotionへの不要なBlendは開始しない。
		if (l_currentAnimation.m_motionIndex != 0U)
		{
			Graphics::SkeletalAnimationPlayer::Animation l_animation = {};
	
			l_animation.m_motionIndex         = 0U;
			l_animation.m_playbackSpeed       = Graphics::SkeletalAnimationPlayer::Animation::k_defaultPlaybackSpeed;
			l_animation.m_blendDurationSecond = 0.5F;
			l_animation.m_isLoop              = true;
	
			FWK_ASSERT_RETURN_IF(!m_characterAnimationPlayer->ApplyAnimation(l_animation), "Motion ZeroへのAnimation Blendを開始できませんでした。");
		}
	}
	
	// 「2」を押すとMotion OneへBlendする。
	//
	// 現在のAntikeではIDLE_TITLEに該当する。
	if (l_isSecondMotionKeyDown   &&
		!l_wasSecondMotionKeyDown && !m_characterAnimationPlayer->GetVALIsBlending())
	{
		const auto& l_currentAnimation = m_characterAnimationPlayer->GetREFAnimation();
	
		// すでにMotion Oneを再生中なら、
		// 同じMotionへの不要なBlendは開始しない。
		if (l_currentAnimation.m_motionIndex != 1U)
		{
			Graphics::SkeletalAnimationPlayer::Animation l_animation = {};
	
			l_animation.m_motionIndex = 1U;
	
			l_animation.m_playbackSpeed = Graphics::SkeletalAnimationPlayer::Animation::k_defaultPlaybackSpeed;
	
			l_animation.m_blendDurationSecond = 0.5F;
	
			l_animation.m_isLoop = true;
	
			FWK_ASSERT_RETURN_IF
			(
				!m_characterAnimationPlayer->ApplyAnimation(l_animation), "Motion OneへのAnimation Blendを開始できませんでした。"
			);
		}
	}

	l_wasFirstMotionKeyDown = l_isFirstMotionKeyDown;
	l_wasSecondMotionKeyDown = l_isSecondMotionKeyDown;

	// Blend元とBlend先の再生時間、Blend経過時間を進め、
	// 現在Frame用のBone Matrixを計算する。
	m_characterAnimationPlayer->AdvanceTime(l_deltaTime);

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
	
	const auto  l_characterWorldPosition = m_characterVirtual->FetchVALWorldPosition();
	const auto& l_characterWorldMatrix   = TypeAlias::Math::Matrix::CreateRotationY(m_characterModelRotationYRadians) * TypeAlias::Math::Matrix::CreateTranslation(l_characterWorldPosition);

	m_characterModelDrawRequest->m_worldMatrix                 = l_characterWorldMatrix;
	m_characterModelDrawRequest->m_worldInverseTransposeMatrix = l_characterWorldMatrix.Invert().Transpose();

	m_characterVirtual->DrawDebug(JPH::ColorArg{ 255U, 255U, 255U, 255U });

	for (const auto& l_gameObject : m_gameObjectList)
	{
		if (!l_gameObject) { continue; }

		l_gameObject->Update();
	}
}
void FWK::Scene::LateUpdate() const
{
	for (const auto& l_gameObject : m_gameObjectList)
	{
		if (!l_gameObject) { continue; }

		l_gameObject->LateUpdate();
	}
}
void FWK::Scene::ConfirmMatrix() const
{
	for (const auto& l_gameObject : m_gameObjectList)
	{
		if (!l_gameObject) { continue; }

		l_gameObject->ConfirmMatrix();
	}
}

nlohmann::json FWK::Scene::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Scene::AddGameObject(const std::shared_ptr<GameObject>& a_gameObject)
{
	if (!a_gameObject) 
	{
		FWK_ADD_LOG("GameObjectクラスが無効となっており、ゲームオブジェクトの追加処理に失敗しました。");
		return; 
	}

	// ゲームオブジェクトとそのUUIDを登録、ただしUUIDがGUID_NULLだったり、
	// 重複するUUIDの場合UUIDを生成してゲームオブジェクト側のUUIDにも反映する
	FWK_ASSERT_RETURN_IF(m_gameObjectUUIDRegistry.Add(a_gameObject, a_gameObject->GetMutableREFUUID()), "ゲームオブジェクトのUUIDの登録に失敗しており、ゲームオブジェクトの追加処理に失敗しました。");
	 
	m_gameObjectList.emplace_back(a_gameObject, a_gameObject.get());
}