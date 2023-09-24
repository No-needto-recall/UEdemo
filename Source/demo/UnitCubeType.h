// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

enum EFaceDirection : uint8;
enum EInstancedMeshType : uint32;
/**
 * 
 */
UENUM(BlueprintType)
enum class EUnitCubeType : uint8
{
	Stone = 0,
	Grass,
	BedRock,
	OakLog,
	OakPlanks,
	OakLeaves
};



class DEMO_API UUnitCubeType : public TSharedFromThis<UUnitCubeType> 
{
public:
	virtual ~UUnitCubeType();
	static TSharedPtr<UUnitCubeType> BuildUnitCubeType(const EUnitCubeType& Type);
	//由方向，判断自身类别需要什么样的实例静态网格体
	virtual  EInstancedMeshType GetMeshType(const EFaceDirection& Direction);
	//获取自身枚举
	virtual EUnitCubeType GetTypeEnum()const;
	//是否为实心
	virtual bool IsSolid()const;//默认为真
};

class DEMO_API UUnitCubeType_Stone: public UUnitCubeType 
{
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;	
	virtual EUnitCubeType GetTypeEnum()const override;
};

class DEMO_API UUnitCubeType_Grass:public UUnitCubeType
{
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetTypeEnum()const override;
};

class DEMO_API UUnitCubeType_BedRock:public UUnitCubeType
{
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetTypeEnum()const override;
};

class DEMO_API UUnitCubeType_OakLog:public UUnitCubeType
{
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetTypeEnum()const override;
};

class DEMO_API UUnitCubeType_OakPlanks:public UUnitCubeType
{
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetTypeEnum()const override;
};

class DEMO_API UUnitCubeType_OakLeaves:public UUnitCubeType
{
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetTypeEnum()const override;
	virtual bool IsSolid() const override;
};
