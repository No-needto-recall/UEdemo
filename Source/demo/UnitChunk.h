// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NoiseBuilder.h"

struct FChunkData;
enum class EUnitCubeType : uint8;
class FUnitCubeType;

enum ETreeLeavesStyle
{
	Surrounded_1,//自身为树叶
	Surrounded_4,//四面为树叶
	Surrounded_5,//四面和自身为树叶
	Surrounded_8,//环绕八面为树叶
	Surrounded_9,//环绕八面和自身为树叶
	Surrounded_12,//环绕八面和突出四面为树叶
};


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
	EUnitCubeType GetUnitCubeType(const FIntVector& CubeMapCoord);
	//根据噪音算法的随机种子值确定方块类型
	void BuildCubesWithNoise(FNoiseBuilder& NoiseBuilder);
	//记录表面方块
	void BuildSurfaceCubes();
	//加载数据
	bool TryLoad();
	//保存数据
	bool TrySave();
	//添加方块
	void AddCubeWith(const FIntVector& CubeMapCoord,const EUnitCubeType& Type);
	void AddCubeWith(const FIntVector& CubeMapCoord,const int& Type);
	//删除方块
	void DelCubeWith(const FIntVector& CubeMapCoord);
	//添加表面方块
	void AddSurfaceCubeWith(const FIntVector& CubeMapCoord);
	//删除表面方块
	void DelSurfaceCubeWith(const FIntVector& CubeMapCoord);
	//获取保存信息
	void GetChunkData(FChunkData& ChunkData) const;
	static const FIntVector ChunkSize ;
	static bool IsinTheBoundary(const FIntVector& Position);
private:	
	//在指定位置生成树
	void BuildTreeWith(const FIntVector& RootLocation, int TreeHeight = 5);
	void BuildTreeLeaves(const FIntVector& Pos, const ETreeLeavesStyle& Style);
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
	void LoadChunkData(const FIntVector& ChunkPosition);
	void LoadChunkData(const FIntVector& ChunkPosition, const FChunkData& ChunkData);
	//卸载指定区块
	void UnloadChunkWith(const FIntVector& ChunkPosition);
	//清理区块
	void CleanAllChunk();
	//根据区块坐标获取区块
	TSharedPtr<FUnitChunk> GetChunkSharedPtr(const FIntVector& ChunkPosition);
};


