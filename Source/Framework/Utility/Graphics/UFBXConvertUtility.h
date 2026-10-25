#pragma once

namespace FWK::Utility
{
    inline TypeAlias::Math::Vector3 ConvertUFBXVector3ToVector3(const ufbx_vec3& a_fbxVector)
    {
        // ufbx_vec3はdouble系の値を持つため、自作フレームワークのVector3で使うfloatへ変換する
        return TypeAlias::Math::Vector3(static_cast<float>(a_fbxVector.x), static_cast<float>(a_fbxVector.y), static_cast<float>(a_fbxVector.z));
    }
    inline TypeAlias::Math::Vector2 ConvertUFBXVector2ToVector2(const ufbx_vec2& a_fbxVector)
    {
        // ufbx_vec2はdouble系の値を持つため、自作フレームワークのVector2で使うfloatへ変換する
        return TypeAlias::Math::Vector2(static_cast<float>(a_fbxVector.x), static_cast<float>(a_fbxVector.y));
    }

    inline std::wstring ConvertUFBXStringToWString(const ufbx_string& a_fbxString)
    {
        if (!a_fbxString.data ||
            a_fbxString.length == Constant::k_emptyStringLength)
        {
            return {};
        }

        std::string l_string = {};

        l_string.assign(a_fbxString.data, a_fbxString.length);

        // ufbxの文字列はUTF-8で入っているため、UTF-8としてUTF-16(std::wstring)へ変換する
        // ※ std::filesystem::path(std::string)はWindowsの既定の文字コード(Shift-JISなど)として読むため、
        //   日本語の名前(ブレンドシェイプ名など)で文字化けや例外が起きていた
        return Utility::StringToWString(l_string);
    }

    inline TypeAlias::Math::Matrix ConvertUFBXMatrixToMatrix(const ufbx_matrix& a_fbxMatrix)
    {
        // UFBXの列ベクトル行列をSimpleMathの行ベクトル行列へ転置する
        return TypeAlias::Math::Matrix
        {
            static_cast<float>(a_fbxMatrix.m00),
            static_cast<float>(a_fbxMatrix.m10),
            static_cast<float>(a_fbxMatrix.m20),
            Constant::k_affineMatrixAxisW,

            static_cast<float>(a_fbxMatrix.m01),
            static_cast<float>(a_fbxMatrix.m11),
            static_cast<float>(a_fbxMatrix.m21),
            Constant::k_affineMatrixAxisW,

            static_cast<float>(a_fbxMatrix.m02),
            static_cast<float>(a_fbxMatrix.m12),
            static_cast<float>(a_fbxMatrix.m22),
            Constant::k_affineMatrixAxisW,

            static_cast<float>(a_fbxMatrix.m03),
            static_cast<float>(a_fbxMatrix.m13),
            static_cast<float>(a_fbxMatrix.m23),
            Constant::k_affineMatrixTranslationW
        };
    }
}