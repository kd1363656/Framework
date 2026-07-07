#pragma once

namespace FWK
{
	class MatrixStrategyBase;
}

namespace FWK
{
	class TransformComponent final : public ComponentBase
	{
	public:

		 TransformComponent();
		~TransformComponent() override;

		void ConfrimMatrix();

		TypeAlias::Math::Matrix CalcScaleMatrix      () const { return TypeAlias::Math::Matrix::CreateScale         (m_transform.m_scale);    }
		TypeAlias::Math::Matrix CalcRotationMatrix   () const { return TypeAlias::Math::Matrix::CreateFromQuaternion(m_transform.m_rotation); }
		TypeAlias::Math::Matrix CalcTranslationMatrix() const { return TypeAlias::Math::Matrix::CreateTranslation   (m_transform.m_position); }
	
		void ApplyParentTransformComponent(const std::weak_ptr<TransformComponent>& a_parentTransformComponent);

		const auto& GetREFParentTransformComponent() const { return m_parentTransformComponent; }

		void SetMatrix(TypeAlias::Math::Matrix&& a_set) { m_transform.m_matrix = std::move(a_set); }

	private:

		void ConfrimMatrixStrategy();

		std::weak_ptr<TransformComponent> m_parentTransformComponent;

		std::unique_ptr<MatrixStrategyBase> m_matrixStrategy;

		Struct::Transform m_transform;

		FWK_DEFINE_TYPE_INFO(TransformComponent, ComponentBase)
	};
}