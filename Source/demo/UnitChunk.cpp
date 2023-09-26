// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitChunk.h"

#include "UnitCube.h"
#include "UnitCubeType.h"

const FIntVector FUnitChunk::ChunkSize = {16,16,20};

FUnitChunk::FUnitChunk(const FIntVector& ChunkPosition)
	:ChunkPosition(ChunkPosition)
{
	Origin.X =ChunkSize.X*AUnitCube::CubeSize.X*ChunkPosition.X;
	Origin.Y =ChunkSize.Y*AUnitCube::CubeSize.Y*ChunkPosition.Y;
	Origin.Z =ChunkSize.Z*AUnitCube::CubeSize.Z*ChunkPosition.Z;
}

EUnitCubeType FUnitChunk::GetUnitCubeType(const FIntVector& MapCoord)
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

void FUnitChunk::BuildCubesWithNoise(FNoiseBuilder& NoiseBuilder)
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
				}else//其余为基岩
				{
					CubeMap.Add(Pos,EUnitCubeType::Stone);	
				}
				//树的生成
			}
		}	
	}	
}

FUnitChunkManager::FUnitChunkManager()
	:FUnitChunkManager({0,0,0},0)
{
}

FUnitChunkManager::FUnitChunkManager(const FIntVector& PlayerPosition, const int32& Seed)
{
	NoiseBuilder = MakeShareable(new FNoiseBuilder(Seed));
}

FUnitChunkManager::~FUnitChunkManager()
{
}

void FUnitChunkManager::LoadChunkWith(const FIntVector& ChunkPosition)
{
	//如果不存在，则需要加载
	if(!ChunkMap.Contains(ChunkPosition))
	{
		TSharedPtr<FUnitChunk> Chunk = MakeShareable(new FUnitChunk(ChunkPosition));
		Chunk->BuildCubesWithNoise(*NoiseBuilder);
		ChunkMap.Add(ChunkPosition,Chunk);
	}
}

void FUnitChunkManager::UnloadChunkWith(const FIntVector& ChunkPosition)
{
	//如果存在，则卸载
	if(ChunkMap.Contains(ChunkPosition))
	{
		//存在保存机制
		//1.后台线程，存储一个“代保存的”队列
		//2.标记状体，避免该区块在缓存中被加载
	}
}

FIntVector FUnitChunkManager::GetChunkPosition(const FVector& Scene)
{
	FIntVector ChunkPosition = FIntVector::ZeroValue;
	ChunkPosition.X = Scene.X / AUnitCube::CubeSize.X / FUnitChunk::ChunkSize.X;
	ChunkPosition.Y = Scene.Y / AUnitCube::CubeSize.Y / FUnitChunk::ChunkSize.Y;
	ChunkPosition.Z = Scene.Z / AUnitCube::CubeSize.Z / FUnitChunk::ChunkSize.Z;
	return ChunkPosition;
}

TSharedPtr<FUnitChunk> FUnitChunkManager::GetChunkSharedPtr(const FIntVector& ChunkPosition)
{
	auto Search = ChunkMap.Find(ChunkPosition);
	if(Search)
	{
		return *Search;
	}else
	{
		UE_LOG(LogTemp,Log,TEXT("can't find Chunk with ChunkPosition %s"),*ChunkPosition.ToString());
		return nullptr;
	}
}
