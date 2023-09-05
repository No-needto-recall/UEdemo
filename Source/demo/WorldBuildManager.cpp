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
	
	for(int i = 0 ;i<Size.Z;++i)
	{
		for(int k=0;k<Size.Y;++k)
		{
			for(int j=0;j<Size.X;++j)
			{
				NewCube = GetWorld()->SpawnActor<ABaseCube>(ABaseCube::StaticClass(),
					MapToScene(FIntVector(j,k,i)),FRotator(0.0f));
				WorldMap.Add(FIntVector(j,k,i),NewCube);
			}	
		}
	}
}

void AWorldBuildManager::SetCubesHidden()
{
	TArray<FIntVector> Directions = {
		FIntVector(1, 0, 0),  // X+
		FIntVector(-1, 0, 0), // X-
		FIntVector(0, 1, 0),  // Y+
		FIntVector(0, -1, 0), // Y-
		FIntVector(0, 0, 1),  // Z+
		FIntVector(0, 0, -1)  // Z-
	};

	for(auto& Pair : WorldMap)
	{
		FIntVector CurrentPosition = Pair.Key;
		ABaseCube* CurrentCube = Pair.Value;
    
		if(CurrentCube && CurrentCube->IsSolid()) // 检查方块是否存在并且是实体的
			{
			for(const FIntVector& Dir : Directions)
			{
				FIntVector NeighbourPosition = CurrentPosition + Dir;
				ABaseCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
            
				if(NeighbourCube && (*NeighbourCube)->IsSolid()) // 如果邻居存在并且是实体的
					{
					// 设置CurrentCube的对应面为不可见。你需要实现这个函数或者有一个方法来做到这一点。
					CurrentCube->SetFaceVisibility(Dir,false);
					}
			}
			}
	}

}

FVector AWorldBuildManager::MapToScene(const FIntVector& MapCoord)
{
	return FVector(100*MapCoord.X,100*MapCoord.Y,100*MapCoord.Z);
}

