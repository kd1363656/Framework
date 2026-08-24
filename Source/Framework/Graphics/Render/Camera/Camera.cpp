#include "Camera.h"

void FWK::Graphics::Camera::Setup(const TypeAlias::Math::Matrix& a_cameraMatrix, 
								  const float					 a_aspectRatio,
								  const float					 a_fovYDegree, 
								  const float					 a_farClip,
								  const float					 a_nearClip)
{
	// ビュー行列の作成
	SetupPerspective(a_cameraMatrix,
					 a_aspectRatio,
					 a_fovYDegree,
				     a_farClip,
					 a_nearClip);

	// 定数バッファへの登録
	RegisterCBCameraPass();

	// エディターにこのカメラクラスを登録
	RegisterToEditorViewportWindow();
}
void FWK::Graphics::Camera::Setup()
{
	// ビュー行列の作成
	SetupPerspective(m_cameraMatrix,
					 m_aspectRatio,
					 m_fovYDegree,
				     m_farClip,
					 m_nearClip);

	// 定数バッファへの登録
	RegisterCBCameraPass();

	// エディターにこのカメラクラスを登録
	RegisterToEditorViewportWindow();
}

void FWK::Graphics::Camera::ApplyCameraMatrix(const TypeAlias::Math::Matrix& a_cameraMatrix)
{
	if (!m_cbCameraPass) { return; }

	m_cameraMatrix = a_cameraMatrix;

	// カメラ行列の逆行列をViewMatrixとして作成する
	// CameraMatrix : カメラのWorldMatrix
	// ViewMatrix   : World空間をカメラ空間へ変換する行列
	m_cbCameraPass->m_viewMatrix = m_cameraMatrix.Invert();

	// 行列からワールド座標を取得
	m_cbCameraPass->m_cameraWorldPosition = m_cameraMatrix.Translation();

	UpdateViewProjectionMatrix();
}

void FWK::Graphics::Camera::ApplyProjectionMatrix(const float a_aspectRatio,
												  const float a_fovYDegree,
												  const float a_farClip,
												  const float a_nearClip)
{
	FWK_ASSERT_RETURN_IF(a_aspectRatio <= Constant::k_cameraInvalidAspectRatio, "CameraのAspectRatioが不正なため、ProjectionMatrixの作成に失敗しました。");

	if (!m_cbCameraPass) { return; }

	// X<ConvertToRadians(度数法の角度);
	// 度数法で指定された視野角を、DirectXMathが扱うラジアンへ変換する
	const float l_fovYRadian = DirectX::XMConvertToRadians(a_fovYDegree);

	// DirectX12で扱いやすい左手系のProjectionMatrixを作成する
	// CreatePerspectiveFieldOfView(縦方向の視野角、
	//								画面の横縦比、
	//							    近クリップ、
	//								遠クリップ);
 	m_cbCameraPass->m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(l_fovYRadian,
																		   a_aspectRatio,
																		   a_nearClip,
																		   a_farClip);

	// Perspective行列を後からAspectRatioだけ変更して
	// 再作成できるよう、、現在の設定をCameraへ保持する
	m_aspectRatio = a_aspectRatio;
	m_fovYDegree  = a_fovYDegree;
	m_farClip     = a_farClip;
	m_nearClip    = a_nearClip;

	m_cbCameraPass->m_nearClip = a_nearClip;
	m_cbCameraPass->m_farClip  = a_farClip;

	UpdatePerspectiveProjectionMatrix();
}

void FWK::Graphics::Camera::ApplyProjectionMatrix(const TypeAlias::Math::Matrix& a_projectionMatrix)
{
	if (!m_cbCameraPass) { return; }

	m_cbCameraPass->m_projectionMatrix = a_projectionMatrix;
	
	UpdateViewProjectionMatrix();
}

void FWK::Graphics::Camera::ApplyPerspectiveAspectRatio(const float a_aspectRatio)
{
	FWK_ASSERT_RETURN_IF(a_aspectRatio <= Constant::k_cameraInvalidAspectRatio, "PerspectiveのAspectRatioが不正なため、ProjectionMatrixを更新できません。");
	FWK_ASSERT_RETURN_IF(m_aspectRatio <= Constant::k_cameraInvalidAspectRatio, "CameraのPerspective設定前にAspectRatioを変更しようとしました。");

	if (!m_cbCameraPass) { return; }

	// Viewportの形状変更で変えるのはAspectRatioだけ。
	// FOVY,NearClip,FarClipは以前の値を維持する
	m_aspectRatio = a_aspectRatio;

	UpdatePerspectiveProjectionMatrix();
}

void FWK::Graphics::Camera::SetupPerspective(const TypeAlias::Math::Matrix& a_cameraMatrix,
											 const float					a_aspectRatio,
											 const float					a_fovYDegree, 
										     const float					a_farClip, 
											 const float					a_nearClip)
{
	// SetupPerspectiveは、を最初に絶対にしてほしいためここで定数バッファのポインタのインスタンスを作成する
	if (!m_cbCameraPass)
	{
		m_cbCameraPass = std::make_shared<Struct::CBCameraPass>();
	}

	// カメラ行列と射影行列をセット
	ApplyCameraMatrix(a_cameraMatrix);
	
	ApplyProjectionMatrix(a_aspectRatio,
						  a_fovYDegree,
						  a_farClip,
						  a_nearClip);
}

void FWK::Graphics::Camera::UpdateViewProjectionMatrix()
{
	if (!m_cbCameraPass) { return; }

	m_cbCameraPass->m_viewProjectionMatrix = m_cbCameraPass->m_viewMatrix * m_cbCameraPass->m_projectionMatrix;
}
void FWK::Graphics::Camera::UpdatePerspectiveProjectionMatrix()
{
	FWK_ASSERT_RETURN_IF(!m_cbCameraPass,                                       "CBCameraPassが無効なため、PerspectiveProjectionを更新できません。");
	FWK_ASSERT_RETURN_IF(m_aspectRatio <= Constant::k_cameraInvalidAspectRatio, "Cameraに保存されているAspectRatioが不正なため、PerspectiveProjectionを更新できません。");

	const float l_fovYRadian = DirectX::XMConvertToRadians(m_fovYDegree);

	m_cbCameraPass->m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(l_fovYRadian,
		                                                                   m_aspectRatio,
		                                                                   m_nearClip,
		                                                                   m_farClip);

	// 縦方向FOVの角度を算出する
	// FOVYは画面上端から画面下端までの全体角度なので、
	// 画面中央から上端までの片側角度にするために半分にする
	const float l_halfFOVYRadian = l_fovYRadian * k_halfFOVScale;

	// Z = 1.0の位置で、画面中央から上端までの高さを求める
	m_cbCameraPass->m_tanHalfFOVY = std::tan(l_halfFOVYRadian);

	// Z = 1.0の位置で、画面中央から右端までの幅を求める
	// 横方向の広さは、縦方向の半分の高さに描画画面ののアスペクト比を掛けることで求まる
	m_cbCameraPass->m_tanHalfFOVX = m_cbCameraPass->m_tanHalfFOVY * m_aspectRatio;

	UpdateViewProjectionMatrix();
}

void FWK::Graphics::Camera::RegisterCBCameraPass()
{
	      auto& l_graphicsManager  = FWK::Graphics::GraphicsManager::GetInstance  ();
	      auto& l_renderer		   = l_graphicsManager.GetMutableREFRenderer      ();
	const auto& l_renderGraph	   = l_renderer.GetREFRenderGraph			      ();
	      auto& l_shadowContext    = l_renderer.GetMutableREFShadowContext        ();
		  auto& l_cascadeShadowMap = l_shadowContext.GetMutableREFCascadeShadowMap();

	if (const auto& l_cameraPassDrawRequest = l_renderGraph.FindVALDrawRequestPass<CameraPassDrawRequest>().lock();
		l_cameraPassDrawRequest)
	{
		// 定数バッファの変更を反映するためにカメラクラスの定数バッファデータを送信する
		l_cameraPassDrawRequest->SetSourceConstantBuffer(m_cbCameraPass);	
	}

	// Cascade計算で使用するCameraの定数バッファを登録する
	l_cascadeShadowMap.SetCBCameraPass(m_cbCameraPass);
}

void FWK::Graphics::Camera::RegisterToEditorViewportWindow()
{
	const auto& l_editorManager = Editor::EditorManager::GetInstance();

	const auto& l_viewportWindow = l_editorManager.FindWindowEditor<Editor::ViewportEditorWindow>().lock();

	if (!l_viewportWindow) { return; }

	l_viewportWindow->SetCamera(weak_from_this());
}