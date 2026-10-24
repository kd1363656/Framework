#pragma once

namespace FWK::Graphics
{
    // シェーダーごとに必要なデータだけを持つMaterialの基底クラス
    //
    // ・派生クラスは、自身のシェーダーが使うデータだけをメンバとして持ち、
    //   そのデータを1要素のStructuredBuffer(GPUDataBuffer)としてGPUへ送る
    // ・描画側はMaterialの型を知らなくても、GPUDataBufferのSRVDescriptorIndexをシェーダーへ渡すだけでよい
    //   (シェーダー側が自身の型のStructuredBufferとして読む)ため、ダウンキャストを必要としない
    // ・どのPassで描画するかは、型ごとのStaticTypeIDで判定する
    class MaterialBase : public AssetRecordBase
    {
    public:

         MaterialBase()          = default;
        ~MaterialBase() override = default;

        MaterialBase(const MaterialBase&)           = delete;
        MaterialBase(      MaterialBase&&) noexcept = default;

        MaterialBase& operator=(const MaterialBase&)           = delete;
        MaterialBase& operator=(      MaterialBase&&) noexcept = default;

        // .materialファイルへの保存、読み込み
        // ファイルの先頭には自身の型名を保存し、読み込み時に型名が自身と違えば失敗する
        bool SaveBinaryData(const std::filesystem::path& a_filePath) const;
        bool LoadBinaryData(const std::filesystem::path& a_filePath);

        // 型名、ファイルパスの表示と、派生クラスごとの値の編集を行う
        // 値が変わった場合はGPUDataBufferの作り直しが必要になるため、作り直し待ちの状態にする
        void EditInspector();

        // テクスチャなど、実行時に必要なリソースを読み込む
        virtual void BuildRuntimeResources() = 0;

        // シェーダーへ送るデータを持つGPUDataBufferを作成する
        virtual bool CreateGPUDataBuffer(const Device&                                                   a_device,
                                         const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
                                               std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
                                               TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool) = 0;

        // GPUDataBufferを遅延解放へ積む
        // 参照数が0になった時に加え、Inspectorでの編集後にGPUDataBufferを作り直す時にも使う
        bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) override;

        // 型名からMaterialの先頭名を取得する(例 : StandardLitMaterial -> StandardLit)
        std::string_view FetchVALFilePrefix() const;

        // 先頭名 + 名前 + 拡張子で.materialのファイル名を作る(例 : StandardLit + Skin -> StandardLitSkin.material)
        std::wstring FetchVALFileName(const std::wstring& a_name) const;

        void SetIsGPUDataDirty(const bool a_set) { m_isGPUDataDirty = a_set; }

        auto GetVALStaticTypeID() const { return GetREFRuntimeTypeINFO().k_staticTypeID; }

        auto GetVALGPUDataBufferSRVDescriptorIndex() const { return m_gpuDataBuffer.GetVALSRVDescriptorIndex(); }

        bool IsGPUDataDirty() const { return m_isGPUDataDirty; }

    protected:

        // Payload(派生クラス固有のデータ)の保存サイズ計算、書き込み、読み込み
        virtual std::uint64_t CalculateBinaryPayloadSize(const Converter::MaterialBinaryConverter& a_materialBinaryConverter)                                          const = 0;
        virtual void          WriteBinaryPayload        (const Converter::MaterialBinaryConverter& a_materialBinaryConverter, std::uint64_t& a_memoryWriteOffset) const = 0;
        virtual bool          ReadBinaryPayload         (const Converter::MaterialBinaryConverter& a_materialBinaryConverter, std::uint64_t& a_memoryReadOffset)        = 0;

        // 派生クラスごとの値の編集、値が変わった場合はtrueを返す
        virtual bool EditInspectorParameter() = 0;

        template <typename GPUDataType>
        bool CreateGPUDataBufferFromGPUData(const GPUDataType&                                              a_gpuData,
                                            const Device&                                                   a_device,
                                            const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
                                                  std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
                                                  TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool)
        {
            const std::vector<GPUDataType> l_gpuDataList = { a_gpuData };

            return m_gpuDataBuffer.Create(l_gpuDataList,
                                          a_device,
                                          a_gpuMemoryAllocator,
                                          a_bufferUploadCommandList,
                                          a_cbvSRVUAVDescriptorPool);
        }

        // .materialファイルからの相対パスを、.materialファイルの場所を基準にしたパスへ変換する
        // ※ パスが空の場合は空のまま返す(テクスチャはデフォルトテクスチャになる)
        std::filesystem::path FetchVALResolvedFilePath(const std::wstring& a_relativeFilePath) const;

        TypeAlias::DescriptorIndex FetchVALTextureSRVDescriptorIndex(const Texture& a_texture) const;

    private:

        static constexpr std::string_view k_classNameSuffix = "Material";

        static constexpr std::uint64_t k_initialMemoryOffset = 0ULL;

        static constexpr std::string_view k_typeNameLabel   = "Type : %s";
        static constexpr std::string_view k_filePathLabel   = "File : %s";
        static constexpr std::string_view k_saveButtonLabel = "Save";

        StaticStructuredBuffer m_gpuDataBuffer = {};

        bool m_isGPUDataDirty = false;

        FWK_DEFINE_TYPE_INFO_ROOT(MaterialBase)
    };
}