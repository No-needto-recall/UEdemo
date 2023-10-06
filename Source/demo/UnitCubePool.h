// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UnitCubePool.generated.h"

class AUnitCube;
/**
 * 
 */
UCLASS()
class DEMO_API UUnitCubePool : public UObject
{
	GENERATED_BODY()
private:
	//存储未使用的对象
	UPROPERTY()
	TArray<AUnitCube*> UnUsedPool;
	//存储正在使用的对象
	UPROPERTY()
	TSet<AUnitCube*> UsedPool;
	//世界上下文
	UPROPERTY()
	UWorld* World;
	//PoolSize
	int32 PoolSize = 100;
	//扩容对象池
	void ExpandUnitCubePool();
	//关闭Cube对象
	void TurnOffCube(AUnitCube* Cube);
	//开启Cube对象
	void TurnOnCube(AUnitCube* Cube);
public:
	//初始化对象池
	void InitializeUnitCubePool(UWorld* InWorld, const int32 Size);
	
	//清理池的标记
	void CleanAllCube();
	//获取和归还对象
	AUnitCube* GetUnitCube();
	void ReturnObject(AUnitCube* Cube);
};
