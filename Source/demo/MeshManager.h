// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshManager.generated.h"

//网格枚举
enum EFaceMeshType : uint8
{
	Stone_Top = 0,
	Stone_Bottom ,
	Stone_Front,
	Stone_Back,
	Stone_Right,
	Stone_Left,
	Size
};



UCLASS()
class DEMO_API AMeshManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMeshManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//网格体数组
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "MeshArray")
	TArray<UInstancedStaticMeshComponent*> MeshArray;
	//初始化网格体数组
	void InitializeTheMeshArray();
	//显示某个方块的某个面
	bool ShowCubeFaceWith(const FIntVector& Key,EFaceMeshType Type);
	//隐藏某个方块的某个面
	bool HideCubeFaceWith(const FIntVector& key,EFaceMeshType Type);
};
