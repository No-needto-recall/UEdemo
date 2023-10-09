// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnitCubeType.h"
#include "GameFramework/SaveGame.h"
#include "UnitCubeMapSaveGame.generated.h"

USTRUCT()
struct FChunkData 
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere)
	TMap<FIntVector,EUnitCubeType> CubeMap;
	UPROPERTY(VisibleAnywhere)
	TSet<FIntVector> SurfaceCubes;
};


UCLASS()
class DEMO_API UUnitCubeMapSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere)
	TMap<FIntVector,FChunkData> ChunkMap;
	UPROPERTY(VisibleAnywhere)
	int32 WorldSeed;
	UPROPERTY(VisibleAnywhere)
	FIntVector PlayerChunkPosition;
	UPROPERTY(VisibleAnywhere)
	FVector PlayerLocationInUE;
	
};
