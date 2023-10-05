#include "UnitCubeManager.h"

#include "MeshManager.h"
#include "MyCustomLog.h"
#include "UnitChunk.h"
#include "UnitCube.h"
#include "UnitCubeMapSaveGame.h"
#include "UnitCubePool.h"
#include "UnitCubeType.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AUnitCubeManager::AUnitCubeManager()
	: WorldSeed(0), ChunkLoaderThreadPool(nullptr),
	  CubePool(nullptr)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshManager = nullptr;
	CubeTypeManager = MakeShareable(new FUnitCubeTypeManager());
	ChunkManager = MakeShareable(new FUnitChunkManager());
}

// Called when the game starts or when spawned
void AUnitCubeManager::BeginPlay()
{
	Super::BeginPlay();
	ChunkLoaderThreadPool = FQueuedThreadPool::Allocate();
	ChunkLoaderThreadPool->Create(TaskNum1);
	BuildMeshManager();
	BuildUnitCubePool();
	//BuildMap();
	if (!LoadWorldMap())
	{
		BuildNewWorld();
		// 延迟几秒来执行操作
		FTimerHandle UnusedHandle;
		GetWorldTimerManager().SetTimer(UnusedHandle,
		                                [this]()
		                                {
			                                OnLoadChunkComplete.Broadcast();
		                                }, 0.5f, false);
	}
}

void AUnitCubeManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ChunkLoaderThreadPool->Destroy();
}

// Called every frame
void AUnitCubeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//每一帧先处理加载，后处理卸载
	ExecuteLoadChunkTask(TaskNum1, TaskNum2, TaskNum3);
	//延迟处理卸载
	if (IsNotHasLoadTask())
	{
		NoLoadTaskTimer += DeltaTime;
		if (NoLoadTaskTimer >= TimeThreshold)
		{
			ExecuteUnloadTask(TaskNum4, TaskNum5);
			if (IsNotHasUnloadTask())
			{
				UnloadChunkNotAroundPlayer();
			}
			NoLoadTaskTimer = 0;
		}
	}
	else
	{
		NoLoadTaskTimer = 0;
	}
}

void AUnitCubeManager::ProcessTaskQueue(TQueue<FIntVector>& TaskQueue, const int NumTasks,
                                        const std::function<void(FIntVector)>& TaskFunction)
{
	FIntVector Task;
	for (int i = 0; i < NumTasks; ++i)
	{
		if (TaskQueue.Dequeue(Task))
		{
			TaskFunction(Task);
		}
		else
		{
			break;
		}
	}
}

void AUnitCubeManager::ExecuteLoadChunkTask(const int& N1, const int& N2, const int& N3)
{
	if (!ChunkManager)
	{
		CUSTOM_LOG_WARNING(TEXT("ChunkManager is nullptr"));
		return;
	}
	//加载数据
	ProcessTaskQueue(LoadChunkTask_PrepareData, N1, [this](FIntVector Task)
	{
		//ChunkLoaderRunnable->TriggerExecution(Task);
		ChunkLoaderThreadPool->AddQueuedWork(new FChunkLoaderTask(this,Task));
		//告知线程去做，并注意临界区问题
	});
	if (!LoadChunkTask_PrepareData.IsEmpty())
		return;
	FIntVector Task;
	for (int i = 0; i < N2; ++i)
	{
		//线程会设计到任务队列的出队
		const bool Ret = SafeThread_LoadChunkTaskAllocateResources_Dequeue(Task);
		if (Ret)
		{
			GetChunkLoadState(Task)->MoveToNextResourceState();
			LoadCubeAndCubeTypeWith(Task);
			LoadChunkTask_AllocateMesh.Enqueue(Task);
		}
		else
		{
			break;
		}
	}
	const bool Ret = !SafeThread_LoadChunkTaskAllocateResources_IsEmpty();
	if (Ret)
		return;
	//分配mesh
	ProcessTaskQueue(LoadChunkTask_AllocateMesh, N3, [this](FIntVector Task)
	{
		LoadCubeMeshWith(Task);
	});
}

bool AUnitCubeManager::IsNotHasLoadTask() const
{
	return SafeThread_LoadChunkTaskAllocateResources_IsEmpty() && LoadChunkTask_PrepareData.IsEmpty() &&
		LoadChunkTask_AllocateMesh.IsEmpty();
}

bool AUnitCubeManager::IsNotHasUnloadTask() const
{
	return UnloadChunkTask_AllocateMesh.IsEmpty() && UnloadChunkTask_AllocateResources.IsEmpty();
}

void AUnitCubeManager::ExecuteUnloadTask(const int& N1, const int& N2)
{
	if (!IsNotHasLoadTask())
	{
		return;
	}
	//卸载渲染的队列
	ProcessTaskQueue(UnloadChunkTask_AllocateMesh, N1, [this](FIntVector Task)
	{
		if (GetChunkLoadState(Task)->IsMarkedForUnload())
		{
			UnloadCubeMeshWith(Task);
			UnloadChunkTask_AllocateResources.Enqueue(Task);
		}
	});
	if (!UnloadChunkTask_AllocateMesh.IsEmpty())
	{
		return;
	}
	ProcessTaskQueue(UnloadChunkTask_AllocateResources, N2, [this](FIntVector Task)
	{
		if (GetChunkLoadState(Task)->IsMarkedForUnload())
		{
			UnloadCubeAndCubeTypeWith(Task);
		}
	});
}

void AUnitCubeManager::SafeThread_LoadChunkData(const FIntVector& Task)
{
	LockForLoadChunkData.Lock();
	if (ChunkManager)
	{
		ChunkManager->LoadChunkData(Task);
	}
	LockForLoadChunkData.Unlock();
}

void AUnitCubeManager::SafeThread_LoadChunkTaskAllocateResources_Enqueue(const FIntVector& Task)
{
	LoadChunkTask_AllocateResources.Enqueue(Task);
}

bool AUnitCubeManager::SafeThread_LoadChunkTaskAllocateResources_IsEmpty() const
{
	return LoadChunkTask_AllocateResources.IsEmpty();
}

bool AUnitCubeManager::SafeThread_LoadChunkTaskAllocateResources_Dequeue(FIntVector& Task)
{
	return  LoadChunkTask_AllocateResources.Dequeue(Task);
}

TSharedPtr<FChunkStatus> AUnitCubeManager::GetChunkLoadState(const FIntVector& Task)
{
	const auto State = AllocationChunks.Find(Task);
	if (State)
	{
		return *State;
	}
	else
	{
		CUSTOM_LOG_ERROR(TEXT("can't find ChunkPositon in AllocationChunks:%s"), *Task.ToString());
		return AllocationChunks.Add(Task, MakeShareable(new FChunkStatus));
	}
}

bool AUnitCubeManager::IsChunkReady(const FIntVector& ChunkPosition)
{
	if(GetChunkLoadState(ChunkPosition)->IsMarkedForIdle())
	{
		if(GetChunkLoadState(ChunkPosition)->GetCurrentResourceState() == FChunkStatus::MeshAllocated)
		{
			return true;
		}
	}
	return false;
}

void AUnitCubeManager::BuildNewWorld()
{
	if (ChunkManager)
	{
		ChunkManager->NoiseBuilder->SetNoiseSeed(WorldSeed);
		ChunkManager->PlayerPosition = {0, 0, 0};
		LoadChunkAroundPlayer(LoadDistance, false);
	}
	else
	{
		CUSTOM_LOG_WARNING(TEXT("ChunkManager is nullptr"))
	}
}

void AUnitCubeManager::LoadChunkAroundPlayer(const int& AroundDistance, bool bWithTick)
{
	if (ChunkManager)
	{
		const auto PlayerPosition = ChunkManager->PlayerPosition;
		CUSTOM_LOG_ERROR(TEXT("PlayerLocation:%s"), *PlayerPosition.ToString());
		for (int x = -AroundDistance; x <= AroundDistance; ++x)
		{
			for (int y = -AroundDistance; y <= AroundDistance; ++y)
			{
				FIntVector Offset(x, y, 0);
				FIntVector TargetPosition = PlayerPosition + Offset;
				LoadChunkAll(TargetPosition, bWithTick);
			}
		}
	}
	else
	{
		CUSTOM_LOG_WARNING(TEXT("ChunkManager is nullptr"))
	}
}

void AUnitCubeManager::LoadChunkAll(const FIntVector& ChunkPosition, bool bWithTick)
{
	if (ChunkManager == nullptr)
	{
		CUSTOM_LOG_WARNING(TEXT("ChunkManager is nullptr"))
		return;
	}
	const auto Chunk = AllocationChunks.Find(ChunkPosition);
	//如果找不到，那么肯定是需要加载的
	if (Chunk == nullptr)
	{
		const auto State = AllocationChunks.Add(ChunkPosition, MakeShareable(new FChunkStatus()));
		State->MarkToLoad();
		if (bWithTick)
		{
			LoadChunkTask_PrepareData.Enqueue(ChunkPosition);
		}
		else
		{
			SafeThread_LoadChunkData(ChunkPosition);
			State->MoveToNextResourceState();
			LoadCubeAndCubeTypeWith(ChunkPosition);
			LoadCubeMeshWith(ChunkPosition);
		}
	}
	else
	{
		//如果找到了需要判断当前区块的状况
		if ((*Chunk)->IsMarkedForIdle())
		{
			return;
		}
		if ((*Chunk)->IsMarkedForLoad() || (*Chunk)->IsMarkedForUnload())
		{
			(*Chunk)->MarkToLoad();
			if ((*Chunk)->GetCurrentResourceState() == FChunkStatus::None)
			{
				if (bWithTick)
				{
					LoadChunkTask_PrepareData.Enqueue(ChunkPosition);
				}
				else
				{
					SafeThread_LoadChunkData(ChunkPosition);
					LoadCubeAndCubeTypeWith(ChunkPosition);
					LoadCubeMeshWith(ChunkPosition);
				}
			}
			else if ((*Chunk)->GetCurrentResourceState() == FChunkStatus::DataLoaded)
			{
				if (bWithTick)
				{
					SafeThread_LoadChunkTaskAllocateResources_Enqueue(ChunkPosition);
				}
				else
				{
					LoadCubeAndCubeTypeWith(ChunkPosition);
					LoadCubeMeshWith(ChunkPosition);
				}
			}
			else if ((*Chunk)->GetCurrentResourceState() == FChunkStatus::CubeAllocated)
			{
				if (bWithTick)
				{
					LoadChunkTask_AllocateMesh.Enqueue(ChunkPosition);
				}
				else
				{
					LoadCubeMeshWith(ChunkPosition);
				}
			}
		}
	}
}

void AUnitCubeManager::LoadCubeAndCubeTypeWith(const FIntVector& ChunkPosition)
{
	if (ChunkManager == nullptr)
	{
		CUSTOM_LOG_WARNING(TEXT("ChunkManager is nullptr"));
	}
	auto Chunk = ChunkManager->GetChunkSharedPtr(ChunkPosition);
	auto PositionInWorldMap = FIntVector::ZeroValue;
	//遍历CubeMap，分配Cube,CubeType
	for (const auto& Pair : Chunk->CubeMap)
	{
		auto NewCube = CubePool->GetUnitCube();
		PositionInWorldMap = Pair.Key + Chunk->Origin;
		NewCube->SetCubeLocation(WorldMapToUE(PositionInWorldMap));
		NewCube->SetCubeType(CubeTypeManager->GetUnitCubeType(Pair.Value));
		NewCube->SetCollisionEnabled(false);
		WorldMap.Add(PositionInWorldMap, NewCube);
	}
	const auto State = AllocationChunks.Find(ChunkPosition);
	if (State)
		(*State)->MoveToNextResourceState();
	else
		CUSTOM_LOG_WARNING(TEXT("Chunk not have State:%s"), *ChunkPosition.ToString());
}

void AUnitCubeManager::LoadCubeMeshWith(const FIntVector& ChunkPosition)
{
	if (ChunkManager == nullptr)
	{
		CUSTOM_LOG_WARNING(TEXT("ChunkManager is nullptr"));
	}
	auto Chunk = ChunkManager->GetChunkSharedPtr(ChunkPosition);
	//同步SurfaceCubes
	auto PositionInWorldMap = FIntVector::ZeroValue;
	for (const auto& Cube : Chunk->SurfaceCubes)
	{
		PositionInWorldMap = Cube + Chunk->Origin;
		SurfaceCubes.Add(PositionInWorldMap);
		UpDateCubeMeshWith(PositionInWorldMap);
		SetCubeCollision(PositionInWorldMap, true);
	}
	UpDateAllMesh();
	const auto State = AllocationChunks.Find(ChunkPosition);
	if (State)
		(*State)->MoveToNextResourceState();
	else
		CUSTOM_LOG_WARNING(TEXT("Chunk not have State:%s"), *ChunkPosition.ToString());
	CUSTOM_LOG_INFO(TEXT("Load Chunck:%s"), *ChunkPosition.ToString());
}

void AUnitCubeManager::UnloadChunkAll(const FIntVector& ChunkPosition, bool bWithTick)
{
	auto Search = AllocationChunks.Find(ChunkPosition);
	if (Search == nullptr)
	{
		CUSTOM_LOG_WARNING(TEXT("Try Unload No mark Chunk:%s"), *ChunkPosition.ToString());
		return;
	}
	//存在
	if ((*Search)->IsMarkedForLoad())
	{
		return;
	}
	(*Search)->MarkToUnload();
	const auto State = (*Search)->GetCurrentResourceState();
	//如果是Idle和Unload的区块，按照资源情况进行卸载
	switch (State)
	{
	case FChunkStatus::None:
		return;
	case FChunkStatus::DataLoaded:
		return;
	case FChunkStatus::CubeAllocated:
		if (bWithTick)
		{
			UnloadChunkTask_AllocateResources.Enqueue(ChunkPosition);
		}
		else
		{
			UnloadCubeAndCubeTypeWith(ChunkPosition);
		}
		break;
	case FChunkStatus::MeshAllocated:
		if (bWithTick)
		{
			UnloadChunkTask_AllocateMesh.Enqueue(ChunkPosition);
		}
		else
		{
			UnloadCubeMeshWith(ChunkPosition);
			UnloadCubeAndCubeTypeWith(ChunkPosition);
		}
		break;
	default: ;
	}
	CUSTOM_LOG_WARNING(TEXT("Try Unload Chunk :%s"), *ChunkPosition.ToString());
}

void AUnitCubeManager::UnloadCubeMeshWith(const FIntVector& ChunkPosition)
{
	if (ChunkManager == nullptr)
	{
		CUSTOM_LOG_WARNING(TEXT("ChunkManager is nullptr"))
		return;
	}
	auto Chunk = ChunkManager->GetChunkSharedPtr(ChunkPosition);
	FIntVector PositionInWorldMap = FIntVector::ZeroValue;
	//卸载渲染
	for (const auto& Tuple : Chunk->CubeMap)
	{
		PositionInWorldMap = Tuple.Key + Chunk->Origin;
		HiedCubeAllFace(PositionInWorldMap);
	}
	UpDateAllMesh();
	//更新区块标记
	GetChunkLoadState(ChunkPosition)->MoveToNextResourceState();
}

void AUnitCubeManager::UnloadCubeAndCubeTypeWith(const FIntVector& ChunkPosition)
{
	if (ChunkManager == nullptr)
	{
		CUSTOM_LOG_WARNING(TEXT("ChunkManager is nullptr"))
		return;
	}
	auto Chunk = ChunkManager->GetChunkSharedPtr(ChunkPosition);
	FIntVector PositionInWorldMap = FIntVector::ZeroValue;
	//卸载Cube和CubeType
	for (const auto& Tuple : Chunk->CubeMap)
	{
		PositionInWorldMap = Tuple.Key + Chunk->Origin;
		ReturnUnitCubeToPool(PositionInWorldMap);
	}
	// 从WorldMap中移除
	for (const auto& Tuple : Chunk->CubeMap)
	{
		PositionInWorldMap = Tuple.Key + Chunk->Origin;
		WorldMap[PositionInWorldMap] = nullptr;
	}
	//更新区块标记
	GetChunkLoadState(ChunkPosition)->MoveToNextResourceState();
	CUSTOM_LOG_INFO(TEXT("Unload Chunk:%s"), *ChunkPosition.ToString());
}

void AUnitCubeManager::UnloadChunkNotAroundPlayer(const int& AroundDistance, bool bWithTick)
{
	//遍历所有已经分配资源的区块
	for (auto& Chunk : AllocationChunks)
	{
		if (!IsAroundPlayer(Chunk.Key, AroundDistance))
		{
			UnloadChunkAll(Chunk.Key, bWithTick);
		}
	}
}

bool AUnitCubeManager::IsAroundPlayer(const FIntVector& ChunkPosition, const int& AroundDistance) const
{
	if (ChunkManager.IsValid())
	{
		const auto PlayerPosition = ChunkManager->PlayerPosition;
		// 判断ChunkPosition是否在以PlayerPosition为中心，边长为2 * AroundDistance的立方体内
		return FMath::Abs(ChunkPosition.X - PlayerPosition.X) <= AroundDistance &&
			FMath::Abs(ChunkPosition.Y - PlayerPosition.Y) <= AroundDistance &&
			FMath::Abs(ChunkPosition.Z - PlayerPosition.Z) <= AroundDistance;
	}
	UE_LOG(LogTemp, Warning, TEXT("ChunkManager is nullptr"))
	return false;
}

void AUnitCubeManager::ReturnUnitCubeToPool(const FIntVector& CubeInWorldMap)
{
	auto Cube = WorldMap.Find(CubeInWorldMap);
	if (Cube)
	{
		if (*Cube)
		{
			CubePool->ReturnObject(*Cube);
			(*Cube)->SetCubeType(nullptr);
		}
	}
	else
	{
		CUSTOM_LOG_INFO(TEXT("can't Find Cube in WorldMap Position:%s"), *CubeInWorldMap.ToString());
	}
}

void AUnitCubeManager::UpdateThePlayerChunkLocation(AActor* Player)
{
	if (Player == nullptr || !IsValid(Player))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player is nullptr"))
		return;
	}
	auto PlayerLocation = Player->GetActorLocation();
	PlayerLocation.Z = 0;
	auto PlayerInWorldMap = UEToWorldMap(PlayerLocation);
	auto PlayerInChunkMap = WorldMapToChunkMap(PlayerInWorldMap);
	PlayerInChunkMap.Z = 0;
	if (ChunkManager)
	{
		if (PlayerInChunkMap != ChunkManager->PlayerPosition)
		{
			ChunkManager->PlayerPosition = PlayerInChunkMap;
			//加载距离为1的
			LoadChunkAroundPlayer(LoadDistance, true);
			//卸载距离为2的
			UnloadChunkNotAroundPlayer(UnloadDistance, true);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ChunkManager is nullptr"))
	}
}

void AUnitCubeManager::SynchronizePlayerPositions(AActor* Player)
{
	//尝试获取标志
	bool bExpected = false;
	if (!BIsRunning.compare_exchange_strong(bExpected, true))
	{
		return;
	}
	if (Player == nullptr || !IsValid(Player))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player is nullptr"))
		return;
	}
	if (ChunkManager)
	{
		auto ChunkMapLocation = ChunkManager->PlayerPosition;
		auto WorldMapLocation = ChunkMapToWorldMap(ChunkMapLocation);
		WorldMapLocation.X += FUnitChunk::ChunkSize.X / 2;
		WorldMapLocation.Y += FUnitChunk::ChunkSize.Y / 2;
		WorldMapLocation.Z += FUnitChunk::ChunkSize.Z;
		auto UELocation = WorldMapToUE(WorldMapLocation);
		Player->SetActorLocation(UELocation);
		UE_LOG(LogTemp, Log, TEXT("PlayerLocation:%s"), *ChunkManager->PlayerPosition.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ChunkManager is nullptr"))
	}
	//释放标志
	BIsRunning.store(false);
}

void AUnitCubeManager::BuildMeshManager()
{
	AMeshManager* NewMeshManager = GetWorld()->SpawnActor<AMeshManager>
		(AMeshManager::StaticClass(), FVector(0.0f), FRotator(0.0f));
	if (NewMeshManager)
	{
		MeshManager = NewMeshManager;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("can't spawn MeshManager"));
	}
}

void AUnitCubeManager::BuildUnitCubePool()
{
	CubePool = NewObject<UUnitCubePool>();
	CubePool->InitializeUnitCubePool(
		GetWorld(),
		FUnitChunk::ChunkSize.X * FUnitChunk::ChunkSize.Y * FUnitChunk::ChunkSize.Z * DirectionsForChunk.Num());
}

void AUnitCubeManager::SaveWorldMap()
{
	UUnitCubeMapSaveGame* SaveGameInstance = Cast<UUnitCubeMapSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UUnitCubeMapSaveGame::StaticClass())
	);
	if (!SaveGameInstance)
	{
		UE_LOG(LogTemp, Log, TEXT("Can't Create SaveGameInstance"));
		return;
	}
	SaveGameInstance->SurfaceCubes = SurfaceCubes;
	SaveGameInstance->WorldSeed = WorldSeed;
	for (const auto& Pair : WorldMap)
	{
		SaveGameInstance->CubesMap.Add(Pair.Key, Pair.Value->GetCubeType()->GetTypeEnum());
	}
	const FString SaveSlotName = "MapSaveSlot";
	const double StartTime = FPlatformTime::Seconds();
	const bool Ret = UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, 0);
	if (!Ret)
	{
		UE_LOG(LogTemp, Log, TEXT("SaveMap Faild"));
	}
	else
	{
		const double EndTime = FPlatformTime::Seconds();
		const double TotalTime = (EndTime - StartTime) * 1000.0;
		UE_LOG(LogTemp, Log, TEXT("SaveMap execution time : %2.f ms"), TotalTime);
	}
}

bool AUnitCubeManager::LoadWorldMap()
{
	const double StartTime = FPlatformTime::Seconds();
	if (UGameplayStatics::DoesSaveGameExist("MapSaveSlot", 0))
	{
		UUnitCubeMapSaveGame* LoadGameInstance = Cast<UUnitCubeMapSaveGame>(
			UGameplayStatics::LoadGameFromSlot("MapSaveSlot", 0));
		if (LoadGameInstance)
		{
			SurfaceCubes = LoadGameInstance->SurfaceCubes;
			WorldSeed = LoadGameInstance->WorldSeed;
			AUnitCube* NewCube = nullptr;
			for (const auto& Pair : LoadGameInstance->CubesMap)
			{
				NewCube = CubePool->GetUnitCube();
				NewCube->SetCubeLocation(WorldMapToUE(Pair.Key));
				NewCube->SetCubeType(CubeTypeManager->GetUnitCubeType(static_cast<EUnitCubeType>(Pair.Value)));
				//添加后配置自身的可视性
				WorldMap.Add(Pair.Key, NewCube);
			}
			for (const auto& Key : SurfaceCubes)
			{
				UpDateCubeMeshWith(Key);
				SetCubeCollision(Key, true);
			}
			UpDateAllMesh();
			const double EndTime = FPlatformTime::Seconds();
			const double TotalTime = (EndTime - StartTime) * 1000.0;
			UE_LOG(LogTemp, Log, TEXT("LoadMap execution time : %2.f ms"), TotalTime);
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Faild Load SaveSlot"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Miss LoadData"));
	}
	return false;
}


void AUnitCubeManager::UpDateCubeMeshWith(const FIntVector& Key)
{
	AUnitCube** CurrentCube = WorldMap.Find(Key);
	TSharedPtr<FUnitChunk> Chunk = nullptr;
	//如果Key存在
	if (CurrentCube)
	{
		if (*CurrentCube == nullptr)
		{
			return;
		}
		//自身存在的情况下还要考虑自身是否是透明的
		uint8 MeshType = 0;
		for (const FIntVector& Dir : DirectionsForCube)
		{
			FIntVector NeighbourPosition = Key + Dir;
			AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
			if (NeighbourCube)
			{
				if ((*NeighbourCube) == nullptr)
				{
					//邻居不存在
					MeshManager->AddMeshToCubeWith(Dir, (*CurrentCube));
					continue;
				}
				//邻居存在
				if(AUnitCube::IsShouldAddMesh(*CurrentCube,*NeighbourCube))
				{
					MeshManager->AddMeshToCubeWith(Dir,(*CurrentCube));
				}else
				{
					MeshManager->DelMeshToCubeWith(Dir,(*CurrentCube));
				}
			}
			else
			{
				//邻居不存在
				MeshManager->AddMeshToCubeWith(Dir, (*CurrentCube));
			}
			++MeshType;
		}
	}
	else
	{
		CUSTOM_LOG_INFO(TEXT("The location does not exist in the map: (%s)"), *Key.ToString());
	}
}

void AUnitCubeManager::UpDateCubeMeshWith(const AUnitCube* Cube)
{
	if (Cube && IsValid(Cube))
	{
		const FIntVector Key = UEToWorldMap(Cube->GetActorLocation());
		UpDateCubeMeshWith(Key);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("This Actor does not exist"));
	}
}

void AUnitCubeManager::UpDateAllMesh() const
{
	if (MeshManager && IsValid(MeshManager))
	{
		MeshManager->UpdateAllInstancedMesh();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Miss MeshManager"));
	}
}

//向下取整的整数除法
int Div_Floor(const int A, const int B)
{
	if (A >= 0)
	{
		return A / B;
	}
	else
	{
		return (A - B + 1) / B;
	}
}
//向下取整的整数取余
int Mod_Floor(const int A, const int B) {
	int Ret = A % B;
	if (Ret < 0) {
		Ret += B;
	}
	return Ret;
}

FIntVector AUnitCubeManager::UEToWorldMap(const FVector& UECoord)
{
	return {
		Div_Floor(static_cast<int32>(UECoord.X), static_cast<int32>(AUnitCube::CubeSize.X)),
		Div_Floor(static_cast<int32>(UECoord.Y), static_cast<int32>(AUnitCube::CubeSize.Y)),
		Div_Floor(static_cast<int32>(UECoord.Z), static_cast<int32>(AUnitCube::CubeSize.Z))
	};
}

FIntVector AUnitCubeManager::UEToChunkMap(const FVector& UECoord)
{
	return WorldMapToChunkMap(UEToWorldMap(UECoord));
}

FVector AUnitCubeManager::WorldMapToUE(const FIntVector& WorldMapCoord)
{
	return {
		WorldMapCoord.X * AUnitCube::CubeSize.X,
		WorldMapCoord.Y * AUnitCube::CubeSize.Y,
		WorldMapCoord.Z * AUnitCube::CubeSize.Z
	};
}


FIntVector AUnitCubeManager::WorldMapToChunkMap(const FIntVector& WorldMapCoord)
{
	int X = Div_Floor(WorldMapCoord.X, FUnitChunk::ChunkSize.X);
	int Y = Div_Floor(WorldMapCoord.Y, FUnitChunk::ChunkSize.Y);
	int Z = Div_Floor(WorldMapCoord.Z, FUnitChunk::ChunkSize.Z);
	return {X, Y, Z};
}

FIntVector AUnitCubeManager::ChunkMapToWorldMap(const FIntVector& ChunkMapCoord)
{
	return {
		ChunkMapCoord.X * FUnitChunk::ChunkSize.X,
		ChunkMapCoord.Y * FUnitChunk::ChunkSize.Y,
		ChunkMapCoord.Z * FUnitChunk::ChunkSize.Z
	};
}

FVector AUnitCubeManager::ChunkMapToUE(const FIntVector& ChunkMapCoord)
{
	return WorldMapToUE(ChunkMapToWorldMap(ChunkMapCoord));
}

FIntVector AUnitCubeManager::WorldMapToCubeMap(const FIntVector& WorldMapCoord)
{
	return {
		Mod_Floor(WorldMapCoord.X, FUnitChunk::ChunkSize.X),
		Mod_Floor(WorldMapCoord.Y, FUnitChunk::ChunkSize.Y),
		Mod_Floor(WorldMapCoord.Z, FUnitChunk::ChunkSize.Z)
	};
}

void AUnitCubeManager::AddCubeWith(const FVector& Scene, const int& Type)
{
	bool bExpected = false;
	if (!BIsAdding.compare_exchange_strong(bExpected, true))
	{
		return;
	}
	//需要添加到对应的区块
	const FIntVector Key = UEToWorldMap(Scene);
	const FIntVector ChunkPosition = WorldMapToChunkMap(Key);
	//如果区块没有准备就绪，不允许添加
	if(!IsChunkReady(ChunkPosition))
	{
		CUSTOM_LOG_WARNING(TEXT("Chunk not Ready:%s"),*ChunkPosition.ToString());
		BIsAdding.store(false);
		return;
	}	
	if (ChunkManager == nullptr)
	{
		CUSTOM_LOG_WARNING(TEXT("ChunkManager is nullptr"));
		BIsAdding.store(false);
		return;
	}
	
	auto NewCube = CubePool->GetUnitCube();
	NewCube->SetCubeLocation(WorldMapToUE(Key));
	NewCube->SetCubeType(CubeTypeManager->GetUnitCubeType(static_cast<EUnitCubeType>(Type)));
	//添加后配置自身的可视性
	WorldMap.Add(Key, NewCube);
	SurfaceCubes.Add(Key);
	UpDateCubeMeshWith(Key);
	//同步区块信息
	auto Chunk =  ChunkManager->GetChunkSharedPtr(ChunkPosition);
	Chunk->AddCubeWith(WorldMapToCubeMap(Key), Type);
	Chunk->AddSurfaceCubeWith(WorldMapToCubeMap(Key));
	//检查以添加方块为中心的方块。
	for (const FIntVector& Dir : DirectionsForCube)
	{
		FIntVector NeighbourPosition = Key + Dir;
		AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
		if (NeighbourCube && (*NeighbourCube) != nullptr)
		{
			//如果邻居存在，更新邻居的隐藏配置
			UpDateCubeMeshWith(NeighbourPosition);
		}
	}
	UpDateAllMesh();
	for (const FIntVector& Dir : DirectionsForCube)
	{
		FIntVector NeighbourPosition = Key + Dir;
		AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
		if (NeighbourCube && (*NeighbourCube) != nullptr)
		{
			auto NeighubourChunk = ChunkManager->GetChunkSharedPtr(WorldMapToChunkMap(NeighbourPosition));
			//更新邻居的碰撞
			if ((*NeighbourCube)->RefreshCollisionEnabled())
			{
				SurfaceCubes.Add(NeighbourPosition);
				NeighubourChunk->AddSurfaceCubeWith(NeighbourPosition);
			}
			else
			{
				SurfaceCubes.Remove(NeighbourPosition);
				NeighubourChunk->DelSurfaceCubeWith(NeighbourPosition);
			}
		}
	}
	NewCube->RefreshCollisionEnabled();
	CUSTOM_LOG_INFO(TEXT("Add Cube in WorldMap:%s,in Chunk:%s"),*Key.ToString(),*ChunkPosition.ToString());
	BIsAdding.store(false);
}

void AUnitCubeManager::DelCubeWith(const FVector& Scene)
{
	FIntVector Key = UEToWorldMap(Scene);
	AUnitCube** Cube = WorldMap.Find(Key);
	if (Cube && (*Cube)) //如果有找到的
	{
		//移除
		WorldMap.Remove(Key);
		SurfaceCubes.Remove(Key);
		if (ChunkManager == nullptr)
		{
			CUSTOM_LOG_WARNING(TEXT("ChunkManager is nullptr"));
			return;
		}
		//同步区块
		auto Chunk = ChunkManager->GetChunkSharedPtr(WorldMapToChunkMap(Key));
		Chunk->DelCubeWith(WorldMapToCubeMap(Key));
		Chunk->DelSurfaceCubeWith(WorldMapToCubeMap(Key));
		
		HiedCubeAllFace(*Cube);
		//刷新周围
		for (const FIntVector& Dir : DirectionsForCube)
		{
			FIntVector NeighbourPosition = Key + Dir;
			AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
			if (NeighbourCube && *NeighbourCube)
			{
				//如果邻居存在，更新隐藏配置
				UpDateCubeMeshWith(NeighbourPosition);
			}
		}
		MeshManager->UpdateAllInstancedMesh();
		for (const FIntVector& Dir : DirectionsForCube)
		{
			FIntVector NeighbourPosition = Key + Dir;
			AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
			if (NeighbourCube && *NeighbourCube)
			{
				auto NeighbourChunk = ChunkManager->GetChunkSharedPtr(WorldMapToChunkMap(NeighbourPosition));
				//需要等待所有邻居的网格体实例更新完，再更新碰撞。
				if ((*NeighbourCube)->RefreshCollisionEnabled())
				{
					SurfaceCubes.Add(NeighbourPosition);
					NeighbourChunk->AddSurfaceCubeWith(NeighbourPosition);
				}
				else
				{
					SurfaceCubes.Remove(NeighbourPosition);
					NeighbourChunk->DelSurfaceCubeWith(NeighbourPosition);
				}
			}
		}
		//Cube销毁
		//(*Cube)->OnDestroyed();
		CubePool->ReturnObject(*Cube);
		//刷新遮挡
		FlushRenderingCommands();
	}
	else
	{
		//Key不存在：
		UE_LOG(LogTemp, Log, TEXT("The location does not exist in the map: (%s)"), *Key.ToString());
	}
}

void AUnitCubeManager::HiedCubeAllFace(AUnitCube* Cube)
{
	if (Cube && IsValid(Cube))
	{
		for (const FIntVector& Dir : DirectionsForCube)
		{
			MeshManager->DelMeshToCubeWith(Dir, Cube);
		}
	}
}

void AUnitCubeManager::HiedCubeAllFace(const FIntVector& WorldMapPosition)
{
	auto Cube = WorldMap.Find(WorldMapPosition);
	if (Cube && *Cube)
	{
		HiedCubeAllFace(*Cube);
	}
	else
	{
		CUSTOM_LOG_INFO(TEXT("can't find cube with worldposition:%s"), *WorldMapPosition.ToString());
	}
}

void AUnitCubeManager::SetCubeCollision(const FIntVector& Key, bool IsTurnOn)
{
	AUnitCube** Cube = WorldMap.Find(Key);
	if (Cube && (*Cube))
	{
		(*Cube)->SetCollisionEnabled(IsTurnOn);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("The key location does not exist in the map: (%s)"), *Key.ToString());
	}
}
