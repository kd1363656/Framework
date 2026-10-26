#pragma once

namespace FWK::Physics
{
    class PhysicsCharacterVirtualBase
    {
    private:

        friend class Scene;

        class PhysicsCharacterVirtualInstance final : public JPH::CharacterVirtual
        {
        public:

            JPH_OVERRIDE_NEW_DELETE

            using JPH::CharacterVirtual::CharacterVirtual;

            ~PhysicsCharacterVirtualInstance() override = default;

            void SetSupportingVolume(const JPH::Plane& a_set) { mSupportingVolume = a_set; }
        };

    public:

                 PhysicsCharacterVirtualBase();
        virtual ~PhysicsCharacterVirtualBase();

        PhysicsCharacterVirtualBase(const PhysicsCharacterVirtualBase&)  = delete;
        PhysicsCharacterVirtualBase(      PhysicsCharacterVirtualBase&&) = delete;

        PhysicsCharacterVirtualBase& operator=(const PhysicsCharacterVirtualBase&)  = delete;
        PhysicsCharacterVirtualBase& operator=(      PhysicsCharacterVirtualBase&&) = delete;

        bool CreateCharacterVirtual(const float                        a_capsuleHalfHeightOfCylinder,
                                    const float                        a_capsuleRadius,
                                    const float                        a_maxSlopeAngleRadians,
                                    const bool                         a_isEnhancedInternalEdgeRemovalDisabled,
                                    const TypeAlias::Math::Quaternion& a_worldRotation,
                                    const TypeAlias::Math::Vector3&    a_worldPosition);

        void Update(const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, const float a_deltaTime);

        void DrawDebug(const JPH::ColorArg a_color) const;

        void ReleaseCharacterVirtual();

        bool ApplyWorldTransform(const TypeAlias::Math::Vector3& a_worldPosition, const TypeAlias::Math::Quaternion& a_worldRotation);
        bool ApplyShapeChange   (const float                     a_capsuleHalfHeightOfCylinder, const float a_capsuleRadius);

        TypeAlias::Math::Vector3 FetchVALWorldPosition () const;
        TypeAlias::Math::Vector3 FetchVALLinearVelocity() const;

        bool FetchVALIsOnGround() const;

    protected:

        virtual JPH::Vec3 CalculateLinearVelocity(const JPH::Vec3&                                 a_physicsGravity,
                                                  const Struct::PhysicsCharacterVirtualUpdateData& a_updateData,
                                                  const float                                      a_deltaTime,
                                                        JPH::CharacterVirtual&                     a_characterVirtual) = 0;

        virtual JPH::Vec3 FetchVALUpdateGravity(const JPH::Vec3& a_physicsGravity) const = 0;

        virtual void ApplyExtendedUpdateSettings(const JPH::CharacterVirtual& a_characterVirtual, JPH::CharacterVirtual::ExtendedUpdateSettings& a_extendedUpdateSettings) const = 0;

    private:

        JPH::RefConst<JPH::Shape> CreateShape(const float a_capsuleHalfHeightOfCylinder, const float a_capsuleRadius) const;

        static constexpr float k_minCharacterVirtualDeltaTime                   = 0.0F;
        static constexpr float k_minCharacterVirtualCapsuleHalfHeightOfCylinder = 0.0F;
        static constexpr float k_minCharacterVirtualCapsuleRadius               = 0.0F;
        static constexpr float k_minCharacterVirtualMaxSlopeAngleRadians        = 0.0F;
        static constexpr float k_maxCharacterVirtualMaxSlopeAngleRadians        = DirectX::XM_PIDIV2;
        static constexpr float k_minCharacterVirtualJumpSpeed                   = 0.0F;

        static constexpr float k_characterVirtualShapeChangePenetrationSlopScale = 1.5F;

        static constexpr JPH::uint64 k_defaultUserData = 0ULL;

        JPH::Ref<PhysicsCharacterVirtualInstance> m_characterVirtual;

        JPH::CharacterVirtual::ExtendedUpdateSettings m_extendedUpdateSettings;

        FWK_DEFINE_TYPE_INFO_ROOT(PhysicsCharacterVirtualBase)
    };
}