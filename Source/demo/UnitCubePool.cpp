// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitCubePool.h"

#include <iterator>
#include <algorithm>
#include "UnitCube.h"

void UUnitCubePool::InitializeUnitCubePool(UWorld* InWorld, const int32 Size)
{
	PoolSize = Size;
	World = InWorld;
	if (World)
	{
		for (int32 i = 0; i < PoolSize; ++i)
		{
			AUnitCube* NewCube = World->SpawnActor<AUnitCube>(AUnitCube::StaticClass(), FVector::ZeroVector,
			                                                  FRotator::ZeroRotator);
			TurnOffCube(NewCube);
			UnUsedPool.Add(NewCube);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetWorld() return nullptr !"));
	}
}

void UUnitCubePool::ExpandUnitCubePool()
{
	if (World)
	{
		//扩容的时候，仅仅扩容预设大小的一半
		for (int32 i = 0; i < PoolSize/2; ++i)
		{
			AUnitCube* NewCube = World->SpawnActor<AUnitCube>(AUnitCube::StaticClass(), FVector::ZeroVector,
			                                                  FRotator::ZeroRotator);
			TurnOffCube(NewCube);
			UnUsedPool.Add(NewCube);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetWorld() return nullptr !"));
	}
}

void UUnitCubePool::TurnOffCube(AUnitCube* Cube)
{
	if (Cube)
	{
		Cube->SetTheCollisionOfTheBoxToBeEnabled(false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cube == nullptr"));
	}
}

void UUnitCubePool::TurnOnCube(AUnitCube* Cube)
{	if (Cube)
 	{
 		//Cube->SetTheCollisionOfTheBoxToBeEnabled(true);
 	}
 	else
 	{
 		UE_LOG(LogTemp, Warning, TEXT("Cube == nullptr"));
 	}
}


AUnitCube* UUnitCubePool::GetUnitCube()
{
	if (UnUsedPool.Num() > 0)
	{
		AUnitCube* Cube = UnUsedPool.Pop();
		UsedPool.Add(Cube);
		TurnOnCube(Cube);
		return Cube;
	}
	else
	{
		//如果没有可用的对象
		ExpandUnitCubePool();
		if (UnUsedPool.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("ExpandUnitCubePool Failed!"));
			return nullptr;
		}
		else
		{
			AUnitCube* Cube = UnUsedPool.Pop();
			UsedPool.Add(Cube);
			TurnOnCube(Cube);
			return Cube;
		}
	}
}

void UUnitCubePool::ReturnObject(AUnitCube* Cube)
{
	if (UsedPool.Contains(Cube))
	{
		UsedPool.Remove(Cube);
		TurnOffCube(Cube);
		UnUsedPool.Add(Cube);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("The pointer does not exist in the pool"))
	}
}
