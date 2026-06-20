#include "Camera.h"

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
	SetCameraMatrix(a_cameraMatrix);

	SetProjectionMatrix(a_aspectRatio,
						a_fovYDegree,
						a_farClip,
						a_nearClip);
}

void FWK::Graphics::Camera::SetCameraMatrix(const TypeAlias::Math::Matrix& a_cameraMatrix)
{
	if (!m_cbCameraPass) { return; }

	m_cameraMatrix = a_cameraMatrix;

	// カメラ行列の逆行列をViewMatrixとして作成する
	// CameraMatrix : カメラのWorldMatrix
	// ViewMatrix   : World空間をカメラ空間へ変換する行列
	m_cbCameraPass->m_viewMatrix = m_cameraMatrix.Invert();

	UpdateViewProjectionMatrix();
}

void FWK::Graphics::Camera::SetProjectionMatrix(const float a_aspectRatio,
												const float a_fovYDegree,
												const float a_farClip,
												const float a_nearClip)
{
	FWK_ASSERT_RETURN_IF_FAILED(a_aspectRatio <= k_invalidAspectRatio, "CameraのAspectRatioが不正なため、ProjectionMatrixの作成に失敗しました。");

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

	m_cbCameraPass->m_nearClip = a_nearClip;
	m_cbCameraPass->m_farClip  = a_farClip;

	// 縦方向FOVの角度を算出する
	// FOVYは画面上端から画面下端までの全体角度なので、
	// 画面中央から上端までの片側角度にするために半分にする
	const float l_halfFOVYRadian = l_fovYRadian * k_halfFOVScale;

	// Z = 1.0の位置で、画面中央から上端までの高さを求める
	m_cbCameraPass->m_tanHalfFOVY = std::tan(l_halfFOVYRadian);

	// Z = 1.0の位置で、画面中央から右端までの幅を求める
	// 横方向の広さは、縦方向の半分の高さに描画画面ののアスペクト比を掛けることで求まる
	m_cbCameraPass->m_tanHalfFOVX = m_cbCameraPass->m_tanHalfFOVY * a_aspectRatio;

	UpdateViewProjectionMatrix();
}

void FWK::Graphics::Camera::SetProjectionMatrix(const TypeAlias::Math::Matrix& a_projectionMatrix)
{
	if (!m_cbCameraPass) { return; }

	m_cbCameraPass->m_projectionMatrix = a_projectionMatrix;
	
	UpdateViewProjectionMatrix();
}

void FWK::Graphics::Camera::UpdateViewProjectionMatrix()
{
	if (!m_cbCameraPass) { return; }

	m_cbCameraPass->m_viewProjectionMatrix = m_cbCameraPass->m_viewMatrix * m_cbCameraPass->m_projectionMatrix;
}