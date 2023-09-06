// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshManager.generated.h"

//网格枚举
enum EFaceMeshType
{
		
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

	
};
