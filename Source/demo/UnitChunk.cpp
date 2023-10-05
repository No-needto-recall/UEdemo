// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitChunk.h"

#include "MyCustomLog.h"
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
		CUSTOM_LOG_INFO(TEXT("can't find MapCoord:%s , in ChunkPosition %s"),*CubeMapCoord.ToString(),*ChunkPosition.ToString());
		return EUnitCubeType::BedRock;
	}
}

void FUnitChunk::BuildCubesWithNoise(FNoiseBuilder& NoiseBuilder)
{
	FIntVector Pos(0, 0, 0);
	TSharedPtr<FNoise> TerrainNoise = NoiseBuilder.GetFNoise(ENoiseLiteType::Terrain);
	TSharedPtr<FNoise> TreeNoise = NoiseBuilder.GetFNoise(ENoiseLiteType::Tree);
	for (Pos.X = 0; Pos.X < ChunkSize.X; ++Pos.X)
	{
		for (Pos.Y = 0; Pos.Y < ChunkSize.Y; ++Pos.Y)
		{
			//基于区块原点的噪音计算
			//保障生成的Max为连续的
			const int MaxZ = TerrainNoise->GetNumWith(
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
			}
			//树的生成
			if(Pos.X >=3 && Pos.X<=12 && Pos.Y >=3 && Pos.Y <=12 )//确保树叶不会丢失
			{
				constexpr int GridSize = 4;
				if(Pos.X % GridSize == 0 && Pos.Y% GridSize == 0)//网格交叉点
				{
					const int TreeValue = TreeNoise->GetNumWith(Pos.X + Origin.X, Pos.Y + Origin.Y, 0, ChunkSize.Z - 1);
					const int Tree = abs(TreeValue -MaxZ);
					if (MaxZ + Tree < ChunkSize.Z && Tree >= 4)
					{
						BuildTreeWith({Pos.X, Pos.Y, MaxZ}, Tree);
					}
				}
			}
		}	
	}
}

void FUnitChunk::BuildTreeWith(const FIntVector& RootLocation, int TreeHeight)
{
	//.确定树的高度
	//.生成树干
	for(int i =0 ;i < TreeHeight;++i)
	{
		FIntVector Position = RootLocation+FIntVector(0,0,i);
		if(IsinTheBoundary(Position))
		{
			CubeMap.Add(Position, EUnitCubeType::OakLog);
		}else
		{
			TreeHeight = i+1;
			break;
		}
	}
	//.生成树叶
	FIntVector Position = FIntVector::ZeroValue;
	for(int Z = 0;Z<TreeHeight+1;++Z)
	{
		Position = RootLocation +FIntVector(0,0,Z);
		//离地高度
		if(Z >= 2)
		{
			//从上往下生成
			if(Z == TreeHeight)//树根顶层的上一层
			{
				if(IsinTheBoundary(Position))
				{
					BuildTreeLeaves(Position, Surrounded_1);
				}
			}else if(Z == TreeHeight-1)//树根顶层
			{
				if(TreeHeight >= 2)
					BuildTreeLeaves(Position, Surrounded_4);
				else
					BuildTreeLeaves(Position, Surrounded_8);
			}else if(Z == 2)
			{
				BuildTreeLeaves(RootLocation+FIntVector(0,0,Z),Surrounded_4);
			}else
			{
				BuildTreeLeaves(RootLocation+FIntVector(0,0,Z),Surrounded_12);	
			}
			
		}
	}
}

void FUnitChunk::BuildTreeLeaves(const FIntVector& Pos, const ETreeLeavesStyle& Style)
{
	TArray<FIntVector> Leaves;
	if(Style == Surrounded_1)
	{
		Leaves.Add(Pos);
	}else if(Style == Surrounded_4)
	{
		Leaves.Add(FIntVector(Pos.X-1,Pos.Y,Pos.Z));
		Leaves.Add(FIntVector(Pos.X+1,Pos.Y,Pos.Z));
		Leaves.Add(FIntVector(Pos.X,Pos.Y-1,Pos.Z));
		Leaves.Add(FIntVector(Pos.X,Pos.Y+1,Pos.Z));
	}else if(Style == Surrounded_5)
	{
		Leaves.Add(FIntVector(Pos.X-1,Pos.Y,Pos.Z));
		Leaves.Add(FIntVector(Pos.X+1,Pos.Y,Pos.Z));
		Leaves.Add(FIntVector(Pos.X,Pos.Y-1,Pos.Z));
		Leaves.Add(FIntVector(Pos.X,Pos.Y+1,Pos.Z));
		Leaves.Add(Pos);
	}else if(Style == Surrounded_8)
	{
		Leaves.Add(FIntVector(Pos.X - 1, Pos.Y, Pos.Z));
		Leaves.Add(FIntVector(Pos.X - 1, Pos.Y-1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X - 1, Pos.Y+1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X + 1, Pos.Y, Pos.Z));
		Leaves.Add(FIntVector(Pos.X + 1, Pos.Y+1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X + 1, Pos.Y-1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X, Pos.Y - 1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X, Pos.Y + 1, Pos.Z));
	}else if(Style == Surrounded_9)
	{
		Leaves.Add(FIntVector(Pos.X - 1, Pos.Y, Pos.Z));
		Leaves.Add(FIntVector(Pos.X - 1, Pos.Y-1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X - 1, Pos.Y+1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X + 1, Pos.Y, Pos.Z));
		Leaves.Add(FIntVector(Pos.X + 1, Pos.Y+1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X + 1, Pos.Y-1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X, Pos.Y - 1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X, Pos.Y + 1, Pos.Z));
		Leaves.Add(Pos);
	}else if(Style == Surrounded_12)
	{
		Leaves.Add(FIntVector(Pos.X - 1, Pos.Y, Pos.Z));
		Leaves.Add(FIntVector(Pos.X - 2, Pos.Y, Pos.Z));
		Leaves.Add(FIntVector(Pos.X - 1, Pos.Y-1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X - 1, Pos.Y+1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X + 1, Pos.Y, Pos.Z));
		Leaves.Add(FIntVector(Pos.X + 2, Pos.Y, Pos.Z));
		Leaves.Add(FIntVector(Pos.X + 1, Pos.Y+1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X + 1, Pos.Y-1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X, Pos.Y - 1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X, Pos.Y - 2, Pos.Z));
		Leaves.Add(FIntVector(Pos.X, Pos.Y + 1, Pos.Z));
		Leaves.Add(FIntVector(Pos.X, Pos.Y + 2, Pos.Z));
	}
	for(const auto& Loc:Leaves)
	{
		if(IsinTheBoundary(Loc))
		{
			CubeMap.Add(Loc,EUnitCubeType::OakLeaves);
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
		//存在透明方块，如果是透明的，也视作不存在邻居
		EUnitCubeType* Search = nullptr;
		for(int i=0;i<6;++i)
		{
			Search = CubeMap.Find(Neighbors[i]);
			//如果没有该方块，则该面不存在
			if(Search == nullptr)
			{
				//如果是边界内的位置,说明该方块为表面方块
				if(IsinTheBoundary(Neighbors[i]))
				{
					bIsSurface = true;
					break;
				}
			}else
			{
				if(FUnitCubeType::IsTransparent(*Search))
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

bool FUnitChunk::TrySave()
{
	return true;
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

void FUnitChunk::AddSurfaceCubeWith(const FIntVector& CubeMapCoord)
{
	SurfaceCubes.Add(CubeMapCoord);
}

void FUnitChunk::DelSurfaceCubeWith(const FIntVector& CubeMapCoord)
{
	SurfaceCubes.Remove(CubeMapCoord);
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

void FUnitChunkManager::LoadChunkData(const FIntVector& ChunkPosition)
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
		CUSTOM_LOG_INFO(TEXT("can't find Chunk with ChunkPosition %s"),*ChunkPosition.ToString());
		return nullptr;
	}
}
