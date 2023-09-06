// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshManager.h"

// Sets default values
AMeshManager::AMeshManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	InitializeTheMeshArray();
}

// Called when the game starts or when spawned
void AMeshManager::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMeshManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMeshManager::InitializeTheMeshArray()
{
	//静态构造加载资源
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ResourceAsset
		(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (ResourceAsset.Succeeded())
	{
		for (int i = 0; i < EFaceMeshType::Size; ++i)
		{
			//创建新的实例静态网格体，然后配置预设
			FString MeshName = FString::Printf(TEXT("Mesh_%d"), i);
			UInstancedStaticMeshComponent* NewMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(*MeshName);
			NewMesh->SetStaticMesh(ResourceAsset.Object);
			NewMesh->SetMobility(EComponentMobility::Type::Static);
			NewMesh->SetCollisionProfileName(TEXT("NoCollision"));
			NewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			NewMesh->SetupAttachment(RootComponent);
			//添加到数组中
			MeshArray.Add(NewMesh);
		}
	}
}

bool AMeshManager::ShowCubeFaceWith(const FIntVector& Key, EFaceMeshType Type)
{
	auto Mesh = MeshArray[Type];
	return true;
}

bool AMeshManager::HideCubeFaceWith(const FIntVector& key, EFaceMeshType Type)
{
	return false;
}
