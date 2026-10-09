#pragma once

namespace FWK::Utility
{
    class IMGUIDragDropPayloadStorage final : public SingletonBase<IMGUIDragDropPayloadStorage>
    {
    private:

        friend class SingletonBase<IMGUIDragDropPayloadStorage>;

         IMGUIDragDropPayloadStorage()         = default;
        ~IMGUIDragDropPayloadStorage()override = default;

    public:

        void BeginFrame()
        {
            // DearImGui側にDragDropPayloadが存在しているなら
            // Dragはまだ継続中なのでFramework側Payloadも保持する
            if (ImGui::GetDragDropPayload()) { return; }

            // DropせずMouseを話した場合など、
            // Dragが終了していればPayloadを破棄する
            Clear();
        }

        template <typename BuilderType>
            requires std::invocable<BuilderType>
        bool DragDropSource(const std::string_view& a_label, BuilderType&& a_payloadBuilder)
        {
            // a_payloadBuilderはPayloadを生成して返す関数オブジェクト
            // BeginDragDropSourceが成功したDrag開始Frameに一度だけ実行されるため
            // 選択Listの構築などコストの掛かるPayload生成を遅延できる
            // 非Drag中は実行されないので、各ノード・各フレームでの無駄な生成が消える
            using PayloadType = std::remove_cvref_t<std::invoke_result_t<BuilderType>>;

            constexpr auto l_kind = TypeTrait::PTRType<PayloadType>::k_kind;

            // 生ポインタを許可しない
            static_assert(l_kind != Enum::PTRKind::Raw, "RawPointerはDragDropPayloadに使用できません");

            // std::anyはPayloadを所有するため、
            // Copy不可能なunique_ptrは使用できない。
            static_assert(l_kind != Enum::PTRKind::Unique, "std::unique_ptrはDragDropPayloadに使用できません");

            // ImGuiDragDropFlags_SourceNoHoldToOpenOthersを指定して
            // ドラッグ中にTreeNodeやCollapsingHeaderをホバーした際の
            // 自動展開挙動を無効化する
            // これによりフォルダペインでドラッグ中に
            // ホバーしたノードが勝手に開く現象を防ぐ
            if (!ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) { return false; }

            // Drag開始FrameだけBuilderを実行してPayload本体を生成する
            // Drag中はm_payloadを保持し続けるため
            // filesystem::pathのListやshared_ptrなどを
            // 毎フレーム生成することはない
            if (!m_payload.has_value())
            {
                // ラムダ関数を呼び出してPayloadを生成する
                PayloadType l_payload = std::invoke(a_payloadBuilder);

                // 生成したPayloadが無効(空のList・無効なPointer等)なら
                // Payloadを登録せずDragを成立させない
                // m_payloadは空のまま残るため、Dragが継続している間は
                // 毎フレームBuilderの再評価が行われる
                if (!IsValidDragPayload(l_payload))
                {
                    ImGui::EndDragDropSource();

                    return false;
                }

                m_payload = std::move(l_payload);

                ++m_payloadID;
            }

            // DearImGuiには実際のC++オブジェクトではなく
            // Framework側Payloadを識別するIDだけを渡す
            const auto& l_payloadID = m_payloadID;

            const bool l_isPayloadSet = ImGui::SetDragDropPayload(a_label.data(),
                                                                  &l_payloadID,
                                                                  sizeof(l_payloadID),
                                                                  ImGuiCond_Once);

            ImGui::TextUnformatted  (a_label.data());
            ImGui::EndDragDropSource();

            return l_isPayloadSet;
        }

        template <typename Type>
        bool DragDropTarget(const std::string_view& a_label, Type& a_outPayload)
        {
            using PayloadType = std::remove_cvref_t<Type>;

            constexpr auto l_kind = TypeTrait::PTRType<PayloadType>::k_kind;

            static_assert(l_kind != Enum::PTRKind::Raw,           "RawPointerはDragDropPayloadに使用できません");
            static_assert(l_kind != Enum::PTRKind::Unique,        "std::unique_ptrはDragDropPayloadに使用できません");
            static_assert(std::is_copy_assignable_v<PayloadType>, "DragDropPayloadはCopyAssign可能である必要があります");

            if (!ImGui::BeginDragDropTarget()) { return false; }

            return AcceptPayload(a_label, a_outPayload);
        }

        // 直前アイテムではなく指定した矩形をドロップ先にする版
        // ペイン空白部分などアイテムが存在しない領域のドロップに使用する
        template <typename Type>
        bool DragDropTargetCustom(const ImRect&           a_targetRECT,
                                  const std::string_view& a_label,
                                  const ImGuiID           a_targetID,
                                        Type&             a_outPayload)
        {
            using PayloadType = std::remove_cvref_t<Type>;

            constexpr auto l_kind = TypeTrait::PTRType<PayloadType>::k_kind;

            static_assert(l_kind != Enum::PTRKind::Raw,           "RawPointerはDragDropPayloadに使用できません");
            static_assert(l_kind != Enum::PTRKind::Unique,        "std::unique_ptrはDragDropPayloadに使用できません");
            static_assert(std::is_copy_assignable_v<PayloadType>, "DragDropPayloadはCopyAssign可能である必要があります");

            if (!ImGui::BeginDragDropTargetCustom(a_targetRECT, a_targetID)) { return false; }

            return AcceptPayload(a_label, a_outPayload);
        }

    private:

        // BeginDragDropTarget/BeginDragDropTargetCustom成功後に呼ぶ共通受信処理
        // 内部でEndDragDropTargetを必ず呼ぶ
        template <typename Type>
        bool AcceptPayload(const std::string_view& a_label, Type& a_outPayload)
        {
            using PayloadType = std::remove_cvref_t<Type>;

            constexpr auto l_kind = TypeTrait::PTRType<PayloadType>::k_kind;

            const auto* l_imGuiPayload = ImGui::AcceptDragDropPayload(a_label.data());

            // Payloadを受信していない
            if (!l_imGuiPayload ||
                !l_imGuiPayload->IsDelivery() ||
                !l_imGuiPayload->Data ||
                l_imGuiPayload->DataSize != sizeof(std::uint64_t))
            {
                ImGui::EndDragDropTarget();

                return false;
            }

            auto l_payloadID = k_initialPayloadID;

            // ImGui側Payloadはuint64_tだけなのでByteCopyして問題ない
            std::memcpy(&l_payloadID, l_imGuiPayload->Data, sizeof(l_payloadID));

            // Framework側Storageと異なるDrag操作なら受け取らない
            if (l_payloadID != m_payloadID)
            {
                ImGui::EndDragDropTarget();

                return false;
            }

            const auto* l_payload = std::any_cast<PayloadType>(&m_payload);

            if (!l_payload)
            {
                ImGui::EndDragDropTarget();

                return false;
            }

            // Drag開始後にweak_ptrの参照先が破棄された場合はDropを成立させない
            if constexpr (l_kind == Enum::PTRKind::Weak)
            {
                if (l_payload->expired())
                {
                    ImGui::EndDragDropTarget();

                    Clear();

                    return false;
                }
            }

            a_outPayload = *l_payload;

            ImGui::EndDragDropTarget();

            Clear();

            return true;
        }

        template <typename Type>
        static bool IsValidDragPayload(const Type& a_payload)
        {
            // 生成されたPayloadがDrag可能な有効状態かを判定する
            constexpr auto l_kind = TypeTrait::PTRType<Type>::k_kind;

            // shared_ptrが空の場合はDragを開始しない
            if constexpr (l_kind == Enum::PTRKind::Shared)
            {
                if (!a_payload) { return false; }
            }

            // weak_ptrの参照先が既にない場合もDragしない
            if constexpr (l_kind == Enum::PTRKind::Weak)
            {
                if (a_payload.expired()) { return false; }
            }

            // Listや文字列などemptyを持つ型で
            // 要素が一つも無い場合もDragしない
            if constexpr (requires { { a_payload.empty() } -> std::convertible_to<bool>; })
            {
                if (a_payload.empty()) { return false; }
            }

            return true;
        }

        void Clear();

        static constexpr std::uint64_t k_initialPayloadID = 0ULL;

        std::any m_payload = {};

        std::uint64_t m_payloadID = k_initialPayloadID;
    };
}