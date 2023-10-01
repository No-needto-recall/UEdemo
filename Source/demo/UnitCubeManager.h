// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitCubeManager.generated.h"

class FChunkLoaderRunnable;
class FUnitCubeTypeManager;
class FUnitChunkManager;
class UUnitCubePool;
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//方块地图
	UPROPERTY()
	TMap<FIntVector,AUnitCube*> WorldMap;
	//表面方块集
	TSet<FIntVector> SurfaceCubes;
	//已分配资源的区块
	TMap<FIntVector,bool> AllocationChunks;
	//地图大小
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Size")
	FIntVector Size ;
	//网格体管理
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Mesh Manager")
	AMeshManager* MeshManager;
	//随机种子值
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Seed")
	int32 WorldSeed;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "LoadDistance", meta = (ClampMin = "1", ClampMax = "10"))
	int32 LoadDistance = 1;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "UnloadDistance", meta = (ClampMin = "1", ClampMax = "10"))
	int32 UnloadDistance = 2;

	//均在构造函数中初始化
	//区块管理
	TSharedPtr<FUnitChunkManager> ChunkManager;
	//Cube的类型原型
	TSharedPtr<FUnitCubeTypeManager> CubeTypeManager;
	//后台线程
	FChunkLoaderRunnable* Runnable;
	FRunnableThread* Thread;
	

#if 1
	//新建世界
	void BuildNewWorld();
	//加载区块
	void LoadChunkAroundPlayer(const int& AroundDistance = 1);
	void LoadChunkAll(const FIntVector& ChunkPosition);
	void LoadCubeAndCubeTypeWith(const FIntVector& ChunkPosition);
	void LoadCubeMeshWith(const FIntVector& ChunkPosition);
	//卸载方块
	void UnloadChunkNotAroundPlayer(const int& AroundDistance = 1);
	void UnloadChunk(const FIntVector& ChunkPosition);
	//判断Chunk是否为玩家周围
	bool IsAroundPlayer(const FIntVector& ChunkPosition, const int& AroundDistance = 1) const;
	void ReturnUnitCubeToPool(const FIntVector& CubeInWorldMap);
	//跟新玩家位置信息
	UFUNCTION(BlueprintCallable,Category = "Update")
	void UpdateThePlayerChunkLocation(AActor* Player);
	UFUNCTION(BlueprintCallable,Category = "Update")
	void SynchronizePlayerPositions(AActor* Player);
	//原子布尔
	std::atomic<bool> BIsRunning{false};
	//声明一个没有参数的事件分发器
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventSignature);
	//添加UPROPERTY宏
	UPROPERTY(BlueprintAssignable,Category = "Category")
	FEventSignature OnLoadChunkComplete;
	
#endif
	
	//构建MeshManager
	void BuildMeshManager();
	//构建pool池
	void BuildUnitCubePool();

	//保存地图信息
	UFUNCTION(BlueprintCallable,Category = "Save And Load")
	void SaveWorldMap();
	//加载地图信息
	UFUNCTION(BlueprintCallable,Category = "Save And Load")
	bool LoadWorldMap();
	
	//更新方块面隐藏
	void UpDateCubeMeshWith(const FIntVector& Key);
	void UpDateCubeMeshWith(const AUnitCube* Cube);
	void UpDateAllMesh() const;
#if 1
	static FIntVector UEToWorldMap(const FVector& UECoord);
	static FIntVector UEToChunkMap(const FVector& UECoord);
	static FVector WorldMapToUE(const FIntVector& WorldMapCoord);
	static FIntVector WorldMapToChunkMap(const FIntVector& WorldMapCoord);
	static FIntVector ChunkMapToWorldMap(const FIntVector& ChunkMapCoord);
	static FVector ChunkMapToUE(const FIntVector& ChunkMapCoord);
	static FIntVector WorldMapToCubeMap(const FIntVector& WorldMapCoord);
#endif

	//在指定坐标增加方块
	UFUNCTION(BlueprintCallable,Category = "Build Cube")
	void AddCubeWith(const FVector& Scene, const int& Type);
	//在指定坐标删除方块
	UFUNCTION(BlueprintCallable,Category = "Build Cube")
	void DelCubeWith(const FVector& Scene);
	std::atomic<bool> BIsAdding{false};
	//隐藏某个方块的所有面
	void HiedCubeAllFace(AUnitCube* Cube);
	void HiedCubeAllFace(const FIntVector& WorldMapPosition);
	//开启指定位置方块的碰撞
	void SetCubeCollision(const FIntVector& Key, bool IsTurnOn);

	
	TArray<FIntVector> DirectionsForCube = {
		FIntVector(0, 0, 1),  // Z+ Top
		FIntVector(0, 0, -1),  // Z- Bottom
		FIntVector(1, 0, 0),  // X+ Front
		FIntVector(-1, 0, 0), // X- Back
		FIntVector(0, 1, 0),  // Y+ Right
		FIntVector(0, -1, 0) // Y- Left
};
	TArray<FIntVector> DirectionsForChunk = {
		{0, 0, 0}, //原点
		{1, 0, 0}, //前
		{-1, 0, 0}, //后
		{0, 1, 0}, //左
		{0, -1, 0}, //右
		{1, 1, 0}, //前左
		{1, -1, 0}, //前右
		{-1, 1, 0}, //后左
		{-1, -1, 0} //后右
	};
private:
	//Cube的对象池
	UPROPERTY()
	UUnitCubePool* CubePool;
};


//后台加载线程
class FChunkLoaderRunnable final : public FRunnable
{
private:
	FCriticalSection CriticalSection;//用于线程安全的锁
	AUnitCubeManager* Manager; //指向 AUnitCubeManager 的指针
	FEvent* EventTrigger;//事件
	volatile bool bShouldRun; // 控制线程何时停止
public:
	explicit FChunkLoaderRunnable(AUnitCubeManager* InManager);
	void TriggerExecution();
	virtual void Stop() override;
	virtual uint32 Run() override;
};