// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "UnitCube.generated.h"

class UUnitCubeType;

//立方体面的方位枚举
enum EFaceDirection : uint8
{
	Top = 0,
	Bottom,
	Front,
	Back,
	Right,
	Left,
	DirectionSize
};



UCLASS()
class DEMO_API AUnitCube : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AUnitCube();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//静态网格体的实例索引数组
	UPROPERTY(VisibleAnywhere,Category = "Face Index")
	TArray<int32> FaceIndex;
private:
	//声明Box碰撞组件
	UPROPERTY(VisibleAnywhere,Category = "Box Collision")
	UBoxComponent* BoxCollisionComponent;
	//方块类别
	UPROPERTY(VisibleAnywhere,Category = "Cube Type")
	UUnitCubeType* CubeType;
public:
	//安全获取Type;
	UUnitCubeType* GetCubeType();
	//配置Type;
	void SetCubeType(UUnitCubeType* Type);
	//是否是实心
	bool IsSolid();
	//是否是透明
	bool IsTransparent();
	//设置Box的碰撞开启
	void SetTheCollisionOfTheBoxToBeEnabled(bool Enabled) const;
	//刷新Box的碰撞启用
	bool RefreshCollisionEnabled();
	//检测是否有面是可见的
	bool CheckIsAnyFaceIsVisible();
	//检查是否所有面均不可见
	bool CheckThatAllFacesAreNotVisible();
	
	//获取面的绝对变换
	FTransform GetFaceTransform(const EFaceDirection& Direction) const;
	
	EFaceDirection GetFaceDirectionWith(const FIntVector& Direction) const;
	
	//销毁
	void OnDestroyed();
	//FaceDirection 转为 FString
	static  FString FaceDirectionToFString(const EFaceDirection& FaceDirection);

	//用于表示空索引，即没有实例，该面处于隐藏状态
	static int32 HideIndex;

private:
	//碰撞盒体的初始化
	void BoxInitialization();
	//实例索引数组的初始化
	void ArrayInitialization();
};
