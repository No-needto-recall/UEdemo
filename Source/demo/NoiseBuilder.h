// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FastNoiseLite.h"

/**
 * 
 */
class DEMO_API FNoiseBuilder :public  TSharedFromThis<FNoiseBuilder>
{
	FastNoiseLite Noise;
	int32 BuildSeed;
public:
	
	FNoiseBuilder();
	explicit FNoiseBuilder(const int32& BuildSeed);
	~FNoiseBuilder();
	
	//设置噪音种子
	void SetNoiseSeed(const int32& Seed);
	//设置噪音类型
	void SetNoiseType(const FastNoiseLite::NoiseType & Type = FastNoiseLite::NoiseType_OpenSimplex2S);
	//生成平面噪音值
	float GetNoiseWith(const float& X, const float& Y) const;
	//缩放噪音值，得到范围[Left,Right]中的整数
	static int32 NoiseNumToCustomRange(float NoiseNum, const int32& Left, const int32& Right);
	//根据平面值，得到最终的范围值
	int32 GetNumWith(const int32& X,const int32& Y,const int32& Left,const int32& Right)const;
};
