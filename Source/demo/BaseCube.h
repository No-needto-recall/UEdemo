// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "BaseCube.generated.h"

UCLASS()
class DEMO_API ABaseCube : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseCube();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//声明Box碰撞组件
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Box Collision")
	UBoxComponent* BoxCollisionComponent;
	//声明静态网格体组件
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Mesh")
	UStaticMeshComponent* UpMesh;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Mesh")
	UStaticMeshComponent* DownMesh;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Mesh")
	UStaticMeshComponent* FrontMesh;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Mesh")
	UStaticMeshComponent* BackMesh;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Mesh")
	UStaticMeshComponent* RightMesh;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Mesh")
	UStaticMeshComponent* LeftMesh;
public:
	//是否是实心
	bool IsSolid()const;
	//设置面的可视性
	void SetFaceVisibility(const FIntVector& Direction, bool Visibility);
	//设置面的移动性
	void SetFaceMobility(EComponentMobility::Type Type);
	//设置Box的碰撞启用
	void SetTheCollisionOfTheBoxToBeEnabled(bool Enabled);
	//检测是否有面是可见的
	bool CheckIsAnyFaceIsVisible();
	//检查是否所有面均不可见
	bool CheckThatAllFacesAreNotVisible();
	//刷新Box的碰撞启用
	void RefreshCollisionEnabled();
	//销毁
	void OnDestroyed();
	
	
private:
	//盒体的默认初始化
	void BoxInitialization();
	//6个面的默认初始化
	void MeshInitialization();
};
