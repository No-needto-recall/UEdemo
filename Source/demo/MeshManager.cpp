// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshManager.h"

#include "UnitCube.h"

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
		for (int i = 0; i < EFaceMeshType::MeshTypeSize; ++i)
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

void AMeshManager::InitializeTheMeshManager()
{
	SetActorLocation(FVector(0.0f));
	for (const auto& Mesh : MeshArray)
	{
		Mesh->SetRelativeLocation(FVector(0.0f));
	}
}

bool AMeshManager::AddMeshToCubeWith(const FIntVector& Direction, EFaceMeshType Type, AUnitCube* Cube)
{
	if (Cube && IsValid(Cube))
	{
		const auto Mesh = MeshArray[Type];
		if (Mesh && IsValid(Mesh))
		{
			const EFaceDirection FaceDirection = Cube->GetFaceDirectionWith(Direction);
			if (Cube->FaceIndex[FaceDirection] != AUnitCube::HideIndex)
			{
				UE_LOG(LogTemp, Log, TEXT("Cube:%s,(%s)Face Already Show"),
				       *Cube->GetName(), *AUnitCube::FaceDirectionToFString(FaceDirection));
				return false;
			}
			const FTransform FaceTransform = Cube->GetFaceTransform(FaceDirection);
			//添加实例，设置索引
			Cube->FaceIndex[FaceDirection] = Mesh->AddInstanceWorldSpace(FaceTransform);
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Mesh not valid"));
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Cube not valid"));
		return false;
	}
}

bool AMeshManager::DelMeshToCubeWith(const FIntVector& Direction, EFaceMeshType Type, AUnitCube* Cube)
{
	if (Cube && IsValid(Cube))
	{
		auto Mesh = MeshArray[Type];
		if (Mesh && IsValid(Mesh))
		{
			const EFaceDirection FaceDirection = Cube->GetFaceDirectionWith(Direction);
			if (Cube->FaceIndex[FaceDirection] == AUnitCube::HideIndex)
			{
				UE_LOG(LogTemp, Log, TEXT("Cube:%s,(%s)Face Already Hide"),
				       *Cube->GetName(), *AUnitCube::FaceDirectionToFString(FaceDirection));
				return false;
			}
			const FTransform FaceTransform = Cube->GetFaceTransform(FaceDirection);
			auto Index = Cube->FaceIndex[FaceDirection];
			FTransform OldTransform;
			Mesh->GetInstanceTransform(Index,OldTransform,true);
			if(FaceTransform.Equals(OldTransform))
			{
				Mesh->RemoveInstance(Cube->FaceIndex[FaceDirection]);
			}
			Cube->FaceIndex[FaceDirection] = AUnitCube::HideIndex;
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Mesh not valid"));
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Cube not valid"));
		return false;
	}
}

void AMeshManager::HiedCubeAllFace(AUnitCube* Cube)
{
}
