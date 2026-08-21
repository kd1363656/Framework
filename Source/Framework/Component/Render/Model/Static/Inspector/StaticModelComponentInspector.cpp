#include "StaticModelComponentInspector.h"

void FWK::StaticModelComponentInspector::EditInspector(StaticModelComponent& a_staticModelComponent) const
{
	ImGui::SeparatorText(k_fbxFilePathSelectorLabel.data());

	a_staticModelComponent.ModelComponentBase::EditInspector();

	const auto& l_assetFilePathHelper = a_staticModelComponent.GetVALAssetFilePathHelper().lock();
	
	if (!l_assetFilePathHelper)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "アセットファイルパスヘルパーが無効になっておりStaticModelComponentインスペクターの処理に失敗しました。");

		return;
	}

	// ファイルパスが変更されていたらモデルを読み込みなおす
	if (l_assetFilePathHelper->GetVALIsFilePathChangedDirty())
	{
		a_staticModelComponent.LoadModel();
	}

	// 描画方法を選択することができるラジオボタンリスト
	if (Utility::FactoryCheckBoxMapSelector<TypeAlias::StaticModelRegisterDrawRequestStrategyBaseUniqueFactory>(k_drawRequestDataStrategySelectorLabel, a_staticModelComponent.GetMutableRegisterDrawRequestStrategyMap()))
	{
		// Graphicsで実行したいシェーダーに渡したDrawRequestDataが不必要になる可能性を考慮して
		// 一旦ポインターを削除してもう一度使用したいシェーダーに登録を行う
		a_staticModelComponent.SetDrawRequestData(nullptr);
		a_staticModelComponent.SetDrawRequestData(std::make_shared<Struct::StaticModelPerObjectDrawRequestData>());

		// 同じファイルをもう一度読みこみ各シェーダーで使用する
		// モデルの情報を読み込む
		a_staticModelComponent.LoadModel();

		// 各シェーダーにDrawRequestDataを転送する
		a_staticModelComponent.PostDeserialize();
	}
}