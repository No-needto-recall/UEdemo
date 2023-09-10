// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "InstancedMesh.generated.h"

enum EFaceDirection : uint8;
//前置声明
class AUnitCube;

//网格体实例的类型
enum EInstancedMeshType:uint32
{
	StoneMesh = 0,
	Size,
};

USTRUCT()
struct FUnitCubeAddData
{
	GENERATED_BODY()
	UPROPERTY()
	AUnitCube* UnitCube;
	
	EFaceDirection Direction;
	FTransform Transform;
};

USTRUCT()
struct FUnitCubeDelData
{
	GENERATED_BODY()
	int32 Index;
	FTransform Transform;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEMO_API UInstancedMesh : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInstancedMesh();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//实例静态网格体
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "InstancedMesh")
	UInstancedStaticMeshComponent* InstancedMesh;
	//初始化网格体
	void InitializeMesh();

	
	//Cube需要在新的位置添加实例
	void AddMeshWith(AUnitCube* Cube, const EFaceDirection& Direction, const FTransform& NewTransform); 
	//哪些索引的实例需要被删除
	void DelMeshWith(const int32& Index,const FTransform& Transform);
	//更新实例变换
	void UpdateInstanceTransformation();
	void UpdateInstanceTransformation_Version2();
	//名称
	static FString GetInstancedMeshName(const EInstancedMeshType& Type);
	//测试函数
	void TestFunc();
private:
	UPROPERTY()
	TArray<FUnitCubeAddData> AddMeshArray;
	UPROPERTY()
	TArray<FUnitCubeDelData> DelMeshArray;
};
