#include "PhysicsManager.h"

FWK::Physics::PhysicsManager::PhysicsManager() :
	m_factory(nullptr),

	m_tempAllocator(nullptr),

	m_jobSystem(nullptr),

	m_physicsLayerSetting(nullptr),

	m_physicsSystem(),

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
	FWK_ASSERT_RETURN_IF_FAILED(m_isInitialized, "既に初期化されているのにもう一度初期化しようとしました。");

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

	m_isInitialized = true;
}

void FWK::Physics::PhysicsManager::Update(const float a_deltaTime)
{
	FWK_ASSERT_RETURN_IF_FAILED(!m_isInitialized, "初期化されていないのに更新処理を実行しようとしました。");
	FWK_ASSERT_RETURN_IF_FAILED(!m_tempAllocator, "TempAllocatorがnullptrのため、更新処理に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!m_jobSystem,     "JobSystemがnullptrのため、更新処理に失敗しました。");

	// Joltの物理ワールドを1フレーム分進める。
	// JPH::PhysicsSystem(前フレームからの経過時間、
	//					  1回のUpdate内で物理計算を何分割するか、
	//					  JoltがこのUpdate中に使う一時作業メモリ、
	//					  Joltが物理計算を並列実行するためのJobSystem);
	m_physicsSystem.Update(a_deltaTime, 
						   k_collisionStepCount,
						   m_tempAllocator.get(),
						   m_jobSystem.get());
}

void FWK::Physics::PhysicsManager::OptimizeBroadPhase()
{
	if (!m_isInitialized) { return; }

	// JoltのBroadPhase空間分割を最適化する。
	// 毎フレーム呼ぶものではなく、
	// ステージ読み込み後など、大量のStaticObjectを追加した後に呼ぶ
	m_physicsSystem.OptimizeBroadPhase();
}

bool FWK::Physics::PhysicsManager::SetupJoltCore()
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(JPH::Factory::sInstance, "JPH::Factory::sInstanceが既に存在しており、コア設定に失敗しました。", false);

	// Joltのメモリアロケータを登録する。
	// Joltの機能を使う前に必要
	JPH::RegisterDefaultAllocator();

#if defined(_DEBUG)
	SetupJoltDebugCallback();
#endif

	m_factory = std::make_unique<JPH::Factory>();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_factory, "JPH::Factoryの作成に失敗しました。", false);

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

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_tempAllocator, "JPH::TempAllocatorImplの作成に失敗しました。", false);

	// CPUの論理スレッド数を取得する
	// JoltのJobSystemThreadPoolで、物理計算用のワーカースレッド数を決めるために使う
	const auto l_hardwareThreadCount = std::thread::hardware_concurrency();

	// メインスレッド数を一つ残して残りをJoltのワーカースレッドとして扱う
	const int l_workerThreadCount = l_hardwareThreadCount <= k_minHardwareThreadCount ? k_fallbackWorkerThreadCount : static_cast<int>(l_hardwareThreadCount - k_mainThreadCount);

	// JoltのJobSystemを作成する(Joltの物理計算を複数スレッドで実行するための仕組み)
	m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(k_maxPhysicsJobCount, k_maxPhysicsBarrierCount, l_workerThreadCount);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_jobSystem, "JPH::JobSystemThreadPoolの作成に失敗しました。", false);

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

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_objectVsBroadPhaseLayerFilter, "ObjectVSBroadPhaseLayerFilterがnullptrです。", false);

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
	// これを設定しないと、Jolt内部のDummyTrace()がよばれ、IssueReporting.cpp内のJPH_ASSERT(false)で止まる
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

void FWK::Physics::PhysicsManager::Release()
{
	// Joltの型登録を解除する
	// RegisterTypes()を読んだ場合だけUnregisterTypes()を呼ぶ
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
	m_jobSystem           = nullptr;
	m_physicsLayerSetting = nullptr;

	m_isInitialized = false;
}