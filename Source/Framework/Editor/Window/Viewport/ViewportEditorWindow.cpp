#include "ViewportEditorWindow.h"

FWK::Editor::ViewportEditorWindow::ViewportEditorWindow () = default;
FWK::Editor::ViewportEditorWindow::~ViewportEditorWindow()
{
	for (const auto& l_srvDescriptorIndex : m_imGuiSRVDescriptorIndexList)
	{
		EditorManager::GetInstance().ReleaseImGuiSRVDescriptorIndex(l_srvDescriptorIndex);
	}

	m_imGuiSRVDescriptorIndexList.clear();
}

void FWK::Editor::ViewportEditorWindow::PostDeserialize()
{
	SetupViewportTextureDescriptors();
}

void FWK::Editor::ViewportEditorWindow::Draw()
{
	// Viewport用のImGuiウィンドウを開始する
	if (!ImGui::Begin(k_editorName.data())) 
	{
		ImGui::End();
		return; 
	}

	// Viewport画像より先にツールバーを書く。
	// これにより画面へ重ならず、Viewport上部へ工程表示される
	m_toolbar.Draw  ();
	ImGui::Separator();

	// 現在のViewportウィンドウ内で、実際に画像を表示できる領域サイズを取得する
	// Dockingでウィンドウサイズが変わると、この値も変わる
	const ImVec2& l_viewportSize = ImGui::GetContentRegionAvail();

	if (l_viewportSize.x  <= k_minViewportSize ||
		l_viewportSize.y  <= k_minViewportSize)
	{
		// 無効サイズの場合は描画をしない
		ImGui::End();

		return;
	}

	// RenderGraphで作成された最終カラーTextureを、ImGuiで表示できるTextureIDとして取得する
	// 今回はRenderTargetTextureの解像度変更は行わなず、既存の描画結果をViewport内に拡縮表示する
	const auto l_viewportTextureID = FetchVALViewportTextureID();

	// 無効なテクスチャなのでreturn
	if (l_viewportTextureID == k_invalidViewportTextureID)
	{
		ImGui::End();

		return; 
	}

	// 現在のViewport表示領域の縦横比を、
	// このViewportへ登録されているCameraへ毎フレーム反映する
	ApplyViewportAspectRatioToCamera(l_viewportSize);

	// Viewportの表示領域全体に、取得したTextureを描画する
	DrawViewportTexture(l_viewportTextureID, l_viewportSize);

	ImGui::End();
}

void FWK::Editor::ViewportEditorWindow::SetupViewportTextureDescriptors()
{
	m_imGuiSRVDescriptorIndexList.clear();

	// GraphicsManagerからRendererへアクセスし、現在フレームのRendererGraphリソースを取得する
	const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	const auto& l_renderer		  = l_graphicsManager.GetREFRenderer	  ();

	for (const auto& l_frameResource : l_renderer.GetREFFrameResourceList())
	{
		FWK_ASSERT_RETURN_IF(!l_frameResource, "フレームリソースの取得に失敗しており、PostDeserialize処理に失敗しました。");

		// フレームリソースで管理されている、レンダーグラフフレームリソースを取得
		const auto& l_renderGraphFrameResource = l_frameResource->GetREFRenderGraphFrameResource();

		// ファイナルカラーパステクスチャを取得
		// (実際にバックバッファに描画する際に使用するガンマ補正などを適用したレンダーターゲットテクスチャ)
		const auto& l_renderTargetPassTexture = l_renderGraphFrameResource.FindVALRenderTargetPassTexture(Enum::RenderGraphRenderTargetType::FinalColor).lock();

		FWK_ASSERT_RETURN_IF(!l_renderTargetPassTexture, "レンダーターゲットパステクスチャの取得に失敗しており、PostDeserialize処理に失敗しました。");

		// RenderTargetTexture本体を取得する
		const auto& l_renderTargetTexture = l_renderTargetPassTexture->GetREFRenderTargetTexture();

		if (l_renderTargetTexture.GetVALSRVDescriptorIndex() == Graphics::DescriptorHeap::k_invalidDescriptorIndex) { return; }

		// ImGuiでTextureを表示するにはTextureをShaderから読めるSRVが必要になる
		// ここではRenderTargetTextureに割り当てられているSRVのDescriptorIndexを取得する
		const auto l_srvDescriptorIndex = l_renderTargetTexture.GetVALSRVDescriptorIndex();

		FWK_ASSERT_RETURN_IF(l_srvDescriptorIndex == Graphics::DescriptorHeap::k_invalidDescriptorIndex, "SRVDescriptorIndexが無効値になっており、PostDeserialize処理に失敗しました。");

		auto& l_editorManager = EditorManager::GetInstance();

		// SRVDescriptorIndexをアロケート
		m_imGuiSRVDescriptorIndexList.emplace_back(l_editorManager.AllocateImGuiSRVDescriptorIndex());

		const auto& l_resourceContext = l_graphicsManager.GetREFResourceContext();

		// メイン描画用SRVDescriptorを、ImGui用SRVDescriptorへコピーする
		const auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetREFCBVSRVUAVDescriptorPool();

		FWK_ASSERT_RETURN_IF(!l_editorManager.CopyGraphicsSRVDescriptor(l_cbvSRVUAVDescriptorPool, l_srvDescriptorIndex, m_imGuiSRVDescriptorIndexList.back()), "SRVDescriptorのコピー処理に失敗しました。");
	}	
}

ImTextureID FWK::Editor::ViewportEditorWindow::FetchVALViewportTextureID() const
{
	const auto& l_editorManager = EditorManager::GetInstance();

	const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	const auto& l_renderer		  = l_graphicsManager.GetREFRenderer	  ();

	const auto& l_currentFrameResourceIndex = l_renderer.GetREFCurrentFrameResourceIndex();

	// レンダーラーから現在のFrameResourceのインデックスを取得
	FWK_ASSERT_RETURN_VALUE_IF(m_imGuiSRVDescriptorIndexList.size() <= l_currentFrameResourceIndex, "フレームリソースの数がimGuiSRVDescriptorIndexListのサイズを超えています", k_invalidViewportTextureID);

	// もし無効なDescriptorIndexならreturn
	if (m_imGuiSRVDescriptorIndexList[l_currentFrameResourceIndex] == Graphics::DescriptorHeap::k_invalidDescriptorIndex) { return k_invalidViewportTextureID; }

	return l_editorManager.FetchVALImGuiTextureID(m_imGuiSRVDescriptorIndexList[l_currentFrameResourceIndex]);
}

void FWK::Editor::ViewportEditorWindow::DrawViewportTexture(const ImTextureID& a_textureID, const ImVec2& a_viewportSize) const
{
	// Texture全体を表示するため、UV範囲は左上から右下までを指定する
	const auto l_uvMIN = ImVec2(k_viewportUVMINX, k_viewportUVMINY);
	const auto l_uvMAX = ImVec2(k_viewportUVMAXX, k_viewportUVMAXY);

	// 取得したTextureをViewportの表示領域いっぱいに描画する
	// a_viewportSizeにGetContentRegionAvailの値を渡しているため、Dockingの拡縮に追従する
	ImGui::Image(a_textureID, 
		         a_viewportSize, 
		         l_uvMIN,
		         l_uvMAX);
}

void FWK::Editor::ViewportEditorWindow::ApplyViewportAspectRatioToCamera(const ImVec2& a_viewportSize) const
{
	FWK_ASSERT_RETURN_IF(a_viewportSize.x <= k_minViewportSize ||
		                 a_viewportSize.y <= k_minViewportSize,
		                 "Viewportの表示サイズが不正なため、CameraへAspectRatioを適用できません。");

	const auto& l_currentSceneCamera = m_currentSceneCamera.lock();

	if (!l_currentSceneCamera) { return; }

	const float l_aspectRatio = a_viewportSize.x / a_viewportSize.y;

	l_currentSceneCamera->ApplyPerspectiveAspectRatio(l_aspectRatio);
}