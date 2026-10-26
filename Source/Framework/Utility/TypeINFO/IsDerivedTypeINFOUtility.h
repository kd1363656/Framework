#pragma once

namespace FWK::Utility
{
    // a_typeINFOの型がa_baseTypeINFOの型そのもの、またはその派生クラスかを調べる
    // TypeINFOは自分の1つ上の基底クラスの型情報(k_baseINFO)を持っているので、
    // 一番上の基底クラス(k_baseINFOがnullptr)まで順にたどり、途中で同じ型IDが見つかれば派生クラスと判定する
    // (dynamic_castを使わずに「この型は〇〇の一種か」を判定するための関数)
    inline bool IsDerivedTypeINFO(const TypeINFORegistry::TypeINFO& a_typeINFO, const TypeINFORegistry::TypeINFO& a_baseTypeINFO)
    {
        const auto* l_currentTypeINFO = &a_typeINFO;

        while (l_currentTypeINFO)
        {
            if (l_currentTypeINFO->k_staticTypeID == a_baseTypeINFO.k_staticTypeID) { return true; }

            l_currentTypeINFO = l_currentTypeINFO->k_baseINFO;
        }

        return false;
    }
}
