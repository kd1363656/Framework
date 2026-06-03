#pragma once

namespace FWK::Utility
{
	// より安全にシングルトンを扱うためのコピー・ムーブを禁止したCRTPベースのシングルトン基底クラス
	template <typename Type>
	class SingletonBase
	{
	public:

		static Type& GetInstance()
		{
			static Type l_instance;

			return l_instance;
		}

	protected:

				 SingletonBase() = default;
		virtual ~SingletonBase() = default;

	private:

		// コピー演算子削除
		SingletonBase(const SingletonBase<Type>&) = delete;

		// コピー代入演算子削除
		SingletonBase& operator=(const SingletonBase<Type>&) = delete;

		// 移動演算子削除
		SingletonBase(SingletonBase<Type>&&) = delete;

		// 移動代入演算子削除
		SingletonBase<Type>& operator=(SingletonBase<Type>&&) = delete;
	};
}