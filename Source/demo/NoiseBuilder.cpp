// Fill out your copyright notice in the Description page of Project Settings.


#include "NoiseBuilder.h"

FNoiseBuilder::FNoiseBuilder():FNoiseBuilder(0)
{
}

FNoiseBuilder::FNoiseBuilder(const int32& BuildSeed)
	:BuildSeed(BuildSeed)
{
	Noise.SetSeed(BuildSeed);
	SetNoiseType();
}

FNoiseBuilder::~FNoiseBuilder()
{
}

void FNoiseBuilder::SetNoiseSeed(const int32& Seed)
{
	this->BuildSeed = Seed;
	Noise.SetSeed(BuildSeed);
}

void FNoiseBuilder::SetNoiseType(const FastNoiseLite::NoiseType& Type)
{
	Noise.SetNoiseType(Type);
}

float FNoiseBuilder::GetNoiseWith(const float& X, const float& Y) const
{
	return Noise.GetNoise(X,Y);
}

int32 FNoiseBuilder::NoiseNumToCustomRange(float NoiseNum, const int32& Left, const int32& Right)
{
	//将噪音值从[-1,1]，映射到[0,1]
	NoiseNum = (NoiseNum + 1.0f) * 0.5f;
	return std::round(NoiseNum * (Right - Left )+ Left);	
}

int32 FNoiseBuilder::GetNumWith(const int32& X, const int32& Y, const int32& Left, const int32& Right) const
{
	return NoiseNumToCustomRange(GetNoiseWith(static_cast<float>(X),static_cast<float>(Y)),Left,Right);
}

