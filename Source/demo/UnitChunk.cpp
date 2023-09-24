// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitChunk.h"

#include "UnitCube.h"
#include "UnitCubeType.h"

const FIntVector FUUnitChunk::ChunkSize = {16,16,20};

FUUnitChunk::FUUnitChunk(const FIntVector& ChunkPosition)
	:ChunkPosition(ChunkPosition)
{
	Origin.X =ChunkSize.X*AUnitCube::CubeSize.X*ChunkPosition.X;
	Origin.Y =ChunkSize.Y*AUnitCube::CubeSize.Y*ChunkPosition.Y;
	Origin.Z =ChunkSize.Z*AUnitCube::CubeSize.Z*ChunkPosition.Z;
}

EUnitCubeType FUUnitChunk::GetUnitCubeType(const FIntVector& MapCoord)
{
	const auto Search = CubeMap.Find(MapCoord);
	if(Search)
	{
		return *Search;
	}else
	{
		UE_LOG(LogTemp,Log,TEXT("can't find MapCoord:%s , in ChunkPosition %s"),*MapCoord.ToString(),*ChunkPosition.ToString());
		return EUnitCubeType::BedRock;
	}
}

void FUUnitChunk::BuildCubesWithNoise(FNoiseBuilder& NoiseBuilder)
{
	FIntVector Pos(0, 0, 0);
	for (Pos.X = 0; Pos.X < ChunkSize.X; ++Pos.X)
	{
		for (Pos.Y = 0; Pos.Y < ChunkSize.Y; ++Pos.Y)
		{
			const int MaxZ = NoiseBuilder.GetNumWith(Pos.X,Pos.Y,-ChunkSize.Z,+ChunkSize.Z);
			for(Pos.Z = 0;Pos.Z <MaxZ;++Pos.Z)
			{
				if(Pos.Z == 0)//底层为基岩
				{
					CubeMap.Add(Pos,EUnitCubeType::BedRock);	
				}else if(MaxZ - Pos.Z <= 3)//地表以下3格为草方块
				{
					CubeMap.Add(Pos,EUnitCubeType::Grass);	
				}else
				{
					CubeMap.Add(Pos,EUnitCubeType::Stone);	
				}
				//树的生成
			}
		}	
	}	
}

FUUnitChunkManager::FUUnitChunkManager()
	:FUUnitChunkManager({0,0,0},0)
{
}

FUUnitChunkManager::FUUnitChunkManager(const FIntVector& PlayerPosition, const int32& Seed)
{
	NoiseBuilder = MakeShareable(new FNoiseBuilder(Seed));
}

FUUnitChunkManager::~FUUnitChunkManager()
{
}

void FUUnitChunkManager::LoadChunkWith(const FIntVector& ChunkPosition)
{
	//如果不存在，则需要加载
	if(!ChunkMap.Contains(ChunkPosition))
	{
		TSharedPtr<FUUnitChunk> Chunk = MakeShareable(new FUUnitChunk(ChunkPosition));
		Chunk->BuildCubesWithNoise(*NoiseBuilder);
		ChunkMap.Add(ChunkPosition,Chunk);
	}
}

void FUUnitChunkManager::UnloadChunkWith(const FIntVector& ChunkPosition)
{
	//如果存在，则卸载
	if(ChunkMap.Contains(ChunkPosition))
	{
		//存在保存机制
		//1.后台线程，存储一个“代保存的”队列
		//2.标记状体，避免该区块在缓存中被加载
	}
}

FIntVector FUUnitChunkManager::GetChunkPosition(const FVector& Scene)
{
	return FIntVector::ZeroValue;
}

TSharedPtr<FUUnitChunk> FUUnitChunkManager::GetChunkSharedPtr(const FIntVector& ChunkPosition)
{
	return nullptr;
}
