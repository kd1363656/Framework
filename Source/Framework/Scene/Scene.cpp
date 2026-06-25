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
	m_charaModelStandardDrawRequest->m_worldInverseTransposeMatrix = m_groundModelStandardDrawRequest->m_worldMatrix.Transpose();

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

	m_camera->SetupCameraMatrix(TypeAlias::Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(l_rot)) * TypeAlias::Math::Matrix::CreateTranslation(l_cameraPos));
}