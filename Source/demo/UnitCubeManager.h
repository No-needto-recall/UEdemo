// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitCubeManager.generated.h"

//前置声明
class AUnitCube;
class AMeshManager;

UCLASS()
class DEMO_API AUnitCubeManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUnitCubeManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//方块地图
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "World Map")
	TMap<FIntVector,AUnitCube*> WorldMap;
	//地图大小
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Size")
	FIntVector Size ;
	//网格体管理
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Mesh Manager")
	AMeshManager* MeshManager;
	//构建MeshManager
	void BuildMeshManager();
	//构建地图
	void BuildMap();
	//初始化方块隐藏
	void InitAllCubesHide();
	//设置方块面隐藏
	void SetCubeHiddenWith(const FIntVector& Key);
	void SetCubeHiddenWith(AUnitCube* Cube);
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
		FIntVector(0, 0, 1),  // Z+ Top
		FIntVector(0, 0, -1),  // Z- Bottom
		FIntVector(1, 0, 0),  // X+ Front
		FIntVector(-1, 0, 0), // X- Back
		FIntVector(0, 1, 0),  // Y+ Right
		FIntVector(0, -1, 0) // Y- Left
};
};
