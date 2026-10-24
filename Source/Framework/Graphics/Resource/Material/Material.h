#pragma once

namespace FWK::Graphics
{
    // MaterialSystemが管理するMaterialを参照するためのクラス
    // コピーすると参照数が増え、破棄すると参照数が減る
    class Material final
    {
    public:

         Material();
         Material(const Material&  a_other);
         Material(      Material&& a_other) noexcept;
        ~Material();

        Material& operator=(const Material&  a_other);
        Material& operator=(      Material&& a_other) noexcept;

        // パスが空の場合はデフォルトマテリアルを参照する
        bool Load(const std::filesystem::path& a_filePath);

        bool IsValid() const;

        const auto& GetREFMaterial() const { return m_material; }

        auto GetVALStorageID() const { return m_storageID; }

    private:

        void AddReferenceCount() const;

        void SubtractReferenceCount();

        std::weak_ptr<MaterialBase> m_material;

        TypeAlias::StorageID m_storageID;
    };
}