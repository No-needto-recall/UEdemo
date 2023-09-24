// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NoiseBuilder.h"
#include "UObject/NoExportTypes.h"

enum class EUnitCubeType : uint8;
class UUnitCubeType;
/**
 * 
 */
class DEMO_API FUUnitChunk:public TSharedFromThis<FUUnitChunk>
{
public:
	explicit FUUnitChunk(const FIntVector& ChunkPosition);	
	//记录当前区块坐标点的方块类型
	TMap<FIntVector,EUnitCubeType> CubeMap;
	//表面区块
	TSet<FIntVector> SurfaceCubes;
	//区块的坐标值
	FIntVector ChunkPosition = {0,0,0};
	//区块的原点坐标
	FVector Origin = {0,0,0};
	//根据当前区块的坐标点获取方块类型
	EUnitCubeType GetUnitCubeType(const FIntVector& MapCoord);
	//根据噪音算法的随机种子值确定方块类型
	void BuildCubesWithNoise(FNoiseBuilder& NoiseBuilder);

	static const FIntVector ChunkSize ;
};

class DEMO_API FUUnitChunkManager :public TSharedFromThis<FUUnitChunkManager>
{
public:
	FUUnitChunkManager();
	explicit FUUnitChunkManager(const FIntVector& PlayerPosition,const int32& Seed);
	~FUUnitChunkManager();
	
	//记录目前持有的区块
	TMap<FIntVector,TSharedPtr<FUUnitChunk>> ChunkMap;
	//噪音器
	TSharedPtr<FNoiseBuilder> NoiseBuilder;
	//当前玩家所处的区块坐标
	FIntVector PlayerPosition;

	//加载指定区块
	void LoadChunkWith(const FIntVector& ChunkPosition);
	//卸载指定区块
	void UnloadChunkWith(const FIntVector& ChunkPosition);
	//根据世界坐标获得的区块坐标
	FIntVector GetChunkPosition(const FVector& Scene);
	//根据区块坐标获取区块
	TSharedPtr<FUUnitChunk> GetChunkSharedPtr(const FIntVector& ChunkPosition);
};
