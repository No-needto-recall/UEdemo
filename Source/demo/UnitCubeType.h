// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

class FUnitCubeType;
enum EFaceDirection : uint8;
enum EInstancedMeshType : uint32;
/**
 * 
 */
//方块类型
UENUM(BlueprintType)
enum class EUnitCubeType : uint8
{
	Stone = 0,//石头
	Grass,//草方块
	BedRock,//基岩 
	OakLog,//橡木原木
	OakPlanks,//橡木方块
	OakLeaves//橡木树叶
};
//原型模式
class DEMO_API FUnitCubeTypeManager :public TSharedFromThis<FUnitCubeTypeManager>
{
public:
	TSharedPtr<FUnitCubeType> GetUnitCubeType(const EUnitCubeType& Type);
private:
	TMap<EUnitCubeType,TSharedPtr<FUnitCubeType>> TypeMap;
};


//尚未考虑用纯虚类
//或者考虑用一个default作为基类
class DEMO_API FUnitCubeType : public TSharedFromThis<FUnitCubeType> 
{
public:
	virtual ~FUnitCubeType();
	//基类中内嵌一个小型工厂
	static TSharedPtr<FUnitCubeType> BuildUnitCubeType(const EUnitCubeType& Type);
	//由方向，判断自身类别需要什么样的实例静态网格体
	virtual  EInstancedMeshType GetMeshType(const EFaceDirection& Direction);
	//获取自身枚举
	virtual EUnitCubeType GetTypeEnum()const;
	//是否为实心
	virtual bool IsSolid()const;//默认为真
};

class DEMO_API FUnitCubeType_Stone final : public FUnitCubeType 
{
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;	
	virtual EUnitCubeType GetTypeEnum()const override;
};

class DEMO_API FUnitCubeType_Grass final :public FUnitCubeType
{
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetTypeEnum()const override;
};

class DEMO_API FUnitCubeType_BedRock final :public FUnitCubeType
{
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetTypeEnum()const override;
};

class DEMO_API FUnitCubeType_OakLog final :public FUnitCubeType
{
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetTypeEnum()const override;
};

class DEMO_API FUnitCubeType_OakPlanks final :public FUnitCubeType
{
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetTypeEnum()const override;
};

class DEMO_API FUnitCubeType_OakLeaves final :public FUnitCubeType
{
public:
	virtual EInstancedMeshType GetMeshType(const EFaceDirection& Direction) override;
	virtual EUnitCubeType GetTypeEnum()const override;
	virtual bool IsSolid() const override;
};
