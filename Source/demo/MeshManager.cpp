// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshManager.h"


#include "InstancedMesh.h"
#include "UnitCube.h"
#include "UnitCubeType.h"

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
		for (int32 i  = 0; i < EInstancedMeshType::Size; ++i)
		{
			//创建新的实例静态网格体，然后配置预设
			FString Name = UInstancedMesh::GetInstancedMeshName(static_cast<EInstancedMeshType>(i));  
			UInstancedMesh* NewMesh = CreateDefaultSubobject<UInstancedMesh>(*Name);
			NewMesh->SetTheMaterial(static_cast<EInstancedMeshType>(i));
			//添加到数组中
			MeshArray.Add(NewMesh);
			NewMesh->SetupAttachment(RootComponent);
		}
	}
}

void AMeshManager::InitializeTheMeshManager()
{
	SetActorLocation(FVector(0.0f));
	for (const auto& Mesh : MeshArray)
	{
		Mesh->SetRelativeLocation(FVector(0.0f));
		Mesh->InstancedMesh->SetRelativeLocation(FVector(0.0f));
	}
}

bool AMeshManager::AddMeshToCubeWith(const FIntVector& Direction, AUnitCube* Cube)
{
	if (Cube && IsValid(Cube))
	{
		const EFaceDirection FaceDirection = Cube->GetFaceDirectionWith(Direction);
		const auto Type = Cube->GetCubeType()->GetMeshType(FaceDirection);
		const auto Mesh = MeshArray[Type];
		if (Mesh )
		{
			if (Cube->FaceIndex[FaceDirection] != AUnitCube::HideIndex)
			{
				//UE_LOG(LogTemp, Log, TEXT("Cube:%s,(%s)Face Already Show"),*Cube->GetName(), *AUnitCube::FaceDirectionToFString(FaceDirection));
				return false;
			}
			const FTransform FaceTransform = Cube->GetFaceTransform(FaceDirection);
			//添加实例，设置索引
			Mesh->AddMeshWith(Cube,FaceDirection,FaceTransform);
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

bool AMeshManager::DelMeshToCubeWith(const FIntVector& Direction, AUnitCube* Cube)
{
	if (Cube && IsValid(Cube))
	{
		const EFaceDirection FaceDirection = Cube->GetFaceDirectionWith(Direction);
		const auto Type = Cube->GetCubeType()->GetMeshType(FaceDirection);
		const auto Mesh = MeshArray[Type];
		if (Mesh && IsValid(Mesh))
		{
			if (Cube->FaceIndex[FaceDirection] == AUnitCube::HideIndex)
			{
				//UE_LOG(LogTemp, Log, TEXT("Cube:%s,(%s)Face Already Hide"),*Cube->GetName(), *AUnitCube::FaceDirectionToFString(FaceDirection));
				return false;
			}
			Mesh->DelMeshWith(Cube->FaceIndex[FaceDirection],Cube->GetFaceTransform(FaceDirection));
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

void AMeshManager::UpdateAllInstancedMesh()
{
	for(const auto& Mesh: MeshArray)
	{
		Mesh->UpdateInstanceTransformation();
		//Mesh->UpdateInstanceTransformation_Version2();
		Mesh->InstancedMesh->FlushInstanceUpdateCommands();
	}
	FlushRenderingCommands();
}

