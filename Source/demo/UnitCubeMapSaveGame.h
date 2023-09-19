// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnitCubeType.h"
#include "GameFramework/SaveGame.h"
#include "UnitCubeMapSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class DEMO_API UUnitCubeMapSaveGame : public USaveGame
{
	GENERATED_BODY()
public:	
	UPROPERTY(VisibleAnywhere)
	TMap<FIntVector,EUnitCubeType> CubesMap;	
	UPROPERTY(VisibleAnywhere)
	TSet<FIntVector> SurfaceCubes;
	UPROPERTY(VisibleAnywhere)
	int32 WorldSeed;
};
