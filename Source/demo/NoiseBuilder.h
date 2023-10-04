// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FastNoiseLite.h"

/**
 * 
 */
enum class ENoiseLiteType : int
{
	Terrain = 0,
	Tree,
	Size,
};

class DEMO_API FNoise : public  TSharedFromThis<FNoise>
{
	FastNoiseLite Noise;
public:
	FNoise();
	explicit FNoise(const int32& BuildSeed);
	//设置噪音种子
	void SetNoiseSeed(const int32& Seed);
	//设置噪音类型
	void SetNoiseType(const FastNoiseLite::NoiseType& Type);
	//生成平面噪音值
	float GetNoiseWith(const float& X, const float& Y) const;
	//生成立体噪音值
	float GetNoiseWith(const float& X, const float& Y, const float& Z) const;
	//缩放噪音值，得到范围[Left,Right]中的整数
	static int32 NoiseNumToCustomRange(float NoiseNum, const int32& Left, const int32& Right);
	//根据平面值，得到最终的范围值
	int32 GetNumWith(const int32& X, const int32& Y, const int32& Left, const int32& Right) const;
};


class DEMO_API FNoiseBuilder :public  TSharedFromThis<FNoiseBuilder>
{
	TMap<ENoiseLiteType,TSharedPtr<FNoise>> NoiseMap;
	int32 BuildSeed;
	void BuildNoise();
public:
	
	FNoiseBuilder();
	explicit FNoiseBuilder(const int32& BuildSeed);
	~FNoiseBuilder();
	void SetNoiseSeed(const int32& Seed);
	TSharedPtr<FNoise> GetFNoise(const ENoiseLiteType& NoiseType);
};
