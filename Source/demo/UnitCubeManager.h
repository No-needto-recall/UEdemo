// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>

#include "CoreMinimal.h"
#include "FChunkStatus.h"
#include "GameFramework/Actor.h"
#include "UnitCubeManager.generated.h"

//前置声明
class FChunkLoaderRunnable;
class FUnitCubeTypeManager;
class FUnitChunkManager;
class UUnitCubePool;
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
	//分配资源的区块
	TMap<FIntVector,TSharedPtr<FChunkStatus>> AllocationChunks;
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
	
	//分帧处理
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoadChunkTask Num", meta = (ClampMin = "1"))
	int32 TaskNum1 = 1;//和线程池紧密相关
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoadChunkTask Num", meta = (ClampMin = "1"))
	int32 TaskNum2 = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoadChunkTask Num", meta = (ClampMin = "1"))
	int32 TaskNum3 = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoadChunkTask Num", meta = (ClampMin = "1"))
	int32 TaskNum4 = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoadChunkTask Num", meta = (ClampMin = "1"))
	int32 TaskNum5 = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoadChunkTask Num", meta = (ClampMin = "1"))
	float TimeThreshold= 5.0f;
	//计时器
	float NoLoadTaskTimer = 0.0f;

	TQueue<FIntVector> LoadChunkTask_PrepareData;
	TQueue<FIntVector> LoadChunkTask_AllocateResources;
	TQueue<FIntVector> LoadChunkTask_AllocateMesh;
	TQueue<FIntVector> UnloadChunkTask_AllocateResources;
	TQueue<FIntVector> UnloadChunkTask_AllocateMesh;
	//多线程
	FQueuedThreadPool* ChunkLoaderThreadPool;
	
	void ProcessTaskQueue(TQueue<FIntVector>& TaskQueue,int NumTasks, const std::function<void(FIntVector)>& TaskFunction);
	void ExecuteLoadChunkTask(const int& N1,const int& N2,const int& N3);
	void ExecuteUnloadTask(const int& N1, const int& N2);
	bool IsNotHasLoadTask() const;
	bool IsNotHasUnloadTask()const;
	
	FCriticalSection LockForLoadChunkData;
	void SafeThread_LoadChunkData(const FIntVector& Task);
	void SafeThread_LoadChunkTaskAllocateResources_Enqueue(const FIntVector& Task);
	bool SafeThread_LoadChunkTaskAllocateResources_IsEmpty() const;
	bool SafeThread_LoadChunkTaskAllocateResources_Dequeue(FIntVector& Task);
	TSharedPtr<FChunkStatus> GetChunkLoadState(const FIntVector& Task);
	bool IsChunkReady(const FIntVector& ChunkPosition);

#if 1
	//新建世界
	void BuildNewWorld();
	//加载区块
	void LoadChunkAroundPlayer(const int& AroundDistance = 1, bool bWithTick = true);
	void LoadChunkAll(const FIntVector& ChunkPosition, bool bWithTick = true);
	void LoadCubeAndCubeTypeWith(const FIntVector& ChunkPosition);
	void LoadCubeMeshWith(const FIntVector& ChunkPosition);
	//卸载方块
	void UnloadChunkNotAroundPlayer(const int& AroundDistance = 1, bool bWithTick = true);
	void UnloadChunkAll(const FIntVector& ChunkPosition, bool bWithTick);
	void UnloadCubeMeshWith(const FIntVector& ChunkPosition);
	void UnloadCubeAndCubeTypeWith(const FIntVector& ChunkPosition);
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

//线程池任务
// FChunkLoaderTask 类用于执行加载和卸载块的任务
class FChunkLoaderTask final : public IQueuedWork
{
private:
	AUnitCubeManager* Manager;
	FIntVector ChunkPosition;

public:
	FChunkLoaderTask(AUnitCubeManager* InManager, const FIntVector& InChunkPosition)
		: Manager(InManager), ChunkPosition(InChunkPosition)
	{
	}

	virtual void DoThreadedWork() override
	{
		// 实际的工作
		if (Manager)
		{
			Manager->SafeThread_LoadChunkData(ChunkPosition);
			Manager->SafeThread_LoadChunkTaskAllocateResources_Enqueue(ChunkPosition);
		}
		// 在完成工作后，记得手动删除这个任务实例
		delete this;
	}

	virtual void Abandon() override
	{
		// 这里处理任务被中断的情况
		delete this;
	}
};

