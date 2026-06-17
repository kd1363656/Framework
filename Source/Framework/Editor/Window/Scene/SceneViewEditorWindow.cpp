#include "SceneViewEditorWindow.h"

FWK::Editor::SceneViewEditorWindow::SceneViewEditorWindow() = default;
FWK::Editor::SceneViewEditorWindow::~SceneViewEditorWindow()
{
	EditorManager::GetInstance().ReleaseImGuiSRVDescriptorIndex(m_imGuiSRVDescriptorIndex);

	m_imGuiSRVDescriptorIndex = Constant::k_invalidDescriptorIndex;
}

void FWK::Editor::SceneViewEditorWindow::PostDeserialize()
{
	// もし無効なSRVIndexを指し示しているならDescriptorIndexをアロケートする
	if (m_imGuiSRVDescriptorIndex != Constant::k_invalidDescriptorIndex) { return; }

	auto& l_editorManager = EditorManager::GetInstance();

	m_imGuiSRVDescriptorIndex = l_editorManager.AllocateImGuiSRVDescriptorIndex();
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

ImTextureID FWK::Editor::SceneViewEditorWindow::FetchVALSceneViewTextureID()
{
	// GraphicsManagerからRendererへアクセスし、現在フレームのRendererGraphリソースを取得する
	const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	const auto& l_renderer		  = l_graphicsManager.GetREFRenderer	  ();

	// 現在描画に使用しているFrameResourceを取得する
	const auto& l_currentFrameResource = l_renderer.GetREFCurrentFrameResource().lock();

	if (!l_currentFrameResource) { return k_invalidSceneViewTextureID; }

	// フレームリソースで管理されている、レンダーグラフフレームリソースを取得
	const auto& l_renderGraphFrameResource = l_currentFrameResource->GetREFRenderGraphFrameResource();

	// ファイナルカラーパステクスチャを取得
	// (実際にバックバッファに描画する際に使用するガンマ補正などを適用したレンダーターゲットテクスチャ)
	const auto& l_renderTargetPassTexture = l_renderGraphFrameResource.FindVALRenderTargetPassTexture(Enum::RenderGraphResourceType::FinalColor).lock();

	if (!l_renderTargetPassTexture) { return k_invalidSceneViewTextureID; }

	// RenderTargetTexture本体を取得する
	const auto& l_renderTargetTexture = l_renderTargetPassTexture->GetREFRenderTargetTexture();

	if (l_renderTargetTexture.GetVALSRVDescriptorIndex() == Constant::k_invalidDescriptorIndex) { return k_invalidSceneViewTextureID; }

	// ImGuiでTextureを表示するにはTextureをShaderから読めるSRVが必要になる
	// ここではRenderTargetTextureに割り当てられているSRVのDescriptorIndexを取得する
	const auto l_srvDescriptorIndex = l_renderTargetTexture.GetVALSRVDescriptorIndex();

	if (l_srvDescriptorIndex == Constant::k_invalidDescriptorIndex) { return k_invalidSceneViewTextureID; }

	const auto& l_editorManager = EditorManager::GetInstance();

	// もし無効なDescriptorIndexならreturn
	if (m_imGuiSRVDescriptorIndex == Constant::k_invalidDescriptorIndex) { return k_invalidSceneViewTextureID; }

	const auto& l_resourceContext = l_graphicsManager.GetREFResourceContext();
	
	// メイン描画用SRVDescriptorを、ImGui用SRVDescriptorへコピーする
	if (const auto& l_srvDescriptorPool = l_resourceContext.GetREFSRVDescriptorPool();
		!l_editorManager.CopyGraphicsSRVDescriptor(l_srvDescriptorPool, l_srvDescriptorIndex, m_imGuiSRVDescriptorIndex)) 
	{
		return k_invalidSceneViewTextureID; 
	}

	return l_editorManager.FetchVALImGuiTextureID(m_imGuiSRVDescriptorIndex);
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