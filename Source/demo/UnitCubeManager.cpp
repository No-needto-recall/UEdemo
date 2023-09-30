#include "UnitCubeManager.h"

#include "FastNoiseLite.h"
#include "MeshManager.h"
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
	}
}

// Called every frame
void AUnitCubeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUnitCubeManager::BuildNewWorld()
{
	if(ChunkManager)
	{
		ChunkManager->NoiseBuilder->SetNoiseSeed(WorldSeed);
		ChunkManager->PlayerPosition = {0,0,0};
		LoadChunkAroundPlayer();
		OnLoadChunkComplete.Broadcast();
	}else
	{
		UE_LOG(LogTemp,Warning,TEXT("ChunkManager is nullptr"))
	}
}

void AUnitCubeManager::LoadChunkAroundPlayer()
{
	if(ChunkManager)
	{
		const auto PlayerPosition = ChunkManager->PlayerPosition;
		UE_LOG(LogTemp, Log, TEXT("PlayerLocation:%s"),*PlayerPosition.ToString());
		for (const auto& Location : DirectionsForChunk)
		{
			LoadChunkAll(Location + PlayerPosition);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ChunkManager is nullptr"))
	}
}

void AUnitCubeManager::LoadChunkAll(const FIntVector& ChunkPosition)
{
	if(ChunkManager)
	{
		const auto Chunk = AllocationChunks.Find(ChunkPosition);
		if(Chunk && *Chunk)
		{
			return;
		}
		//加载对应区块的数据
		ChunkManager->LoadChunkWith(ChunkPosition);
		//加载区块结束后，开始处理分配Cube和CubeType以及渲染
		LoadCubeAndCubeTypeWith(ChunkPosition);
		//分配区块的渲染
		LoadCubeMeshWith(ChunkPosition);
		AllocationChunks.Add(ChunkPosition,true);
		UE_LOG(LogTemp,Log,TEXT("Load Chunck:%s"),*ChunkPosition.ToString());
	}else
	{
		UE_LOG(LogTemp,Warning,TEXT("ChunkManager is nullptr"))
	}
}

void AUnitCubeManager::LoadCubeAndCubeTypeWith(const FIntVector& ChunkPosition)
{
	if(ChunkManager)
	{
		auto Chunk = ChunkManager->GetChunkSharedPtr(ChunkPosition);
		auto PositionInWorldMap = FIntVector::ZeroValue;
		//遍历CubeMap，分配Cube,CubeType
		for(const auto& Pair:Chunk->CubeMap)
		{
			auto NewCube = CubePool->GetUnitCube();
			PositionInWorldMap = Pair.Key+Chunk->Origin;	
			NewCube->SetCubeLocation(WorldMapToUE(PositionInWorldMap));
			NewCube->SetCubeType(CubeTypeManager->GetUnitCubeType(Pair.Value));
			NewCube->SetCollisionEnabled(false);
			WorldMap.Add(PositionInWorldMap,NewCube);
		}
	}else
	{
		UE_LOG(LogTemp,Warning,TEXT("ChunkManager is nullptr"))
	}
}

void AUnitCubeManager::LoadCubeMeshWith(const FIntVector& ChunkPosition)
{
	if(ChunkManager)
	{
		auto Chunk = ChunkManager->GetChunkSharedPtr(ChunkPosition);
		//同步SurfaceCubes
		auto PositionInWorldMap = FIntVector::ZeroValue;
		for (const auto& Cube : Chunk->SurfaceCubes)
		{
			PositionInWorldMap = Cube+Chunk->Origin;
			SurfaceCubes.Add(PositionInWorldMap);
			UpDateCubeMeshWith(PositionInWorldMap);
			SetCubeCollision(PositionInWorldMap,true);
		}
		UpDateAllMesh();
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("ChunkManager is nullptr"))
	}
}

void AUnitCubeManager::UnloadChunk(const FIntVector& ChunkPosition)
{
	//将该区块分配的资源卸载
	//卸载渲染
	if(ChunkManager)
	{
		auto Chunk = ChunkManager->GetChunkSharedPtr(ChunkPosition);
		FIntVector PositionInWorldMap = FIntVector::ZeroValue;
		AUnitCube* Cube = nullptr;
		for(const auto& Tuple : Chunk->CubeMap)
		{
			PositionInWorldMap = Tuple.Key + Chunk->Origin;
			HiedCubeAllFace(PositionInWorldMap);
		}
		UpDateAllMesh();
		for (const auto& Tuple : Chunk->CubeMap)
		{
			PositionInWorldMap = Tuple.Key + Chunk->Origin;
			ReturnUnitCubeToPool(PositionInWorldMap);
		}
		AllocationChunks[ChunkPosition] = false;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ChunkManager is nullptr"))
	}
	//卸载Cube和CubeType
}

void AUnitCubeManager::UnloadChunkNotAroundPlayer()
{
	for(auto& Chunk : AllocationChunks)
	{
		if(!IsAroundPlayer(Chunk.Key))
		{
			UnloadChunk(Chunk.Key);	
		}
	}
}

bool AUnitCubeManager::IsAroundPlayer(const FIntVector& ChunkPosition)
{
	if(ChunkManager)
	{
		const auto PlayerPosition = ChunkManager->PlayerPosition;
		for(const auto& Position:DirectionsForChunk)
		{
			if(ChunkPosition == Position + PlayerPosition)
			{
				return true;
			}
		}
		return false;
	}else
	{
		UE_LOG(LogTemp,Warning,TEXT("ChunkManager is nullptr"))
		return false;
	}
}

void AUnitCubeManager::ReturnUnitCubeToPool(const FIntVector& CubeInWorldMap)
{
	auto Cube = WorldMap.Find(CubeInWorldMap);
	if(Cube)
	{
		CubePool->ReturnObject(*Cube);
		(*Cube)->SetCubeType(nullptr);
		WorldMap.Remove(CubeInWorldMap);
	}else
	{
		UE_LOG(LogTemp,Log,TEXT("can't Find Cube in WorldMap Position:%s"),*CubeInWorldMap.ToString());
	}
}

void AUnitCubeManager::UpdateThePlayerChunkLocation(AActor* Player)
{
	if(Player == nullptr || !IsValid(Player))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player is nullptr"))
		return;
	}
	auto PlayerLocation = Player->GetActorLocation();
	PlayerLocation.Z = 0;
	auto PlayerInWorldMap = UEToWorldMap(PlayerLocation);
	auto PlayerInChunkMap = WorldMapToChunkMap(PlayerInWorldMap);
	PlayerInChunkMap.Z = 0;
	if(ChunkManager)
	{
		if (PlayerInChunkMap != ChunkManager->PlayerPosition)
		{
			ChunkManager->PlayerPosition = PlayerInChunkMap;
			LoadChunkAroundPlayer();
			UnloadChunkNotAroundPlayer();
		}
		//UE_LOG(LogTemp,Log,TEXT("PlayerLocation:%s,PlayerWorldPosition:%s,PlayerChunkPositon:%s"),*PlayerLocation.ToString(),*PlayerInWorldMap.ToString(),*PlayerInChunkMap.ToString());
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
	if(!BIsRunning.compare_exchange_strong(bExpected,true))
	{
		return;
	}
	if(Player == nullptr || !IsValid(Player))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player is nullptr"))
		return;
	}
	if(ChunkManager)
	{
		auto ChunkMapLocation = ChunkManager->PlayerPosition;
		auto WorldMapLocation = ChunkMapToWorldMap(ChunkMapLocation);
		WorldMapLocation.X += FUnitChunk::ChunkSize.X/2;
		WorldMapLocation.Y += FUnitChunk::ChunkSize.Y/2;
		WorldMapLocation.Z += FUnitChunk::ChunkSize.Z;
		auto UELocation = WorldMapToUE(WorldMapLocation);
		Player->SetActorLocation(UELocation);
		UE_LOG(LogTemp, Log, TEXT("PlayerLocation:%s"),*ChunkManager->PlayerPosition.ToString());
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

void AUnitCubeManager::BuildMap()
{
	AUnitCube* NewCube = nullptr;

	for (int i = 0; i < Size.Z; ++i)
	{
		for (int k = 0; k < Size.Y; ++k)
		{
			for (int j = 0; j < Size.X; ++j)
			{
				NewCube = CubePool->GetUnitCube();
				NewCube->SetCubeLocation(MapToScene(FIntVector(j, k, i)));
				NewCube->SetCubeType(CubeTypeManager->GetUnitCubeType(EUnitCubeType::Stone));
				WorldMap.Add(FIntVector(j, k, i), NewCube);
			}
		}
	}
}

void AUnitCubeManager::BuildMapWithNoise()
{
	const double StartTime = FPlatformTime::Seconds();

	AUnitCube* NewCube = nullptr;
	FastNoiseLite Noise;
	//设置种子和噪音类型
	Noise.SetSeed(WorldSeed);
	Noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
	//玩家出生点为0.0，生成为17*17*257的地图
	for (int x = -Size.X; x <= Size.X; ++x)
	{
		for (int y = -Size.Y; y <= Size.Y; ++y)
		{
			const float RawNoise = Noise.GetNoise(static_cast<float>(x), static_cast<float>(y));
			const float NormalizedValue = (RawNoise + 1.0f) * 0.5f;
			//const float MappedValue = NormalizedValue * 256.0f - 128.0f;
			const float MappedValue = NormalizedValue * static_cast<float>(Size.Z * 2) - static_cast<float>(Size.Z);
			const int MaxZ = static_cast<int>(std::round(MappedValue));
			SurfaceCubes.Add(FIntVector(x, y, MaxZ));
			for (int z = -Size.Z; z <= MaxZ; ++z)
			{
				NewCube = CubePool->GetUnitCube();
				NewCube->SetCubeLocation(MapToScene(FIntVector(x, y, z)));
				WorldMap.Add(FIntVector(x, y, z), NewCube);
				if (z == -Size.Z) //最底层为基岩
				{
					NewCube->SetCubeType(CubeTypeManager->GetUnitCubeType(EUnitCubeType::BedRock));
				}
				else if (MaxZ - z <= 5) //地表以下10格子为草方块
				{
					NewCube->SetCubeType(CubeTypeManager->GetUnitCubeType(EUnitCubeType::Grass));
				}
				else
				{
					NewCube->SetCubeType(CubeTypeManager->GetUnitCubeType(EUnitCubeType::Stone));
				}
			}
		}
	}
	const double EndTime = FPlatformTime::Seconds();
	const double TotalTime = (EndTime - StartTime) * 1000.0;
	UE_LOG(LogTemp, Log, TEXT("BuildMapWithNoise execution time : %2.f ms"), TotalTime);
}

void AUnitCubeManager::BuildAllCubesMesh()
{
	//遍历地图
	for (const auto& Pair : WorldMap)
	{
		FIntVector CurrentPosition = Pair.Key;
		AUnitCube* CurrentCube = Pair.Value;
		//是边界方块，且不是表面方块
		if (IsABorderCube(CurrentPosition) && !IsSurfaceCube(CurrentPosition))
		{
			CurrentCube->SetCollisionEnabled(false);
			continue;
		}
		//判断坐标是否为当前渲染区域的边界坐标。

		if (CurrentCube && IsValid(CurrentCube) && CurrentCube->IsSolid()) //检查方块是否存在，且是实体
		{
			int EnabledCollision = 0;
			uint8 MeshType = 0;
			for (const FIntVector& Dir : DirectionsForCube)
			{
				FIntVector NeighbourPosition = CurrentPosition + Dir;
				AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
				//因为所有cube默认没有静态网格体实例，所以检测存在不为实心||不存在
				if (NeighbourCube && IsValid((*NeighbourCube)))
				{
					if ((*NeighbourCube)->IsTransparent())
					{
						//在对应的Dir添加静态网格体实例
						MeshManager->AddMeshToCubeWith(Dir, CurrentCube);
						SurfaceCubes.Add(CurrentPosition);
					}
					else
					{
						++EnabledCollision;
					}
				}
				else //邻居不存在
				{
					MeshManager->AddMeshToCubeWith(Dir, CurrentCube);
					SurfaceCubes.Add(CurrentPosition);
				}
				++MeshType;
			}
			//循环结束后，如果方块没被实心体包围，则开启碰撞
			if (EnabledCollision != 6)
			{
				CurrentCube->SetCollisionEnabled(true);
			}
			else
			{
				CurrentCube->SetCollisionEnabled(false);
			}
		}
	}
} 
bool AUnitCubeManager::IsSurfaceCube(const FIntVector& Position) const
{
	return SurfaceCubes.Contains(Position);
}

bool AUnitCubeManager::IsABorderCube(const FIntVector& Position) const
{
	if (Position.X == -Size.X ||
		Position.X == Size.X ||
		Position.Y == -Size.Y ||
		Position.Y == Size.Y ||
		Position.Z == -Size.Z)
	{
		return true;
	}
	else
	{
		return false;
	}
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
				NewCube->SetCubeLocation(MapToScene(Pair.Key));
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
		UE_LOG(LogTemp, Log, TEXT("The location does not exist in the map: (%s)"), *Key.ToString());
	}
}

void AUnitCubeManager::UpDateCubeMeshWith(const AUnitCube* Cube)
{
	if (Cube && IsValid(Cube))
	{
		const FIntVector Key = SceneToMap(Cube->GetActorLocation());
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

FVector AUnitCubeManager::MapToScene(const FIntVector& MapCoord)
{
	return FVector(100 * MapCoord.X, 100 * MapCoord.Y, 100 * MapCoord.Z);
}

FIntVector AUnitCubeManager::SceneToMap(const FVector& Scene)
{
	return FIntVector(Scene.X / 100, Scene.Y / 100, Scene.Z / 100);
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

FIntVector AUnitCubeManager::WorldMapToChunkMap(const FIntVector& WorldMapCoord)
{
	int X = WorldMapCoord.X / FUnitChunk::ChunkSize.X;
	int Y = WorldMapCoord.Y / FUnitChunk::ChunkSize.Y;
	int Z = WorldMapCoord.Z / FUnitChunk::ChunkSize.Z;
	if(WorldMapCoord.X<0)--X;
	if(WorldMapCoord.Y<0)--Y;
	if(WorldMapCoord.Z<0)--Z;
	return {X,Y,Z};
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
	//需要添加到对应的区块
	if (!IsLock)
	{
		IsLock = true;
		FIntVector Key = UEToWorldMap(Scene);
		if(ChunkManager)
		{
			ChunkManager->GetChunkSharedPtr(WorldMapToChunkMap(Key))->AddCubeWith(WorldMapToCubeMap(Key),Type);
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
		IsLock = false;
	}
}

void AUnitCubeManager::DelCubeWith(const FVector& Scene)
{
	FIntVector Key = SceneToMap(Scene);
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
	if(Cube)
	{
		HiedCubeAllFace(*Cube);
	}else
	{
		UE_LOG(LogTemp,Log,TEXT("can't find cube with worldposition:%s"),*WorldMapPosition.ToString());
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
