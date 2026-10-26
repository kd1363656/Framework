#include "ModelOutlineNormalBuilder.h"

std::size_t FWK::Graphics::ModelOutlineNormalBuilder::PositionKeyHash::operator()(const PositionKey& a_key) const
{
    // 3つの整数のハッシュ値を、ビットをずらしながら混ぜ合わせる(boost::hash_combineと同じ考え方)
    // 単純に足したりXORしたりすると、(1, 2, 3)と(3, 2, 1)のように並びが違うだけの値が同じハッシュ値になりやすいため
    const std::hash<std::int64_t> l_hasher = {};

    std::size_t l_hash = l_hasher(a_key.m_x);

    l_hash ^= l_hasher(a_key.m_y) + k_hashMixConstant + (l_hash << k_hashMixLeftShift) + (l_hash >> k_hashMixRightShift);
    l_hash ^= l_hasher(a_key.m_z) + k_hashMixConstant + (l_hash << k_hashMixLeftShift) + (l_hash >> k_hashMixRightShift);

    return l_hash;
}

FWK::Graphics::ModelOutlineNormalBuilder::PositionKey FWK::Graphics::ModelOutlineNormalBuilder::CreatePositionKey(const TypeAlias::Math::Vector3& a_position) const
{
    // 位置を許容距離で割って四捨五入し、整数にする(許容距離より近い頂点は、ほぼ同じ整数になる)
    PositionKey l_positionKey = {};

    l_positionKey.m_x = std::llround(a_position.x / k_positionWeldTolerance);
    l_positionKey.m_y = std::llround(a_position.y / k_positionWeldTolerance);
    l_positionKey.m_z = std::llround(a_position.z / k_positionWeldTolerance);

    return l_positionKey;
}

bool FWK::Graphics::ModelOutlineNormalBuilder::TryCalculateFaceNormal(const std::array<TypeAlias::Math::Vector3, Constant::k_triangleVertexCount>& a_positionList,
                                                                      const std::array<TypeAlias::Math::Vector3, Constant::k_triangleVertexCount>& a_normalList,
                                                                            TypeAlias::Math::Vector3&                                               a_faceNormal) const
{
    const auto& l_edgeA = a_positionList[k_secondCornerOffset] - a_positionList[k_firstCornerOffset];
    const auto& l_edgeB = a_positionList[k_thirdCornerOffset]  - a_positionList[k_firstCornerOffset];

    // 2辺の外積は、面に垂直な向きになる(長さは三角形の面積の2倍)
    auto l_faceNormal = l_edgeA.Cross(l_edgeB);

    if (l_faceNormal.LengthSquared() <= k_minDirectionLengthSquared) { return false; }

    l_faceNormal.Normalize();

    // 三角形の頂点の並び順(時計回り / 反時計回り)によって外積の向きは反対になるため、
    // 頂点の法線の平均と同じ側を向くようにそろえる
    const auto& l_averageNormal = a_normalList[k_firstCornerOffset] + a_normalList[k_secondCornerOffset] + a_normalList[k_thirdCornerOffset];

    if (l_faceNormal.Dot(l_averageNormal) < k_zeroValue)
    {
        l_faceNormal = -l_faceNormal;
    }

    a_faceNormal = l_faceNormal;

    return true;
}

float FWK::Graphics::ModelOutlineNormalBuilder::CalculateCornerAngle(const std::array<TypeAlias::Math::Vector3, Constant::k_triangleVertexCount>& a_positionList, const std::size_t a_corner) const
{
    // a_corner番目の頂点から、残り2つの頂点へ向かう辺
    const auto& l_position     = a_positionList[a_corner];
    const auto& l_nextPosition = a_positionList[(a_corner + k_secondCornerOffset) % Constant::k_triangleVertexCount];
    const auto& l_prevPosition = a_positionList[(a_corner + k_thirdCornerOffset)  % Constant::k_triangleVertexCount];

    auto l_edgeA = l_nextPosition - l_position;
    auto l_edgeB = l_prevPosition - l_position;

    if (l_edgeA.LengthSquared() <= k_minDirectionLengthSquared ||
        l_edgeB.LengthSquared() <= k_minDirectionLengthSquared)
    {
        return k_zeroValue;
    }

    l_edgeA.Normalize();
    l_edgeB.Normalize();

    // 長さ1の2辺のdotは、間の角のcosになる(誤差で-1 ~ 1を少しはみ出すとacosが壊れるため、範囲内に収める)
    return std::acos(std::clamp(l_edgeA.Dot(l_edgeB), k_minCosine, k_maxCosine));
}

FWK::TypeAlias::Math::Vector3 FWK::Graphics::ModelOutlineNormalBuilder::CalculateSmoothNormal(const std::vector<TypeAlias::Math::Vector3>& a_faceNormalList, const TypeAlias::Math::Vector3& a_vertexNormal) const
{
    TypeAlias::Math::Vector3 l_vertexNormal = a_vertexNormal;

    if (l_vertexNormal.LengthSquared() <= k_minDirectionLengthSquared) { return k_vertexNormalTangentDirection; }

    l_vertexNormal.Normalize();

    TypeAlias::Math::Vector3 l_smoothNormal = {};

    for (const auto& l_faceNormal : a_faceNormalList)
    {
        // 面の向き × 角度なので、長さで割れば面の向き(長さ1)とのdotになる
        const float l_faceNormalLength = l_faceNormal.Length();

        if (l_faceNormalLength <= k_zeroValue) { continue; }

        // 頂点の法線と大きく逆を向いた面(薄い布の裏側など)は足さない
        if (l_faceNormal.Dot(l_vertexNormal) / l_faceNormalLength < k_minMergeNormalDot) { continue; }

        l_smoothNormal += l_faceNormal;
    }

    // 足した向きが打ち消し合ってほぼ0になった場合は、頂点の法線をそのまま使う
    if (l_smoothNormal.LengthSquared() <= k_minDirectionLengthSquared) { return l_vertexNormal; }

    l_smoothNormal.Normalize();

    return l_smoothNormal;
}

FWK::TypeAlias::Math::Vector3 FWK::Graphics::ModelOutlineNormalBuilder::TransformToTangentSpace(const TypeAlias::Math::Vector3& a_direction,
                                                                                                const TypeAlias::Math::Vector3& a_normal,
                                                                                                const TypeAlias::Math::Vector4& a_tangent) const
{
    TypeAlias::Math::Vector3 l_normal = a_normal;

    if (l_normal.LengthSquared() <= k_minDirectionLengthSquared) { return k_vertexNormalTangentDirection; }

    l_normal.Normalize();

    // 接線を法線に対して直交化する(描画時のFetchWorldNormalと同じ作り方にして、組み立て直したときに同じ向きへ戻るようにする)
    const TypeAlias::Math::Vector3 l_rawTangent = { a_tangent.x, a_tangent.y, a_tangent.z };

    auto l_tangent = l_rawTangent - l_normal * l_normal.Dot(l_rawTangent);

    TypeAlias::Math::Vector3 l_bitangent = {};

    if (l_tangent.LengthSquared() <= k_minDirectionLengthSquared)
    {
        // 接線が無い(又は法線と同じ向きの)頂点は、法線だけから接線と従法線を作る
        // (描画時も同じ条件で同じ作り方をするため、同じ向きへ組み立て直せる)
        BuildOrthonormalBasis(l_normal, l_tangent, l_bitangent);
    }
    else
    {
        l_tangent.Normalize();

        // 従法線 = 法線 × 接線(tangent.wはミラーUVなどで向きが反転するときの補正)
        l_bitangent = l_normal.Cross(l_tangent) * a_tangent.w;
    }

    // TBNの各軸とのdotが、Tangent空間での成分になる
    return { a_direction.Dot(l_tangent), a_direction.Dot(l_bitangent), a_direction.Dot(l_normal) };
}

void FWK::Graphics::ModelOutlineNormalBuilder::BuildOrthonormalBasis(const TypeAlias::Math::Vector3& a_normal, TypeAlias::Math::Vector3& a_tangent, TypeAlias::Math::Vector3& a_bitangent) const
{
    // 法線だけから、法線に直交する2つの軸を作る(Duffらの方法、2017年)
    // 法線のzの符号で式を切り替えることで、どの向きの法線でも0除算や精度の悪化が起きない
    const float l_sign = std::copysign(k_oneValue, a_normal.z);
    const float l_a    = -k_oneValue / (l_sign + a_normal.z);
    const float l_b    = a_normal.x * a_normal.y * l_a;

    a_tangent   = { k_oneValue + l_sign * a_normal.x * a_normal.x * l_a, l_sign * l_b, -l_sign * a_normal.x };
    a_bitangent = { l_b, l_sign + a_normal.y * a_normal.y * l_a, -a_normal.y };
}

std::uint32_t FWK::Graphics::ModelOutlineNormalBuilder::EncodeOctahedralNormal(const TypeAlias::Math::Vector3& a_direction) const
{
    // Octahedral符号化
    // 長さ1の向き(球の表面の点)を、|x| + |y| + |z| = 1の正八面体へ写し、
    // 上半分(z >= 0)はそのままxyを、下半分は四隅へ折り返したxyを使う
    // こうすると、全ての向きを-1 ~ 1の2つの値だけで表せる
    const float l_absoluteSum = std::abs(a_direction.x) + std::abs(a_direction.y) + std::abs(a_direction.z);

    if (l_absoluteSum <= k_octahedralZero) { return EncodeOctahedralNormal(k_vertexNormalTangentDirection); }

    float l_x = a_direction.x / l_absoluteSum;
    float l_y = a_direction.y / l_absoluteSum;

    if (a_direction.z < k_octahedralZero)
    {
        const float l_foldedX = (k_octahedralOne - std::abs(l_y)) * (l_x >= k_octahedralZero ? k_octahedralOne : -k_octahedralOne);
        const float l_foldedY = (k_octahedralOne - std::abs(l_x)) * (l_y >= k_octahedralZero ? k_octahedralOne : -k_octahedralOne);

        l_x = l_foldedX;
        l_y = l_foldedY;
    }

    // -1 ~ 1を0 ~ 65535の整数にして、xを下位16bit、yを上位16bitに詰める
    const auto l_encodedX = static_cast<std::uint32_t>((l_x * k_octahedralRangeScale + k_octahedralRangeBias) * k_octahedralMaxValue + k_octahedralRound);
    const auto l_encodedY = static_cast<std::uint32_t>((l_y * k_octahedralRangeScale + k_octahedralRangeBias) * k_octahedralMaxValue + k_octahedralRound);

    return l_encodedX | (l_encodedY << k_octahedralYShift);
}