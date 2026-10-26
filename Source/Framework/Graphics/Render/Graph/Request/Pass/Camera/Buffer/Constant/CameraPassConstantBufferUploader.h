#pragma once

namespace FWK::Graphics
{
    // 1フレームの中で描画カメラが変わる(エディターのカメラのプレビューを追加で描く)ため、書き込むたびに書き込み位置を進める
    // (位置を固定すると、GPUが命令を実行する時点では最後に書いたカメラの値しか残らない)
    class CameraPassDynamicConstantBufferUploader final : public DynamicConstantBufferAdvancingWritePositionUploaderBase<Struct::CBCameraPass>
    {
    public:

         CameraPassDynamicConstantBufferUploader()          = default;
        ~CameraPassDynamicConstantBufferUploader() override = default;

        FWK_DEFINE_TYPE_INFO(CameraPassDynamicConstantBufferUploader, DynamicBufferUploaderBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DynamicBufferSharedFactory, FWK::Graphics::CameraPassDynamicConstantBufferUploader)