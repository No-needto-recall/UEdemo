// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldBuildManager.h"
#include "BaseCube.h"

// Sets default values
AWorldBuildManager::AWorldBuildManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AWorldBuildManager::BeginPlay()
{
	Super::BeginPlay();
	BuildMap();
	SetCubesHidden();
}

// Called every frame
void AWorldBuildManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWorldBuildManager::BuildMap()
{
	ABaseCube* NewCube = nullptr;

	for (int i = 0; i < Size.Z; ++i)
	{
		for (int k = 0; k < Size.Y; ++k)
		{
			for (int j = 0; j < Size.X; ++j)
			{
				NewCube = GetWorld()->SpawnActor<ABaseCube>(ABaseCube::StaticClass(),
				                                            MapToScene(FIntVector(j, k, i)), FRotator(0.0f));
				WorldMap.Add(FIntVector(j, k, i), NewCube);
			}
		}
	}
}

void AWorldBuildManager::SetCubesHidden()
{
	for (const auto& Pair : WorldMap)
	{
		FIntVector CurrentPosition = Pair.Key;
		ABaseCube* CurrentCube = Pair.Value;

		if (CurrentCube && CurrentCube->IsSolid()) // 检查方块是否存在并且是实体的
		{
			int  EnabledCollision = 0; 
			for (const FIntVector& Dir : Directions)
			{
				FIntVector NeighbourPosition = CurrentPosition + Dir;
				ABaseCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
				//因为默认所有面都是可见的，所以只需要检测邻居为实心
				if (NeighbourCube && (*NeighbourCube)->IsSolid()) // 如果邻居存在并且是实体的
				{
					// 设置CurrentCube的对应面为不可见。
					CurrentCube->SetFaceVisibility(Dir, false);
					++EnabledCollision;
				}
			}
			//循环结束后，如果方块被实心体包围，则取消碰撞
			if(EnabledCollision == 6)
			{
				CurrentCube->SetTheCollisionOfTheBoxToBeEnabled(false);
			}else//否则开启碰撞
			{
				CurrentCube->SetTheCollisionOfTheBoxToBeEnabled(true);
			}
		}
	}
}

void AWorldBuildManager::SetCubeHiddenWith(const FIntVector& Key)
{
	ABaseCube** CurrentCube = WorldMap.Find(Key);
	//如果Key存在
	if (CurrentCube)
	{
		for (const FIntVector& Dir : Directions)
		{
			FIntVector NeighbourPosition = Key + Dir;
			ABaseCube** NeighbourCube = WorldMap.Find(NeighbourPosition);

			if (NeighbourCube) //邻居存在
			{
				if ((*NeighbourCube)->IsSolid())//是实心的
				{
					//设置对应的面不可见
					(*CurrentCube)->SetFaceVisibility(Dir, false);
				}else
				{
					//设置对应的面可见
					(*CurrentCube)->SetFaceVisibility(Dir,true);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("The location does not exist in the map: (%s)"), *Key.ToString());
	}
}

void AWorldBuildManager::SetCubeHiddenWith(ABaseCube* Cube)
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

FVector AWorldBuildManager::MapToScene(const FIntVector& MapCoord)
{
	return FVector(100 * MapCoord.X, 100 * MapCoord.Y, 100 * MapCoord.Z);
}

FIntVector AWorldBuildManager::SceneToMap(const FVector& Scene)
{
	return FIntVector(Scene.X / 100, Scene.Y / 100, Scene.Z / 100);
}

void AWorldBuildManager::AddCubeWith(const FVector& Scene)
{
	FIntVector Key = SceneToMap(Scene);
	auto NewCube = GetWorld()->SpawnActor<ABaseCube>(ABaseCube::StaticClass(),
	                                                 MapToScene(Key), FRotator(0.0f));
	//添加后配置自身的可视性
	WorldMap.Add(Key, NewCube);
	SetCubeHiddenWith(Key);
	//检查以刚方块为中心的方块。
	for (const FIntVector& Dir : Directions)
	{
		FIntVector NeighbourPosition = Key + Dir;
		ABaseCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
		if (NeighbourCube)
		{
			//如果邻居存在，更新邻居的隐藏配置
			SetCubeHiddenWith(NeighbourPosition);
			//更新邻居的碰撞
			(*NeighbourCube)->RefreshCollisionEnabled();
		}
	}
}
