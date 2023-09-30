// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitChunk.h"

#include "UnitCube.h"
#include "UnitCubeType.h"

const FIntVector FUnitChunk::ChunkSize = {16,16,20};

FUnitChunk::FUnitChunk(const FIntVector& ChunkPosition)
	:ChunkPosition(ChunkPosition)
{
	Origin.X =ChunkSize.X*ChunkPosition.X;
	Origin.Y =ChunkSize.Y*ChunkPosition.Y;
	Origin.Z =ChunkSize.Z*ChunkPosition.Z;
}

EUnitCubeType FUnitChunk::GetUnitCubeType(const FIntVector& CubeMapCoord)
{
	const auto Search = CubeMap.Find(CubeMapCoord);
	if(Search)
	{
		return *Search;
	}else
	{
		UE_LOG(LogTemp,Log,TEXT("can't find MapCoord:%s , in ChunkPosition %s"),*CubeMapCoord.ToString(),*ChunkPosition.ToString());
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
			//基于区块原点的噪音计算
			//保障生成的Max为连续的
			const int MaxZ = NoiseBuilder.GetNumWith(
				Pos.X+Origin.X,
				Pos.Y+Origin.Y,
				0,ChunkSize.Z-1);
			for(Pos.Z = 0;Pos.Z <MaxZ;++Pos.Z)
			{
				if(Pos.Z == 0)//底层为基岩
				{
					CubeMap.Add(Pos,EUnitCubeType::BedRock);	
				}else if(MaxZ - Pos.Z <= 3)//地表以下3格为草方块
				{
					CubeMap.Add(Pos,EUnitCubeType::Grass);	
				}else//其余为石头
				{
					CubeMap.Add(Pos,EUnitCubeType::Stone);	
				}
				//树的生成
			}
		}	
	}	
}

void FUnitChunk::BuildSurfaceCubes()
{
	FIntVector Neighbors[6];
	//筛选表面方块
	for(const auto&Pair:CubeMap)
	{
		FIntVector Position = Pair.Key;
		bool bIsSurface = false;
		//邻居坐标
		Neighbors[0] = FIntVector(Position.X + 1, Position.Y, Position.Z);
		Neighbors[1] = FIntVector(Position.X - 1, Position.Y, Position.Z);
		Neighbors[2] = FIntVector(Position.X, Position.Y + 1, Position.Z);
		Neighbors[3] = FIntVector(Position.X, Position.Y - 1, Position.Z);
		Neighbors[4] = FIntVector(Position.X, Position.Y, Position.Z + 1);
		Neighbors[5] = FIntVector(Position.X, Position.Y, Position.Z - 1);
		//邻近区块前后左右的位置，视作为邻居
		for(int i=0;i<6;++i)
		{
			//如果没有该方块，则该面不存在
			if(!CubeMap.Contains(Neighbors[i]))
			{
				//如果是边界内的位置,说明该方块为表面方块
				if(IsinTheBoundary(Neighbors[i]))
				{
					bIsSurface = true;
					break;
				}
			}
		}
		if (bIsSurface)
		{
			SurfaceCubes.Add(Position);
		}
	}
}

bool FUnitChunk::TryLoad()
{
	//先尝试读取地图文件
	//如果没有则尝试创建
	return false;
}

void FUnitChunk::AddCubeWith(const FIntVector& CubeMapCoord, const EUnitCubeType& Type)
{
	CubeMap.Add(CubeMapCoord,Type);
}

void FUnitChunk::AddCubeWith(const FIntVector& CubeMapCoord, const int& Type)
{
	AddCubeWith(CubeMapCoord,static_cast<EUnitCubeType>(Type));
}

void FUnitChunk::DelCubeWith(const FIntVector& CubeMapCoord)
{
	CubeMap.Remove(CubeMapCoord);
}

bool FUnitChunk::IsinTheBoundary(const FIntVector& Position)
{
	return Position.X >= 0 && Position.X < ChunkSize.X &&
		Position.Y >= 0 && Position.Y < ChunkSize.Y &&
		Position.Z >= 0 && Position.Z < ChunkSize.Z;
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
		//区块先尝试读取区块存档
		if(!Chunk->TryLoad())
		{
			Chunk->BuildCubesWithNoise(*NoiseBuilder);
			Chunk->BuildSurfaceCubes();
		}
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
