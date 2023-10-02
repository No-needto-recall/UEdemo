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
	: WorldSeed(0), CubePool(nullptr)
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
	BuildMeshManager();
	BuildUnitCubePool();
	//BuildMap();
	if (!LoadWorldMap())
	{
		//BuildMapWithNoise();
		//BuildAllCubesMesh();
		//UpDateAllMesh();
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
}

// Called every frame
void AUnitCubeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//每一帧先处理加载，后处理卸载
	ExecuteLoadChunkTask(TaskNum1, TaskNum2, TaskNum3);
	ExecuteUnloadTask(1);
}

void AUnitCubeManager::ProcessTaskQueue(TQueue<FIntVector>& TaskQueue, int NumTasks,
                                        std::function<void(FIntVector)> TaskFunction)
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
		ChunkManager->LoadChunkWith(Task);
	});
	if (!LoadChunkTask_PrepareData.IsEmpty())
		return;
	//分配cube
	ProcessTaskQueue(LoadChunkTask_AllocateResources, N2, [this](FIntVector Task)
	{
		LoadCubeAndCubeTypeWith(Task);
	});
	if (!LoadChunkTask_AllocateResources.IsEmpty())
		return;
	//分配mesh
	ProcessTaskQueue(LoadChunkTask_AllocateMesh, N3, [this](FIntVector Task)
	{
		LoadCubeMeshWith(Task);
		AllocationChunks.Add(Task, true);
		CUSTOM_LOG_INFO(TEXT("Load Chunck:%s"), *Task.ToString());
	});
}

void AUnitCubeManager::ExecuteUnloadTask(const int& N)
{
	if (!LoadChunkTask_AllocateResources.IsEmpty() && !LoadChunkTask_PrepareData.IsEmpty() && !
		LoadChunkTask_AllocateMesh.IsEmpty())
	{
		return;
	}
	for (int i = 0; i < N; ++i)
	{
		FIntVector Task;
		if (UnloadChunkTask.Dequeue(Task))
		{
			UnloadChunkAll(Task);
		}
		else
		{
			break;
		}
	}
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
				if (bWithTick)
				{
					LoadChunkAll(TargetPosition, true);
				}
				else
				{
					LoadChunkAll(TargetPosition, false);
				}
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
	if (ChunkManager)
	{
		const auto Chunk = AllocationChunks.Find(ChunkPosition);
		if (Chunk && *Chunk)
		{
			return;
		}
		if (bWithTick)
		{
			LoadChunkTask_PrepareData.Enqueue(ChunkPosition);
			LoadChunkTask_AllocateResources.Enqueue(ChunkPosition);
			LoadChunkTask_AllocateMesh.Enqueue(ChunkPosition);
		}
		else
		{
			//加载对应区块的数据
			ChunkManager->LoadChunkWith(ChunkPosition);
			//加载区块结束后，开始处理分配Cube和CubeType以及渲染
			LoadCubeAndCubeTypeWith(ChunkPosition);
			//分配区块的渲染
			LoadCubeMeshWith(ChunkPosition);
			AllocationChunks.Add(ChunkPosition, true);
			CUSTOM_LOG_INFO(TEXT("Load Chunck:%s"), *ChunkPosition.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ChunkManager is nullptr"))
	}
}

void AUnitCubeManager::LoadCubeAndCubeTypeWith(const FIntVector& ChunkPosition)
{
	if (ChunkManager)
	{
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
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ChunkManager is nullptr"))
	}
}

void AUnitCubeManager::LoadCubeMeshWith(const FIntVector& ChunkPosition)
{
	if (ChunkManager)
	{
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
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ChunkManager is nullptr"))
	}
}

void AUnitCubeManager::UnloadChunkAll(const FIntVector& ChunkPosition)
{
	auto Seach = AllocationChunks.Find(ChunkPosition);
	if (Seach == nullptr)
	{
		//CUSTOM_LOG_WARNING(TEXT("Try Unload No mark Chunk:%s"),*ChunkPosition.ToString());
		return;
	}
	if (*Seach == false)
	{
		//CUSTOM_LOG_WARNING(TEXT("Try Unload Chunk is notAllocation:%s"),*ChunkPosition.ToString());
		return;
	}
	//将该区块分配的资源卸载
	if (ChunkManager)
	{
		auto Chunk = ChunkManager->GetChunkSharedPtr(ChunkPosition);
		FIntVector PositionInWorldMap = FIntVector::ZeroValue;
		//卸载渲染
		for (const auto& Tuple : Chunk->CubeMap)
		{
			PositionInWorldMap = Tuple.Key + Chunk->Origin;
			HiedCubeAllFace(PositionInWorldMap);
		}
		UpDateAllMesh();
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
			WorldMap.Remove(PositionInWorldMap);
		}
		AllocationChunks[ChunkPosition] = false;
		CUSTOM_LOG_INFO(TEXT("Unload Chunk:%s"), *ChunkPosition.ToString());
	}
	else
	{
		CUSTOM_LOG_WARNING(TEXT("ChunkManager is nullptr"))
	}
}

void AUnitCubeManager::UnloadChunkNotAroundPlayer(const int& AroundDistance, bool bWithTick)
{
	//遍历所有已经分配资源的区块
	for (auto& Chunk : AllocationChunks)
	{
		if (!IsAroundPlayer(Chunk.Key, AroundDistance))
		{
			if (bWithTick)
			{
				UnloadChunkTask.Enqueue(Chunk.Key);
			}
			else
			{
				UnloadChunkAll(Chunk.Key);
			}
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
		CubePool->ReturnObject(*Cube);
		(*Cube)->SetCubeType(nullptr);
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
			LoadChunkAroundPlayer(LoadDistance);
			//卸载距离为2的
			UnloadChunkNotAroundPlayer(UnloadDistance);
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
	if (CurrentCube && IsValid((*CurrentCube)))
	{
		uint8 MeshType = 0;
		for (const FIntVector& Dir : DirectionsForCube)
		{
			FIntVector NeighbourPosition = Key + Dir;
			AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
			if (NeighbourCube && IsValid((*NeighbourCube)))
			{
				//邻居存在
				if ((*NeighbourCube)->IsSolid())
				{
					//邻居是实心的
					//设置对应的面不可见
					MeshManager->DelMeshToCubeWith(Dir, (*CurrentCube));
				}
				else
				{
					MeshManager->AddMeshToCubeWith(Dir, (*CurrentCube));
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

FIntVector AUnitCubeManager::UEToWorldMap(const FVector& UECoord)
{
	return {
		static_cast<int32>(UECoord.X / AUnitCube::CubeSize.X),
		static_cast<int32>(UECoord.Y / AUnitCube::CubeSize.Y),
		static_cast<int32>(UECoord.Z / AUnitCube::CubeSize.Z)
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

//向下取整的整数除法
int Div_Floor(int a, int b)
{
	if (a >= 0)
	{
		return a / b;
	}
	else
	{
		return (a - b + 1) / b;
	}
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
		WorldMapCoord.X % FUnitChunk::ChunkSize.X,
		WorldMapCoord.Y % FUnitChunk::ChunkSize.Y,
		WorldMapCoord.Z % FUnitChunk::ChunkSize.Z
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
	FIntVector Key = UEToWorldMap(Scene);
	if (ChunkManager)
	{
		ChunkManager->GetChunkSharedPtr(WorldMapToChunkMap(Key))->AddCubeWith(WorldMapToCubeMap(Key), Type);
	}
	auto NewCube = CubePool->GetUnitCube();
	NewCube->SetCubeLocation(WorldMapToUE(Key));
	NewCube->SetCubeType(CubeTypeManager->GetUnitCubeType(static_cast<EUnitCubeType>(Type)));
	//添加后配置自身的可视性
	WorldMap.Add(Key, NewCube);
	SurfaceCubes.Add(Key);
	UpDateCubeMeshWith(Key);
	//检查以添加方块为中心的方块。
	for (const FIntVector& Dir : DirectionsForCube)
	{
		FIntVector NeighbourPosition = Key + Dir;
		AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
		if (NeighbourCube)
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
		if (NeighbourCube)
		{
			//更新邻居的碰撞
			if ((*NeighbourCube)->RefreshCollisionEnabled())
			{
				SurfaceCubes.Add(NeighbourPosition);
			}
			else
			{
				SurfaceCubes.Remove(NeighbourPosition);
			}
		}
	}
	NewCube->RefreshCollisionEnabled();
	BIsAdding.store(false);
}

void AUnitCubeManager::DelCubeWith(const FVector& Scene)
{
	FIntVector Key = UEToWorldMap(Scene);
	AUnitCube** Cube = WorldMap.Find(Key);
	if (Cube) //如果有找到的
	{
		//移除
		WorldMap.Remove(Key);
		SurfaceCubes.Remove(Key);
		HiedCubeAllFace(*Cube);
		//刷新周围
		for (const FIntVector& Dir : DirectionsForCube)
		{
			FIntVector NeighbourPosition = Key + Dir;
			AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
			if (NeighbourCube)
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
			if (NeighbourCube)
			{
				//需要等待所有邻居的网格体实例更新完，再更新碰撞。
				if ((*NeighbourCube)->RefreshCollisionEnabled())
				{
					SurfaceCubes.Add(NeighbourPosition);
				}
				else
				{
					SurfaceCubes.Remove(NeighbourPosition);
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
	if (Cube)
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
	if (Cube)
	{
		(*Cube)->SetCollisionEnabled(IsTurnOn);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("The key location does not exist in the map: (%s)"), *Key.ToString());
	}
}

FChunkLoaderRunnable::FChunkLoaderRunnable(AUnitCubeManager* InManager)
	: Manager(InManager), bShouldRun(true)
{
	EventTrigger = FPlatformProcess::GetSynchEventFromPool(); //创建事件
}

void FChunkLoaderRunnable::TriggerExecution()
{
	EventTrigger->Trigger(); //触发事件，使线程开始执行
}

void FChunkLoaderRunnable::Stop()
{
	bShouldRun = false;
	EventTrigger->Trigger(); //确保线程从等待状态中退出
}

uint32 FChunkLoaderRunnable::Run()
{
	while (bShouldRun)
	{
		EventTrigger->Wait(); // 等待事件被触发
		if (!bShouldRun)
		{
			return 0; //结束线程
		}
		CriticalSection.Lock();
		if (Manager)
		{
			//使用Manager来调用 AUnitCubeManager 的成员函数或者变量
			Manager->LoadChunkAroundPlayer();
			Manager->UnloadChunkNotAroundPlayer();
		}
		CriticalSection.Unlock();
		FPlatformProcess::Sleep(0.1);
	}
	return 0;
}
