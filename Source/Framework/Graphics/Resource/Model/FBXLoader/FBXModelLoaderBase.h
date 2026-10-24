#pragma once

namespace FWK::Graphics
{
    class FBXModelLoaderBase
    {
    public:

                 FBXModelLoaderBase() = default;
        virtual ~FBXModelLoaderBase() = default;

    protected:

        ufbx_scene* LoadFBXScene(const std::filesystem::path& a_filePath) const;

        // FBXのMaterialからStandardLitMaterialを作り、モデルファイルと同じ場所へ.materialファイルとして保存する
        // 戻り値はモデルファイルの場所を基準にした.materialファイルの相対パス(Materialが無い場合は空)
        // ※ 既に.materialファイルがある場合は、編集した内容を消さないように上書きしない
        std::wstring ExtractModelMaterial(const ufbx_material* a_fbxMaterial, const std::filesystem::path& a_modelFilePath) const;

        void DestroyFBXScene(ufbx_scene* a_fbxScene) const;

        TypeAlias::Math::Vector3 FetchWorldVertexPosition(const ufbx_node* a_fbxNode, const ufbx_mesh*    a_fbxMesh, const std::uint32_t a_vertexIndex) const;
        TypeAlias::Math::Vector2 FetchVertexUV           (const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex)                                const;
        TypeAlias::Math::Vector3 FetchWorldVertexNormal  (const ufbx_node* a_fbxNode, const ufbx_mesh*    a_fbxMesh, const std::uint32_t a_vertexIndex) const;
        TypeAlias::Math::Vector4 FetchWorldVertexTangent (const ufbx_node* a_fbxNode, const ufbx_mesh*    a_fbxMesh, const std::uint32_t a_vertexIndex) const;

        std::wstring FetchMaterialTextureFilePath(const ufbx_material_map& a_materialMap) const;

        TypeAlias::Math::Color FetchBaseColorFactor(const ufbx_material_map& a_materialMap) const;

        float FetchMaterialFactor(const ufbx_material_map& a_materialMap, const float a_defaultValue) const;

    private:

        ufbx_load_opts CreateFBXLoadOptions() const;

        // .materialファイル名に使う名前を作る(モデル名_Material名、ファイル名に使えない文字は置き換える)
        std::wstring CreateMaterialName(const ufbx_material* a_fbxMaterial, const std::filesystem::path& a_modelFilePath) const;

        static constexpr std::wstring_view k_invalidFileNameCharacterList = L"<>:\"/\\|?*";

        static constexpr std::wstring_view k_materialNameSeparator = L"_";
        static constexpr std::wstring_view k_defaultMaterialName   = L"Material";

        static constexpr wchar_t k_invalidFileNameReplacementCharacter = L'_';

#if defined(_DEBUG)
        static constexpr std::size_t k_errorTextBufferSize = 1024ULL;
#endif

        static constexpr ufbx_real k_modelFBXTargetUnitMeters = 1.0;

        static constexpr float k_uvCoordinateMax = 1.0F;

        static constexpr float k_defaultTangentX = 1.0F;
        static constexpr float k_defaultTangentY = 0.0F;
        static constexpr float k_defaultTangentZ = 0.0F;
        static constexpr float k_defaultTangentW = 1.0F;
    };
}