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
	for (auto& Mesh : MeshArray)
	{
		Mesh->SetRelativeLocation(FVector(0.0f));
	}
}

bool AMeshManager::ShowCubeFaceWith(const FIntVector& Direction, EFaceMeshType Type, AUnitCube* Cube)
{
	if (Cube && IsValid(Cube))
	{
		auto Mesh = MeshArray[Type];
		if (Mesh && IsValid(Mesh))
		{
			//获取Cube世界坐标、旋转、缩放
			FVector Location = Cube->GetActorLocation();
			FRotator Rotator = Cube->GetActorRotation();
			FVector Scale = Cube->GetActorScale();

			EFaceDirection FaceDirection = EFaceDirection::Front;
			if (Direction == FIntVector(1, 0, 0))
			{
				FaceDirection = EFaceDirection::Front;
				Location += FVector(50.0f, 0.0f, 0.0f);
				Rotator += FRotator(-90.0f, 0.0f, 0.0f);
			}
			else if (Direction == FIntVector(-1, 0, 0))
			{
				FaceDirection = EFaceDirection::Back;
				Location += FVector(-50.0f, 0.0f, 0.0f);
				Rotator += FRotator(90.0f, 0.0f, 0.0f);
			}
			else if (Direction == FIntVector(0, 1, 0))
			{
				FaceDirection = EFaceDirection::Right;
				Location += FVector(0.0f, 50.0f, 0.0f);
				Rotator += FRotator(0.0f, 0.0f, 90.0f);
			}
			else if (Direction == FIntVector(0, -1, 0))
			{
				FaceDirection = EFaceDirection::Left;
				Location += FVector(0.0f, -50.0f, 0.0f);
				Rotator += FRotator(0.0f, 0.0f, -90.0f);
			}
			else if (Direction == FIntVector(0, 0, 1))
			{
				FaceDirection = EFaceDirection::Top;
				Location += FVector(0.0f, 0.0f, 50.0f);
				Rotator += FRotator(0.0f, 0.0f, 0.0f);
			}
			else if (Direction == FIntVector(0, 0, -1))
			{
				FaceDirection = EFaceDirection::Bottom;
				Location += FVector(0.0f, 0.0f, -50.0f);
				Rotator += FRotator(0.0f, 0.0f, 180.0f);
			}else
			{
				const FString Msg = Direction.ToString();
				UE_LOG(LogTemp, Log, TEXT("Direction not valid:%s"),*Msg);
				return false;
			}
			//构建对应面的世界变换
			FQuat Quat = Rotator.Quaternion();
			FTransform FaceTransform(Quat, Location, Scale);
			//1.获取ISM组件的逆变换
			FTransform IsmTransform = Mesh->GetComponentTransform().Inverse();
			//2.获取相对于ISM组件的本地变换
			FaceTransform = FaceTransform * IsmTransform;
			//3.添加实例，设置索引
			int32 Index = FaceDirection;
			Cube->FaceIndex[Index] = Mesh->AddInstance(FaceTransform);
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

bool AMeshManager::HideCubeFaceWith(const FIntVector& Direction, EFaceMeshType Type, AUnitCube* Cube)
{
	if (Cube && IsValid(Cube))
	{
		auto Mesh = MeshArray[Type];
		if (Mesh && IsValid(Mesh))
		{
			EFaceDirection FaceDirection = EFaceDirection::Front;
			if (Direction == FIntVector(1, 0, 0))
			{
				FaceDirection = EFaceDirection::Front;
			}
			else if (Direction == FIntVector(-1, 0, 0))
			{
				FaceDirection = EFaceDirection::Back;
			}
			else if (Direction == FIntVector(0, 1, 0))
			{
				FaceDirection = EFaceDirection::Right;
			}
			else if (Direction == FIntVector(0, -1, 0))
			{
				FaceDirection = EFaceDirection::Left;
			}
			else if (Direction == FIntVector(0, 0, 1))
			{
				FaceDirection = EFaceDirection::Top;
			}
			else if (Direction == FIntVector(0, 0, -1))
			{
				FaceDirection = EFaceDirection::Bottom;
			}else
			{
				const FString Msg = Direction.ToString();
				UE_LOG(LogTemp, Log, TEXT("Direction not valid:%s"),*Msg);
				return false;
			}

			Mesh->RemoveInstance(Cube->FaceIndex[FaceDirection]);
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
