// Fill out your copyright notice in the Description page of Project Settings.


#include "NoiseBuilder.h"

#include "MyCustomLog.h"


FNoise::FNoise()
	:FNoise(0)
{
}

FNoise::FNoise(const int32& BuildSeed)
{
	Noise.SetSeed(BuildSeed);
}

void FNoise::SetNoiseSeed(const int32& Seed)
{
	Noise.SetSeed(Seed);
}

void FNoise::SetNoiseType(const FastNoiseLite::NoiseType& Type)
{
	Noise.SetNoiseType(Type);
}

float FNoise::GetNoiseWith(const float& X, const float& Y) const
{
	return Noise.GetNoise(X,Y);
}

float FNoise::GetNoiseWith(const float& X, const float& Y, const float& Z) const
{
	return Noise.GetNoise(X,Y,Z);
}

int32 FNoise::NoiseNumToCustomRange(float NoiseNum, const int32& Left, const int32& Right)
{
	//将噪音值从[-1,1]，映射到[0,1]
	NoiseNum = (NoiseNum + 1.0f) * 0.5f;
	return std::round(NoiseNum * (Right - Left )+ Left);	
}

int32 FNoise::GetNumWith(const int32& X, const int32& Y, const int32& Left, const int32& Right) const
{
	return NoiseNumToCustomRange(GetNoiseWith(static_cast<float>(X),static_cast<float>(Y)),Left,Right);
}


void FNoiseBuilder::BuildNoise()
{
	constexpr int Size = static_cast<int>(ENoiseLiteType::Size);
	for(int i=0;i<Size;++i)
	{
		NoiseMap.Add(static_cast<ENoiseLiteType>(i),MakeShareable(new FNoise(BuildSeed)));
	}
	GetFNoise(ENoiseLiteType::Terrain)->SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2S);
	GetFNoise(ENoiseLiteType::Tree)->SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
}

FNoiseBuilder::FNoiseBuilder():FNoiseBuilder(0)
{
}

FNoiseBuilder::FNoiseBuilder(const int32& BuildSeed)
	:BuildSeed(BuildSeed)
{
	BuildNoise();
}
FNoiseBuilder::~FNoiseBuilder()
{
}

void FNoiseBuilder::SetNoiseSeed(const int32& Seed)
{
	for(const auto& Noise:NoiseMap)
	{
		Noise.Value->SetNoiseSeed(Seed);
	}
}

TSharedPtr<FNoise> FNoiseBuilder::GetFNoise(const ENoiseLiteType& NoiseType)
{
	const auto Search = NoiseMap.Find(NoiseType);
	if(Search)
	{
		return *Search;
	}
	CUSTOM_LOG_WARNING(TEXT("Noise is nullptr"));
	return nullptr;
}
