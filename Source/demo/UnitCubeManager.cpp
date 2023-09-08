// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitCubeManager.h"

#include "MeshManager.h"
#include "UnitCube.h"

// Sets default values
AUnitCubeManager::AUnitCubeManager()
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
	BuildMap();
	BuildAllCubesMesh();
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
				WorldMap.Add(FIntVector(j, k, i), NewCube);
			}
		}
	}
}

void AUnitCubeManager::BuildAllCubesMesh()
{
	//遍历地图
	for (const auto& Pair : WorldMap)
	{
		FIntVector CurrentPosition = Pair.Key;
		AUnitCube* CurrentCube = Pair.Value;
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
						MeshManager->AddMeshToCubeWith(Dir,static_cast<EFaceMeshType>(MeshType),CurrentCube);
					}
					else
					{
						++EnabledCollision;
					}
				}
				else
				{
					MeshManager->AddMeshToCubeWith(Dir,static_cast<EFaceMeshType>(MeshType),CurrentCube);
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

void AUnitCubeManager::SetCubeHiddenWith(const FIntVector& Key)
{
	AUnitCube** CurrentCube = WorldMap.Find(Key);
	//如果Key存在
	if(CurrentCube && IsValid((*CurrentCube)))
	{
		uint8 MeshType = 0;
		for(const FIntVector& Dir : Directions)
		{
			FIntVector NeighbourPosition = Key + Dir;
			AUnitCube** NeighbourCube= WorldMap.Find(NeighbourPosition);
			if(NeighbourCube && IsValid((*NeighbourCube)))
			{
				//邻居存在
				if((*NeighbourCube)->IsSolid())
				{
					//邻居是实心的
					//设置对应的面不可见
					MeshManager->DelMeshToCubeWith(Dir,static_cast<EFaceMeshType>(MeshType),(*CurrentCube));
				}else
				{
					MeshManager->AddMeshToCubeWith(Dir,static_cast<EFaceMeshType>(MeshType),(*CurrentCube));
				}
			}else
			{
				//邻居不存在
				MeshManager->AddMeshToCubeWith(Dir,static_cast<EFaceMeshType>(MeshType),(*CurrentCube));
			}
			++MeshType;
		}
	}else
	{
		UE_LOG(LogTemp, Log, TEXT("The location does not exist in the map: (%s)"), *Key.ToString());
	}
}

void AUnitCubeManager::SetCubeHiddenWith(AUnitCube* Cube)
{
	if (Cube && IsValid(Cube))
	{
		const FIntVector Key = SceneToMap(Cube->GetActorLocation());
		SetCubeHiddenWith(Key);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("This Actor does not exist"));
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

void AUnitCubeManager::AddCubeWith(const FVector& Scene)
{
	FIntVector Key = SceneToMap(Scene);
	auto NewCube = GetWorld()->SpawnActor<AUnitCube>(AUnitCube::StaticClass(),
													 MapToScene(Key), FRotator(0.0f));
	//添加后配置自身的可视性
	WorldMap.Add(Key, NewCube);
	SetCubeHiddenWith(Key);
	//检查以刚方块为中心的方块。
	for (const FIntVector& Dir : Directions)
	{
		FIntVector NeighbourPosition = Key + Dir;
		AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
		if (NeighbourCube)
		{
			//如果邻居存在，更新邻居的隐藏配置
			SetCubeHiddenWith(NeighbourPosition);
			//更新邻居的碰撞
			(*NeighbourCube)->RefreshCollisionEnabled();
		}
	}
}

void AUnitCubeManager::DelCubeWith(const FVector& Scene)
{
	FIntVector Key = SceneToMap(Scene);
	AUnitCube** Cube = WorldMap.Find(Key);
	if(Cube)//如果有找到的
		{
		//移除
		WorldMap.Remove(Key);
		//刷新周围
		for(const FIntVector& Dir : Directions)
		{
			FIntVector NeighbourPosition = Key +Dir;
			AUnitCube** NeighbourCube= WorldMap.Find(NeighbourPosition);
			if(NeighbourCube)
			{
				//如果邻居存在，更新隐藏配置
				SetCubeHiddenWith(NeighbourPosition);
				//更新邻居的碰撞
				(*NeighbourCube)->RefreshCollisionEnabled();
			}	
		}
		//Cube销毁
		(*Cube)->OnDestroyed();
		//刷新遮挡
		FlushRenderingCommands();
		}else
		{
			//Key不存在：
			UE_LOG(LogTemp, Log, TEXT("The location does not exist in the map: (%s)"), *Key.ToString());
		}
}
