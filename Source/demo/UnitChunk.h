// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NoiseBuilder.h"
#include "UObject/NoExportTypes.h"

enum class EUnitCubeType : uint8;
class FUnitCubeType;
/**
 * 
 */
class DEMO_API FUnitChunk:public TSharedFromThis<FUnitChunk>
{
public:
	explicit FUnitChunk(const FIntVector& ChunkPosition);	
	//记录当前区块坐标点的方块类型
	TMap<FIntVector,EUnitCubeType> CubeMap;
	//表面区块
	TSet<FIntVector> SurfaceCubes;
	//区块的坐标值
	FIntVector ChunkPosition = {0,0,0};
	//区块的原点坐标,位于WorldMap的
	FIntVector Origin = {0,0,0};
	//根据当前区块的坐标点获取方块类型
	EUnitCubeType GetUnitCubeType(const FIntVector& MapCoord);
	//根据噪音算法的随机种子值确定方块类型
	void BuildCubesWithNoise(FNoiseBuilder& NoiseBuilder);
	//记录表面方块
	void BuildSurfaceCubes();
	//加载数据
	bool TryLoad();
	static const FIntVector ChunkSize ;
private:
	static bool IsinTheBoundary(const FIntVector& Position);
};

class DEMO_API FUnitChunkManager :public TSharedFromThis<FUnitChunkManager>
{
public:
	FUnitChunkManager();
	explicit FUnitChunkManager(const FIntVector& PlayerPosition,const int32& Seed);
	~FUnitChunkManager();
	
	//记录目前持有的区块
	TMap<FIntVector,TSharedPtr<FUnitChunk>> ChunkMap;
	//噪音器
	TSharedPtr<FNoiseBuilder> NoiseBuilder;
	//当前玩家所处的区块坐标
	FIntVector PlayerPosition;

	//加载指定区块
	void LoadChunkWith(const FIntVector& ChunkPosition);
	//卸载指定区块
	void UnloadChunkWith(const FIntVector& ChunkPosition);
	//根据区块坐标获取区块
	TSharedPtr<FUnitChunk> GetChunkSharedPtr(const FIntVector& ChunkPosition);
};
