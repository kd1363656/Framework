#pragma once

namespace FWK::Editor
{
    // エディターのビューポート専用の、自由に飛び回れるカメラ(デバッグビルドでのみ使える)
    //
    // 有効にすると、ビューポートはこのカメラから見た絵になる
    // ただしカリング(見えない物を描かない判定)はシーンのメインカメラのまま行うため、
    // 「メインカメラがどこを描いて、どこを消しているか」を外から見ることができる
    //
    // 操作 : ビューポート上で右ボタンを押している間、マウスで向きを変え、WASDで前後左右、E / Qで上下に動く(Shiftで速く動く)
    //        ホイールで前後に動く
    class ViewportDebugCamera final
    {
    public:

         ViewportDebugCamera() = default;
        ~ViewportDebugCamera() = default;

        void Deserialize(const nlohmann::json& a_rootJson);

        void Update(const float a_aspectRatio, const bool a_isViewportHovered);

        // 描画に使うカメラとカリングに使うカメラを、このカメラの状態に合わせてCameraPassDrawRequestへ設定する
        // (無効のときは差し替えを外し、メインカメラで描画する)
        void ApplyRenderView  (Graphics::CameraPassDrawRequest& a_cameraPassDrawRequest) const;
        void ReleaseRenderView(Graphics::CameraPassDrawRequest& a_cameraPassDrawRequest) const;

        // メインカメラの見える範囲(視錐台)を線で描く
        void DrawMainCameraFrustum(ImDrawList&                        a_drawList,
                                   const Struct::ViewportScreenRect&  a_viewportRect,
                                   const Struct::CBCameraPass&        a_drawCameraConstantBuffer,
                                   const Struct::CBCameraPass&        a_mainCameraConstantBuffer) const;

        void EditInspector();

        nlohmann::json Serialize() const;

        // 指定したカメラ行列(カメラのワールド行列)と同じ位置、同じ向きへ移動する
        void MoveToCameraMatrix(const TypeAlias::Math::Matrix& a_cameraMatrix);

        void SetPosition(const TypeAlias::Math::Vector3& a_set) { m_position = a_set; }

        void SetYawDegree          (const float a_set) { m_yawDegree           = a_set; }
        void SetPitchDegree        (const float a_set) { m_pitchDegree         = a_set; }
        void SetMoveSpeed          (const float a_set) { m_moveSpeed           = a_set; }
        void SetRotationSpeedDegree(const float a_set) { m_rotationSpeedDegree = a_set; }

        void SetIsEnabled              (const bool a_set) { m_isEnabled               = a_set; }
        void SetIsCullingDebugVisualize(const bool a_set) { m_isCullingDebugVisualize = a_set; }
        void SetIsDrawMainCameraFrustum(const bool a_set) { m_isDrawMainCameraFrustum = a_set; }

        // デバッグビルドで、かつ有効にしているときだけtrue
        bool FetchVALIsActive() const;

        const auto& GetREFPosition() const { return m_position; }

        float GetVALYawDegree          () const { return m_yawDegree; }
        float GetVALPitchDegree        () const { return m_pitchDegree; }
        float GetVALMoveSpeed          () const { return m_moveSpeed; }
        float GetVALRotationSpeedDegree() const { return m_rotationSpeedDegree; }

        bool GetVALIsEnabled              () const { return m_isEnabled; }
        bool GetVALIsCullingDebugVisualize() const { return m_isCullingDebugVisualize; }
        bool GetVALIsDrawMainCameraFrustum() const { return m_isDrawMainCameraFrustum; }
        bool GetVALIsControlling          () const { return m_isControlling; }

    private:

        void UpdateInput       (const bool  a_isViewportHovered);
        void UpdateCameraMatrix(const float a_aspectRatio);

        TypeAlias::Math::Quaternion CalculateRotation() const;

        void DrawClippedLine(ImDrawList&                       a_drawList,
                             const Struct::ViewportScreenRect& a_viewportRect,
                             const TypeAlias::Math::Matrix&    a_viewProjectionMatrix,
                             const TypeAlias::Math::Vector3&   a_startPosition,
                             const TypeAlias::Math::Vector3&   a_endPosition) const;

        static constexpr std::size_t k_frustumCornerCount = 8ULL;
        static constexpr std::size_t k_frustumEdgeCount   = 12ULL;

        static constexpr std::array<DirectX::XMFLOAT3, k_frustumCornerCount> k_ndcFrustumCornerList =
        {
            DirectX::XMFLOAT3{ -1.0F, -1.0F, 0.0F },
            DirectX::XMFLOAT3{  1.0F, -1.0F, 0.0F },
            DirectX::XMFLOAT3{  1.0F,  1.0F, 0.0F },
            DirectX::XMFLOAT3{ -1.0F,  1.0F, 0.0F },
            DirectX::XMFLOAT3{ -1.0F, -1.0F, 1.0F },
            DirectX::XMFLOAT3{  1.0F, -1.0F, 1.0F },
            DirectX::XMFLOAT3{  1.0F,  1.0F, 1.0F },
            DirectX::XMFLOAT3{ -1.0F,  1.0F, 1.0F },
        };

        static constexpr std::array<std::pair<std::size_t, std::size_t>, k_frustumEdgeCount> k_frustumEdgeList =
        {
            std::pair<std::size_t, std::size_t>{ 0ULL, 1ULL },
            std::pair<std::size_t, std::size_t>{ 1ULL, 2ULL },
            std::pair<std::size_t, std::size_t>{ 2ULL, 3ULL },
            std::pair<std::size_t, std::size_t>{ 3ULL, 0ULL },
            std::pair<std::size_t, std::size_t>{ 4ULL, 5ULL },
            std::pair<std::size_t, std::size_t>{ 5ULL, 6ULL },
            std::pair<std::size_t, std::size_t>{ 6ULL, 7ULL },
            std::pair<std::size_t, std::size_t>{ 7ULL, 4ULL },
            std::pair<std::size_t, std::size_t>{ 0ULL, 4ULL },
            std::pair<std::size_t, std::size_t>{ 1ULL, 5ULL },
            std::pair<std::size_t, std::size_t>{ 2ULL, 6ULL },
            std::pair<std::size_t, std::size_t>{ 3ULL, 7ULL },
        };

        static constexpr ImU32 k_frustumLineColor = IM_COL32(255, 214, 64, 255);

        static constexpr float k_frustumLineThickness = 1.5F;

        static constexpr float k_maxPitchDegree     =  89.0F;
        static constexpr float k_minPitchDegree     = -89.0F;
        static constexpr float k_fastMoveMultiplier =  3.0F;
        static constexpr float k_normalMultiplier   =  1.0F;
        static constexpr float k_wheelMoveSecond    =  0.1F;
        static constexpr float k_noRollRadian       =  0.0F;
        static constexpr float k_positiveAxisSign   =  1.0F;
        static constexpr float k_negativeAxisSign   = -1.0F;

        Graphics::Camera m_camera = {};

        ViewportDebugCameraEditInspector m_editInspector = {};

        Converter::ViewportDebugCameraJsonConverter m_jsonConverter = {};

        TypeAlias::Math::Vector3 m_position = { Constant::k_viewportDebugCameraDefaultPositionX,
                                                Constant::k_viewportDebugCameraDefaultPositionY,
                                                Constant::k_viewportDebugCameraDefaultPositionZ };

        float m_yawDegree           = Constant::k_viewportDebugCameraDefaultYawDegree;
        float m_pitchDegree         = Constant::k_viewportDebugCameraDefaultPitchDegree;
        float m_moveSpeed           = Constant::k_viewportDebugCameraDefaultMoveSpeed;
        float m_rotationSpeedDegree = Constant::k_viewportDebugCameraDefaultRotationSpeedDegree;

        bool m_isEnabled               = Constant::k_viewportDebugCameraDefaultIsEnabled;
        bool m_isCullingDebugVisualize = Constant::k_viewportDebugCameraDefaultIsCullingDebugVisualize;
        bool m_isDrawMainCameraFrustum = Constant::k_viewportDebugCameraDefaultIsDrawMainCameraFrustum;
        bool m_isControlling           = false;
        bool m_isCameraSetup           = false;
    };
}
