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
	InitAllCubesHide();
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

void AUnitCubeManager::InitAllCubesHide()
{
	for (const auto& Pair : WorldMap)
	{
		FIntVector CurrentPosition = Pair.Key;
		AUnitCube* CurrentCube = Pair.Value;
		if (CurrentCube && IsValid(CurrentCube) && CurrentCube->IsSolid()) //检查方块是否存在，且是实体
		{
			uint8 MeshType = 0;
			for (const FIntVector& Dir : Directions)
			{
				FIntVector NeighbourPosition = CurrentPosition + Dir;
				AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
				//因为所有cube默认没有静态网格体实例，所以检测存在不为实心||不存在
				if (NeighbourCube)
				{
					if ((*NeighbourCube)->IsTransparent())
					{
						//在对应的Dir添加静态网格体实例
						MeshManager->ShowCubeFaceWith(Dir,
						                              static_cast<EFaceMeshType>(MeshType),
						                              CurrentCube);
					}
				}
				else
				{
					MeshManager->ShowCubeFaceWith(Dir,
					                              static_cast<EFaceMeshType>(MeshType),
					                              CurrentCube);
				}
				++MeshType;
			}
		}
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
