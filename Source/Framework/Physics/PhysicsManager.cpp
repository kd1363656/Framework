#include "PhysicsManager.h"

FWK::Physics::PhysicsManager::PhysicsManager() :
	m_activeBodyIDIndexMap(),

	m_activeBodyIDList(),

	m_factory(nullptr),

	m_tempAllocator(nullptr),

	m_physicsLayerSetting(nullptr),

	m_debugRenderer(nullptr),

	m_physicsSystem(),

	m_bodyCreator(),

	m_isInitialized(false),

	m_isJoltTypeRegistered(false),

	m_isDisableDebugDraw(false)
{}
FWK::Physics::PhysicsManager::~PhysicsManager()
{
	Release();
}

void FWK::Physics::PhysicsManager::INIT()
{
	FWK_ASSERT_RETURN_IF(m_isInitialized, "既に初期化されているのにもう一度初期化しようとしました。");

	// Joltのメモリアロケータ登録やFactoryの作成などを行う
	if (!SetupJoltCore())
	{
		Release          ();
		FWK_ASSERT_RETURN("Joltのメモリアロケータなどの設定に失敗しており、初期化に失敗しました");
	}

	// Joltの物理ワールド本体を初期化する
	if (!SetupPhysicsSystem())
	{
		Release          ();
		FWK_ASSERT_RETURN("Joltの物理システムの設定に失敗しており、初期化に失敗しました");
	}

	if (!m_debugRenderer)
	{
		m_debugRenderer = std::make_shared<FWK::Physics::PhysicsDebugRenderer>();
	}

	m_debugRenderer->ReserveLineVertexCount();

	m_isInitialized = true;
}

void FWK::Physics::PhysicsManager::OptimizeBroadPhase()
{
	if (!m_isInitialized) { return; }

	// JoltのBroadPhase空間分割を最適化する。
	// 毎フレーム呼ぶものではなく、
	// ステージ読み込み後など、大量のStaticObjectを追加した後に呼ぶ
	m_physicsSystem.OptimizeBroadPhase();
}

void FWK::Physics::PhysicsManager::CollectPhysicsDebugDrawCommands()
{
	if (!m_isInitialized) { return; }
	if (!m_debugRenderer) { return; }

	// まず前フレームのデバック描画情報を消す
	// デバック描画を無効化している場合でも、古い線が残らないように先に消す
	m_debugRenderer->ClearFrame();

	// デバック描画が無効化されているならしない
	if (m_isDisableDebugDraw) { return; }

	JPH::BodyManager::DrawSettings l_drawSettings = {};

	l_drawSettings.mDrawShape		   = true;
	l_drawSettings.mDrawShapeWireframe = true;
	l_drawSettings.mDrawBoundingBox    = false;

	m_physicsSystem.DrawBodies(l_drawSettings, m_debugRenderer.get());

	m_debugRenderer->NextFrame();
}


void FWK::Physics::PhysicsManager::ReleaseBody(Struct::PhysicsBodyHandle& a_bodyHandle)
{
	FWK_ASSERT_RETURN_IF(!m_isInitialized,                  "PhysicsManagerが初期化されていないため、Body解放に失敗しました。");
	FWK_ASSERT_RETURN_IF(!a_bodyHandle.m_isValid,           "PhysicsBodyHandleが無効なため、Body解放に失敗しました。");
	FWK_ASSERT_RETURN_IF(a_bodyHandle.m_bodyID.IsInvalid(), "BodyIDが無効なため、Body解放に失敗しました。");

	auto& l_bodyInterface = m_physicsSystem.GetBodyInterface();

	const auto l_bodyID = a_bodyHandle.m_bodyID;

	// BodyがPhysicsSystemに追加されている場合は、まずPhysicsSystemから外す。
	// RemoveBodyを呼ぶと、以後このBodyは物理更新・衝突判定の対象外になる。
	if (l_bodyInterface.IsAdded(l_bodyID))
	{
		l_bodyInterface.RemoveBody(l_bodyID);
	}

	// Body本体を破棄する
	// RemoveBodyだけでは、Jolt内部にBodyが残るためDestroyBodyも必要
	l_bodyInterface.DestroyBody(l_bodyID);

	UnregisterActiveBodyID(l_bodyID);

	// 呼び出し側が削除済みBodyIDを持ち続けないようにする。
	a_bodyHandle.m_bodyID  = JPH::BodyID();
	a_bodyHandle.m_isValid = false;
}

void FWK::Physics::PhysicsManager::TogglePhysicsDebugDraw()
{
	m_isDisableDebugDraw = m_isDisableDebugDraw ? false : true;
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsManager::CreateStaticSphereBody(const TypeAlias::Math::Vector3& a_worldPosition, const float a_radius)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_isInitialized,       "PhysicsManagerが初期化されていないため、StaticSphereBodyの作成に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(!m_physicsLayerSetting, "PhysicsLayerSettingがnullptrのため、StaticSphereBodyの作成に失敗しました。",     {});

	const auto l_bodyHandle = m_bodyCreator.CreateStaticSphereBody(*m_physicsLayerSetting,
																   a_worldPosition,
																   a_radius,
																   m_physicsSystem);

	RegisterActiveBodyID(l_bodyHandle);

	return l_bodyHandle;
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsManager::CreateStaticBoxBody(const TypeAlias::Math::Vector3& a_worldPosition, const TypeAlias::Math::Vector3& a_halfExtent)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_isInitialized,       "PhysicsManagerが初期化されていないため、StaticBoxBodyの作成に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(!m_physicsLayerSetting, "PhysicsLayerSettingがnullptrのため、StaticBoxBodyの作成に失敗しました。",     {});

	const auto l_bodyHandle = m_bodyCreator.CreateStaticBoxBody(*m_physicsLayerSetting,
															    a_worldPosition,
															    a_halfExtent,
															    m_physicsSystem);

	RegisterActiveBodyID(l_bodyHandle);

	return l_bodyHandle;
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsManager::CreateStaticCapsuleBody(const TypeAlias::Math::Vector3& a_worldPosition, const float a_halfHeightOfCylinder, const float a_radius)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_isInitialized,       "PhysicsManagerが初期化されていないため、StaticCapsuleBodyの作成に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(!m_physicsLayerSetting, "PhysicsLayerSettingがnullptrのため、StaticCapsuleBodyの作成に失敗しました。",     {});

	const auto l_bodyHandle = m_bodyCreator.CreateStaticCapsuleBody(*m_physicsLayerSetting,
												                    a_worldPosition,
												                    a_halfHeightOfCylinder,
												                    a_radius,
												                    m_physicsSystem);

	RegisterActiveBodyID(l_bodyHandle);

	return l_bodyHandle;
}

FWK::TypeAlias::Math::Vector3 FWK::Physics::PhysicsManager::FetchVALBodyWorldPosition(const Struct::PhysicsBodyHandle& a_bodyHandle) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_isInitialized,                  "PhysicsManagerが初期化されていないため、Bodyの座標取得に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(!a_bodyHandle.m_isValid,           "PhysicsBodyHandleが無効なため、Bodyの座標取得に失敗しました。",          {});
	FWK_ASSERT_RETURN_VALUE_IF(a_bodyHandle.m_bodyID.IsInvalid(), "BodyIDが無効なため、Bodyの座標取得に失敗しました。",                     {});

	// JoltのBodyInterfaceを取得する
	// BodyInterfaceは、BodyIDを使ってBodyの位置・回転。速度などを操作する入口。
	auto& l_bodyInterface = m_physicsSystem.GetBodyInterface();

	// Jolt側の現在座標を取得する
	// GetPosition()はBodyのワールド座標を返す
	const auto& l_bodyPosition = l_bodyInterface.GetPosition(a_bodyHandle.m_bodyID);

	return { l_bodyPosition.GetX(), l_bodyPosition.GetY(), l_bodyPosition.GetZ() };
}

bool FWK::Physics::PhysicsManager::SetupJoltCore()
{
	FWK_ASSERT_RETURN_VALUE_IF(JPH::Factory::sInstance, "JPH::Factory::sInstanceが既に存在しており、コア設定に失敗しました。", false);

	// Joltのメモリアロケータを登録する。
	// Joltの機能を使う前に必要
	JPH::RegisterDefaultAllocator();

#if defined(_DEBUG)
	SetupJoltDebugCallback();
#endif

	m_factory = std::make_unique<JPH::Factory>();

	FWK_ASSERT_RETURN_VALUE_IF(!m_factory, "JPH::Factoryの作成に失敗しました。", false);

	// Jolt内部はFactoryをstaticなraw pointerとして要求する。
	// ただし、所有権はm_factoryのstd::unique_ptrが持つ
	JPH::Factory::sInstance = m_factory.get();

	// Joltの標準Shapeや内部型をFactoryへ登録する
	JPH::RegisterTypes();

	// UnregisterTypesをReleaseで呼べるようにしておく
	m_isJoltTypeRegistered = true;

	const auto l_tempAllocatorSize = k_tempAllocatorSizeMB * k_kiloBytePerMB * k_bytePerKB;

	// Joltの一時作業用メモリの作成
	m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(l_tempAllocatorSize);

	FWK_ASSERT_RETURN_VALUE_IF(!m_tempAllocator, "JPH::TempAllocatorImplの作成に失敗しました。", false);

	return true;
}

bool FWK::Physics::PhysicsManager::SetupPhysicsSystem()
{
	// PhysicsLayerSettingはJoltのTable系クラスを作成する
	// その内部でJoltのAllocatorを使用するため、RegisterDefaultAllocator()が終わった後で生成する必要がある
	if (!m_physicsLayerSetting)
	{
		m_physicsLayerSetting = std::make_unique<PhysicsLayerSetting>();
	}

	m_physicsLayerSetting->INIT();

	const auto& l_objectVsBroadPhaseLayerFilter = m_physicsLayerSetting->GetREFObjectVSBroadPhaseLayerFilter();

	FWK_ASSERT_RETURN_VALUE_IF(!l_objectVsBroadPhaseLayerFilter, "ObjectVSBroadPhaseLayerFilterがnullptrです。", false);

	// ここでJoltのPhysicsSystemを初期化する
	// この時点で、JoltのBroadPhase空間分割も使える状態になる
	// JPH::PhysicsSystem::Init(登録できるBodyの最大数、
	//							0を指定するとJolt側の標準的な扱いに任せる、
	//							BroadPhaseが保存できるBodyペア候補の最大数、
	//							接触拘束の最大数、
	//							ObjectLayerをBroadPhaseLayerへ変換する表、
	//							あるObjectLayerがどのBroadPhaseLayerを探索するべきかを決めるFilter,
	//							ObjectLayer同士が衝突して良いか決めるFilter
	m_physicsSystem.Init(k_maxBodyCount,
						 k_bodyMutexCount,
						 k_maxBodyPairCount,
						 k_maxContactConstraintCount,
						 m_physicsLayerSetting->GetREFBroadPhaseLayerInterface(),
						 *l_objectVsBroadPhaseLayerFilter,
						 m_physicsLayerSetting->GetREFObjectLayerPairFilter());

	return true;
}

#if defined(_DEBUG)
void FWK::Physics::PhysicsManager::TraceJoltMessage(const char* a_format, ...)
{
	if (!a_format) { return; }

	char l_buffer[k_joltTraceBufferSize] = {};

	va_list l_argumentList;

	va_start(l_argumentList, a_format);

	vsnprintf_s(l_buffer,
				k_joltTraceBufferSize,
				_TRUNCATE,
				a_format,
				l_argumentList);

	va_end(l_argumentList);

	OutputDebugStringA("[Jolt]");
	OutputDebugStringA(l_buffer);
	OutputDebugStringA("\n");
}

void FWK::Physics::PhysicsManager::SetupJoltDebugCallback() const
{
	// JoltのTrace出力先を用意した関数に差し替える
	// これを設定しないと、Jolt内部のDummyTrace()が呼ばれ、IssueReporting.cpp内のJPH_ASSERT(false)で止まる
	JPH::Trace = TraceJoltMessage;

#ifdef JPH_ENABLE_ASSERTS

	// JoltのAssert内容をVisualStudioの出力ウィンドウに流す
	// VersionのMismatchの原因となっているdefineを確認するために使う
	JPH::AssertFailed = HandleJoltAssertFailed;
#endif
}

#ifdef JPH_ENABLE_ASSERTS
bool FWK::Physics::PhysicsManager::HandleJoltAssertFailed(const char* a_expression, const char* a_message, const char* a_file, const JPH::uint a_line)
{
	char l_buffer[k_joltTraceBufferSize] = {};

	snprintf(l_buffer, 
			 k_joltTraceBufferSize,
			 "(Jolt Assert) File : %s Line : %u Expression : %s Message : %s\n", 
			 a_file ? a_file : "Unknown",
			 a_line,
			 a_expression ? a_expression : "Unknown",
			 a_message ? a_message : "");

	OutputDebugStringA(l_buffer);

	return false;
}
#endif
#endif

void FWK::Physics::PhysicsManager::RegisterActiveBodyID(const Struct::PhysicsBodyHandle& a_bodyHandle)
{
	if (!a_bodyHandle.m_isValid ||
		a_bodyHandle.m_bodyID.IsInvalid()) 
	{
		return;
	}

	const auto l_bodyIDKey = FetchVALBodyIDKey(a_bodyHandle.m_bodyID);

	// すでに登録済みなら何もしない。
	// unordered_mapなので、登録済み確認が高速
	if (m_activeBodyIDIndexMap.contains(l_bodyIDKey)) { return; }

	const auto l_bodyIDIndex = m_activeBodyIDList.size();

	m_activeBodyIDList.emplace_back   (a_bodyHandle.m_bodyID);
	m_activeBodyIDIndexMap.try_emplace(l_bodyIDKey, l_bodyIDIndex);
}

void FWK::Physics::PhysicsManager::UnregisterActiveBodyID(const JPH::BodyID & a_bodyID)
{
	if (a_bodyID.IsInvalid()) { return; }

	const auto  l_removeBodyIDKey = FetchVALBodyIDKey		   (a_bodyID);
	const auto& l_itr             = m_activeBodyIDIndexMap.find(l_removeBodyIDKey);

	// マップ内になければreturn
	if (l_itr == m_activeBodyIDIndexMap.end()) { return; }

	// リストから削除したいインデックスをマップから取得
	const auto l_removeIndex = l_itr->second;
	
	if (const auto l_lastIndex = m_activeBodyIDList.size() - k_lastElementIndexOffset;
		l_removeIndex != l_lastIndex)
	{
		// 最後尾のインデックスが持つBodyIDを取得
		const auto l_lastBodyID    = m_activeBodyIDList[l_lastIndex];
		const auto l_lastBodyIDKey = FetchVALBodyIDKey(l_lastBodyID);

		// 順番に意味がないので、削除したいBodyIDと最後尾のBodyIDを入れ替える
		std::swap(m_activeBodyIDList[l_removeIndex], m_activeBodyIDList[l_lastIndex]);
		
		// リムーブするIndexと最後尾を入れ替えたので
		// 最後尾から削除位置へ移動してきたBodyIDのIndexを更新する
		m_activeBodyIDIndexMap[l_lastBodyIDKey] = l_removeIndex;
	}

	// Swap後、削除対象BodyIDは最後尾にいる。
	// そのためpop_backだけで高速に削除できる
	m_activeBodyIDList.pop_back ();
	m_activeBodyIDIndexMap.erase(l_itr);
}

void FWK::Physics::PhysicsManager::ReleaseAllBodies()
{
	if (!m_isInitialized)
	{
		m_activeBodyIDList.clear    ();
		m_activeBodyIDIndexMap.clear();

		return;
	}

	auto& l_bodyInterface = m_physicsSystem.GetBodyInterface();

	for (const auto& l_bodyID : m_activeBodyIDList)
	{
		if (l_bodyID.IsInvalid()) { continue; }

		if (l_bodyInterface.IsAdded(l_bodyID))
		{
			l_bodyInterface.RemoveBody(l_bodyID);
		}

		l_bodyInterface.DestroyBody(l_bodyID);
	}

	m_activeBodyIDList.clear    ();
	m_activeBodyIDIndexMap.clear();
}

void FWK::Physics::PhysicsManager::Release()
{
	// JoltのDebuigRendererと、それが参照するGeometry / Batchを
	// Joltの型登録解除より前に破棄する
	m_debugRenderer = nullptr;

	// Joltの型登録を解除する前に、作成済みBodyをすべて破棄する
	ReleaseAllBodies();

	// Joltの型登録を解除する
	// RegisterTypes()を呼んだ場合だけUnregisterTypes()を呼ぶ
	if (m_isJoltTypeRegistered)
	{
		JPH::UnregisterTypes();

		m_isJoltTypeRegistered = false;
	}

	// m_factoryの所有権はstd::unique_ptrが持っているため、
	// JPH::Factory::sInstance側が参照するための生ポインタを明示的にnullptr戻す
	if (JPH::Factory::sInstance == m_factory.get())
	{
		JPH::Factory::sInstance = nullptr;
	}

	m_factory		      = nullptr;
	m_tempAllocator       = nullptr;
	m_physicsLayerSetting = nullptr;

	m_isInitialized = false;
}

std::uint32_t FWK::Physics::PhysicsManager::FetchVALBodyIDKey(const JPH::BodyID& a_bodyID) const
{
	// BodyIDをunordered_map用のキーに変換する
	// BodyIDのIndexだけを使うと、Destroy後に同じIndexが再利用されたときに危ない。
	// GetIndexAndSequenceNumber()はIndex + SequenceNumberを含む値を返す
	return a_bodyID.GetIndexAndSequenceNumber();
}