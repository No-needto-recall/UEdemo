// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldBuildManager.generated.h"

//前置声明
class ABaseCube;

UCLASS()
class DEMO_API AWorldBuildManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldBuildManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "World Map")
	TMap<FIntVector,ABaseCube*> WorldMap;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Size")
	FIntVector Size ;

	void BuildMap();
	void SetCubesHidden();
	FVector MapToScene(const FIntVector& MapCoord);
};
