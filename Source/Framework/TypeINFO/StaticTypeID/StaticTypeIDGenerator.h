#pragma once

namespace FWK
{
	// 型ごとの静的ID取得のみを公開し、実際の採番処理は隠蔽するためのクラス
	class StaticTypeIDGenerator final
	{
	private:

		// コンストラクタを削除することでこのクラスの生成を許さない
		 StaticTypeIDGenerator() = delete;
		~StaticTypeIDGenerator() = delete;

	public:

		// テンプレートを使った関数の特殊化を用いて動的に型ごとに一意なIDを生成
		template <typename Type>
		static TypeAlias::StaticTypeID GetVALTypeID()
		{
			static auto l_staticTypeID = GenerateTypeID();

			return l_staticTypeID;
		}

	private:

		// 実際に静的IDを生成する関数
		static TypeAlias::StaticTypeID GenerateTypeID()
		{
			static auto l_staticTypeID = k_initialStaticTypeID;

			// 戻り値として扱うStaticTypeIDを格納
			const auto l_generatedID = l_staticTypeID;

			// 次のStaticTypeID出力備えてインクリメント
			++l_staticTypeID;

			// 登録可能上限を超えていたらアサート
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_staticTypeID == Constant::k_invalidStaticTypeID, "静的TypeIDの登録可能上限に到達しており、静的IDの登録に失敗しました。。", Constant::k_invalidStaticTypeID);

			return l_generatedID;
		}

		static constexpr TypeAlias::StaticTypeID k_initialStaticTypeID = 0U;
	};
}