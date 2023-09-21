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
	AUnitCubeManager();

protected:
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
	//随机种子值
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Seed")
	int32 WorldSeed;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Top Cubes")
	TSet<FIntVector> SurfaceCubes;
	
	//构建MeshManager
	void BuildMeshManager();
	//构建地图
	void BuildMap();
	void BuildMapWithNoise();//用噪音值来构建地图
	//构建所有方块的静态网格体实例
	void BuildAllCubesMesh();
	//检测方块是否为表层方块
	bool IsSurfaceCube(const FIntVector& Position)const;
	//检测方块是否为边界方块
	bool IsABorderCube(const FIntVector& Position)const;

	//保存地图信息
	UFUNCTION(BlueprintCallable,Category = "Save And Load")
	void SaveWorldMap();
	//加载地图信息
	UFUNCTION(BlueprintCallable,Category = "Save And Load")
	bool LoadWorldMap();
	UFUNCTION(BlueprintCallable,Category = "Save And Load")
	bool LevelSave();
	
	//更新方块面隐藏
	void UpDateCubeMeshWith(const FIntVector& Key);
	void UpDateCubeMeshWith(const AUnitCube* Cube);
	void UpDateAllMesh() const;
	//地图坐标转场景坐标
	static FVector MapToScene(const FIntVector& MapCoord);
	//场景坐标转地图坐标
	static FIntVector SceneToMap(const FVector& Scene);

	//在指定坐标增加方块
	UFUNCTION(BlueprintCallable,Category = "Build Cube")
	void AddCubeWith(const FVector& Scene, const int& Type);
	//在指定坐标删除方块
	UFUNCTION(BlueprintCallable,Category = "Build Cube")
	void DelCubeWith(const FVector& Scene);
	//隐藏某个方块的所有面
	void HiedCubeAllFace(AUnitCube* Cube);
	//开启指定位置方块的碰撞
	void TurnOnCubeCollision(const FIntVector& Key);

	UPROPERTY()
	bool IsLock = false;
	
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
