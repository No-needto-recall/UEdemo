// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UnitCubeType.generated.h"

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
	OakPlanks
};


UCLASS()
class DEMO_API UUnitCubeType : public UObject
{
	GENERATED_BODY()

public:
	static UUnitCubeType* BuildUnitCubeType(const EUnitCubeType& Type);
	//由方向，判断自身类别需要什么样的实例静态网格体
	virtual  EInstancedMeshType GetMeshType(const EFaceDirection& Direction);
	//获取自身枚举
	virtual EUnitCubeType GetCubeType()const;
};

UCLASS()
class DEMO_API UUnitCubeType_Stone: public UUnitCubeType 
{
	GENERATED_BODY()

public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;	
	virtual EUnitCubeType GetCubeType()const override;
};

UCLASS()
class DEMO_API UUnitCubeType_Grass:public UUnitCubeType
{
	GENERATED_BODY()
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetCubeType()const override;
};

UCLASS()
class DEMO_API UUnitCubeType_BedRock:public UUnitCubeType
{
	GENERATED_BODY()
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetCubeType()const override;
};

UCLASS()
class DEMO_API UUnitCubeType_OakLog:public UUnitCubeType
{
	GENERATED_BODY()
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetCubeType()const override;
};

UCLASS()
class DEMO_API UUnitCubeType_OakPlanks:public UUnitCubeType
{
	GENERATED_BODY()
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetCubeType()const override;
};
