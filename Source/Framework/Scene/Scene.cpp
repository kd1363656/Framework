#include "Scene.h"

void FWK::Scene::INIT()
{
	m_camera                                = std::make_shared<Graphics::Camera>								   ();
	m_staticModel                           = std::make_shared<Graphics::StaticModel>							   ();
	m_staticGraoundModel                    = std::make_shared<Graphics::StaticModel>							   ();
	m_staticModelStandardDrawRequest        = std::make_shared<Struct::StaticModelStandardPerObjectDrawRequestData>();
	m_staticGraoundModelStandardDrawRequest = std::make_shared<Struct::StaticModelStandardPerObjectDrawRequestData>();

	const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	const auto& l_renderer        = l_graphicsManager.GetREFRenderer      ();
	const auto& l_renderGraph     = l_renderer.GetREFRenderGraph		  ();

	// モデル
	m_staticModel->Load       ("Asset/Model/Antike.fbx");
	m_staticGraoundModel->Load("Asset/Model/Terrain/Terrain.fbx");

	// 本来はUpdateなどで更新する
	m_staticModelStandardDrawRequest->m_staticModelRecord           = m_staticModel->GetREFStaticModelRecord();
	m_staticModelStandardDrawRequest->m_worldMatrix					= TypeAlias::Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(0.0F));
	m_staticModelStandardDrawRequest->m_worldMaxScale               = 1.0F;
	m_staticModelStandardDrawRequest->m_worldInverseTransposeMatrix = m_staticModelStandardDrawRequest->m_worldMatrix.Transpose();

	m_staticGraoundModelStandardDrawRequest->m_staticModelRecord           = m_staticGraoundModel->GetREFStaticModelRecord();
	m_staticGraoundModelStandardDrawRequest->m_worldMaxScale               = 1.0F;
	m_staticGraoundModelStandardDrawRequest->m_worldInverseTransposeMatrix = m_staticGraoundModelStandardDrawRequest->m_worldMatrix.Transpose();

	const auto& l_staticModelStandardPerObjectDrawRequest = l_renderGraph.FindVALDrawRequestPerObject<Graphics::StaticModelStandardLitPerObjectDrawRequest>().lock();

	if (!l_staticModelStandardPerObjectDrawRequest) { return; }

	l_staticModelStandardPerObjectDrawRequest->AddDrawRequest(m_staticModelStandardDrawRequest);
	l_staticModelStandardPerObjectDrawRequest->AddDrawRequest(m_staticGraoundModelStandardDrawRequest);

	const auto& l_viewport = l_renderer.GetREFRenderArea().GetREFViewport();

	const auto l_aspectRatio = l_viewport.Width / l_viewport.Height;

	// カメラ
	m_camera->Setup(TypeAlias::Math::Matrix::CreateTranslation(0.0F, 1.0F, -1.15F), l_aspectRatio);

	// ライト
	m_lightSystem.ApplyDefaultSettings();

	// JoltPhysicsBody作成テスト
	auto& l_physicsManager = Physics::PhysicsManager::GetInstance();

	const TypeAlias::Math::Vector3& l_floorWorldPosition = { 0.0F, -0.1F, 0.0F };
	const TypeAlias::Math::Vector3& l_floorHalfExtent    = { 5.0F,  0.1F, 5.0F };

	m_staticFloorBodyHandle = l_physicsManager.CreateStaticBoxBody(l_floorWorldPosition, l_floorHalfExtent);

	const TypeAlias::Math::Vector3& l_sphereWorldPosition = { 0.0F, 2.0F, 0.0F };

	m_dynamicSphereBodyHandle = l_physicsManager.CreateDynamicSphereBody(l_sphereWorldPosition, 0.25F);

	l_physicsManager.OptimizeBroadPhase();
}

void FWK::Scene::Update()
{
	if (GetAsyncKeyState('2'))
	{
		m_staticModel					 = nullptr;
		m_staticModelStandardDrawRequest = nullptr;
	}

	// テスト実装カメラ(絶対に後で消す)
	static TypeAlias::Math::Vector3 l_cameraPos = { 0.0F, 1.0F, -1.15F };
	static float					l_rot		= 0.0F;

	if (GetAsyncKeyState(VK_SHIFT))
	{
		if (GetAsyncKeyState('W'))
		{
			l_cameraPos.y += 0.01F;
		}
		else if (GetAsyncKeyState('S'))
		{
			l_cameraPos.y -= 0.01F;
		}
	}
	else if (GetAsyncKeyState('W'))
	{
		l_cameraPos.z += 0.01F;
	}
	else if (GetAsyncKeyState('S'))
	{
		l_cameraPos.z -= 0.01F;
	}

	if (GetAsyncKeyState('A'))
	{
		l_cameraPos.x -= 0.01F;
	}
	else if (GetAsyncKeyState('D'))
	{
		l_cameraPos.x += 0.01F;
	}

	if (GetAsyncKeyState('Q'))
	{
		l_rot -= 1.0F;
	}
	else if (GetAsyncKeyState('E'))
	{
		l_rot += 1.0F;
	}

	m_camera->SetupCameraMatrix(TypeAlias::Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(l_rot)) * TypeAlias::Math::Matrix::CreateTranslation(l_cameraPos));
}