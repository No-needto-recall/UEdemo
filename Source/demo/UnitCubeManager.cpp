#include "UnitCubeManager.h"

#include "FastNoiseLite.h"
#include "MeshManager.h"
#include "UnitCube.h"
#include "UnitCubeMapSaveGame.h"
#include "UnitCubeType.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AUnitCubeManager::AUnitCubeManager()
	: WorldSeed(0)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshManager = nullptr;
}

// Called when the game starts or when spawned
void AUnitCubeManager::BeginPlay()
{
	Super::BeginPlay();
	BuildMeshManager();
	//BuildMap();
	if (!LoadWorldMap())
	{
		BuildMapWithNoise();
		BuildAllCubesMesh();
		UpDateAllMesh();
	}
}

// Called every frame
void AUnitCubeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

void AUnitCubeManager::BuildMap()
{
	AUnitCube* NewCube = nullptr;

	for (int i = 0; i < Size.Z; ++i)
	{
		for (int k = 0; k < Size.Y; ++k)
		{
			for (int j = 0; j < Size.X; ++j)
			{
				NewCube = GetWorld()->SpawnActor<AUnitCube>(AUnitCube::StaticClass(),
				                                            MapToScene(FIntVector(j, k, i)), FRotator(0.0f));
				NewCube->CubeType = UUnitCubeType::BuildUnitCubeType(EUnitCubeType::Stone);
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
				NewCube = GetWorld()->SpawnActor<AUnitCube>(AUnitCube::StaticClass(),
				                                            MapToScene(FIntVector(x, y, z)), FRotator(0.0f));
				WorldMap.Add(FIntVector(x, y, z), NewCube);
				if (z == -Size.Z) //最底层为基岩
				{
					NewCube->CubeType = UUnitCubeType::BuildUnitCubeType(EUnitCubeType::BedRock);
				}
				else if (MaxZ - z <= 5) //地表以下10格子为草方块
				{
					NewCube->CubeType = UUnitCubeType::BuildUnitCubeType(EUnitCubeType::Grass);
				}
				else
				{
					NewCube->CubeType = UUnitCubeType::BuildUnitCubeType(EUnitCubeType::Stone);
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
			CurrentCube->SetTheCollisionOfTheBoxToBeEnabled(false);
			continue;
		}
		//判断坐标是否为当前渲染区域的边界坐标。

		if (CurrentCube && IsValid(CurrentCube) && CurrentCube->IsSolid()) //检查方块是否存在，且是实体
		{
			int EnabledCollision = 0;
			uint8 MeshType = 0;
			for (const FIntVector& Dir : Directions)
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
				CurrentCube->SetTheCollisionOfTheBoxToBeEnabled(true);
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
		SaveGameInstance->CubesMap.Add(Pair.Key, Pair.Value->CubeType->GetCubeType());
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
			for (const auto& Pair : LoadGameInstance->CubesMap)
			{
				auto NewCube = GetWorld()->SpawnActor<AUnitCube>(AUnitCube::StaticClass(),
				                                                 MapToScene(Pair.Key), FRotator(0.0f));
				NewCube->CubeType = UUnitCubeType::BuildUnitCubeType(static_cast<EUnitCubeType>(Pair.Value));
				//添加后配置自身的可视性
				WorldMap.Add(Pair.Key, NewCube);
			}
			for (const auto& Key : SurfaceCubes)
			{
				UpDateCubeMeshWith(Key);
				TurnOnCubeCollision(Key);
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
	//如果Key存在
	if (CurrentCube && IsValid((*CurrentCube)))
	{
		uint8 MeshType = 0;
		for (const FIntVector& Dir : Directions)
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

void AUnitCubeManager::AddCubeWith(const FVector& Scene, const int& Type)
{
	if (!IsLock)
	{
		IsLock = true;
		FIntVector Key = SceneToMap(Scene);
		auto NewCube = GetWorld()->SpawnActor<AUnitCube>(AUnitCube::StaticClass(),
		                                                 MapToScene(Key), FRotator(0.0f));
		NewCube->CubeType = UUnitCubeType::BuildUnitCubeType(static_cast<EUnitCubeType>(Type));
		//添加后配置自身的可视性
		WorldMap.Add(Key, NewCube);
		SurfaceCubes.Add(Key);
		UpDateCubeMeshWith(Key);
		//检查以刚方块为中心的方块。
		for (const FIntVector& Dir : Directions)
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
		for (const FIntVector& Dir : Directions)
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
		for (const FIntVector& Dir : Directions)
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
		for (const FIntVector& Dir : Directions)
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
		(*Cube)->OnDestroyed();
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
		for (const FIntVector& Dir : Directions)
		{
			MeshManager->DelMeshToCubeWith(Dir, Cube);
		}
	}
}

void AUnitCubeManager::TurnOnCubeCollision(const FIntVector& Key)
{
	AUnitCube** Cube = WorldMap.Find(Key);
	if (Cube)
	{
		(*Cube)->SetTheCollisionOfTheBoxToBeEnabled(true);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("The key location does not exist in the map: (%s)"), *Key.ToString());
	}
}
