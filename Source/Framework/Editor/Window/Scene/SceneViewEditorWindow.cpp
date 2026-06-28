#include "SceneViewEditorWindow.h"

FWK::Editor::SceneViewEditorWindow::SceneViewEditorWindow() = default;
FWK::Editor::SceneViewEditorWindow::~SceneViewEditorWindow()
{
	for (const auto& l_srvDescriptorIndex : m_imGuiSRVDescriptorIndexList)
	{
		EditorManager::GetInstance().ReleaseImGuiSRVDescriptorIndex(l_srvDescriptorIndex);
	}

	m_imGuiSRVDescriptorIndexList.clear();
}

void FWK::Editor::SceneViewEditorWindow::PostDeserialize()
{
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

		if (l_renderTargetTexture.GetVALSRVDescriptorIndex() == Constant::k_invalidDescriptorIndex) { return; }

		// ImGuiでTextureを表示するにはTextureをShaderから読めるSRVが必要になる
		// ここではRenderTargetTextureに割り当てられているSRVのDescriptorIndexを取得する
		const auto l_srvDescriptorIndex = l_renderTargetTexture.GetVALSRVDescriptorIndex();

		FWK_ASSERT_RETURN_IF(l_srvDescriptorIndex == Constant::k_invalidDescriptorIndex, "SRVDescriptorIndexが無効値になっており、PostDeserialize処理に失敗しました。");

		auto& l_editorManager = EditorManager::GetInstance();

		// SRVDescriptorIndexをアロケート
		m_imGuiSRVDescriptorIndexList.emplace_back(l_editorManager.AllocateImGuiSRVDescriptorIndex());

		const auto& l_resourceContext = l_graphicsManager.GetREFResourceContext();

		// メイン描画用SRVDescriptorを、ImGui用SRVDescriptorへコピーする
		const auto& l_srvDescriptorPool = l_resourceContext.GetREFSRVDescriptorPool();

		FWK_ASSERT_RETURN_IF(!l_editorManager.CopyGraphicsSRVDescriptor(l_srvDescriptorPool, l_srvDescriptorIndex, m_imGuiSRVDescriptorIndexList.back()), "SRVDescriptorのコピー処理に失敗しました。");
	}
}

void FWK::Editor::SceneViewEditorWindow::Draw()
{
	// SceneView用のImGuiウィンドウを開始する
	if (!ImGui::Begin(k_editorName.data())) 
	{
		ImGui::End();
		return; 
	}
	
	// 現在のSceneViewウィンドウ内で、実際に画像を表示できる領域サイズを取得する
	// Dockingでウィンドウサイズが変わると、この値も変わる
	const ImVec2& l_sceneViewSize = ImGui::GetContentRegionAvail();

	if (l_sceneViewSize.x <= k_minSceneViewSize ||
		l_sceneViewSize.y <= k_minSceneViewSize)
	{
		// 無効サイズの場合は描画をしない
		ImGui::End();
		return;
	}

	// RenderGraphで作成された最終カラーTextureを、ImGuiで表示できるTextureIDとして取得する
	// 今回はRenderTargetTextureの解像度変更は行わなず、既存の描画結果をSceneView内に拡縮表示する
	const auto l_sceneViewTextureID = FetchVALSceneViewTextureID();

	// 無効なテクスチャなのでreturn
	if (l_sceneViewTextureID == k_invalidSceneViewTextureID)
	{
		ImGui::End();
		return; 
	}

	// SceneViewの表示領域全体に、取得したTextureを描画する
	DrawSceneViewTexture(l_sceneViewTextureID, l_sceneViewSize);

	ImGui::End();
}

ImTextureID FWK::Editor::SceneViewEditorWindow::FetchVALSceneViewTextureID() const
{
	const auto& l_editorManager = EditorManager::GetInstance();

	const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	const auto& l_renderer		  = l_graphicsManager.GetREFRenderer	  ();

	const auto& l_currentFrameResourceIndex = l_renderer.GetREFCurrentFrameResourceIndex();

	// レンダーラーから現在のFrameResourceのインデックスを取得
	FWK_ASSERT_RETURN_VALUE_IF(m_imGuiSRVDescriptorIndexList.size() <= l_currentFrameResourceIndex, "フレームリソースの数がimGuiSRVDescriptorIndexListのサイズを超えています", k_invalidSceneViewTextureID);

	// もし無効なDescriptorIndexならreturn
	if (m_imGuiSRVDescriptorIndexList[l_currentFrameResourceIndex] == Constant::k_invalidDescriptorIndex) { return k_invalidSceneViewTextureID; }

	return l_editorManager.FetchVALImGuiTextureID(m_imGuiSRVDescriptorIndexList[l_currentFrameResourceIndex]);
}

void FWK::Editor::SceneViewEditorWindow::DrawSceneViewTexture(const ImTextureID& a_textureID, const ImVec2& a_sceneViewSize) const
{
	// Texture全体を表示するため、UV範囲は左上から右下までを指定する
	const auto l_uvMIN = ImVec2(k_sceneViewUVMINX, k_sceneViewUVMINY);
	const auto l_uvMAX = ImVec2(k_sceneViewUVMAXX, k_sceneViewUVMAXY);

	// 取得したTextureをSceneViewの表示領域いっぱいに描画する
	// a_sceneViewSizeにGetContentRegionAvailの値を渡しているため、Dockingの拡縮に追従する
	ImGui::Image(a_textureID, a_sceneViewSize, l_uvMIN, l_uvMAX);
}