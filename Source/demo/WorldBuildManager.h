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

	//构建地图
	void BuildMap();
	//构建所有方块的静态网格体实例
	void BuildAllCubesMesh();
	//设置方块面隐藏
	void SetCubeHiddenWith(const FIntVector& Key);
	void SetCubeHiddenWith(ABaseCube* Cube);
	//地图坐标转场景坐标
	static FVector MapToScene(const FIntVector& MapCoord);
	//场景坐标转地图坐标
	static FIntVector SceneToMap(const FVector& Scene);

	//在指定坐标增加方块
	UFUNCTION(BlueprintCallable,Category = "Build Cube")
	void AddCubeWith(const FVector& Scene);
	//在指定坐标删除方块
	UFUNCTION(BlueprintCallable,Category = "Build Cube")
	void DelCubeWith(const FVector& Scene);
	
	UPROPERTY()
	TArray<FIntVector> Directions = {
    		FIntVector(1, 0, 0),  // X+
    		FIntVector(-1, 0, 0), // X-
    		FIntVector(0, 1, 0),  // Y+
    		FIntVector(0, -1, 0), // Y-
    		FIntVector(0, 0, 1),  // Z+
    		FIntVector(0, 0, -1)  // Z-
	};
};
