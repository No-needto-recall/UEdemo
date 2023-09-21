// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedMesh.h"
#include "UnitCube.h"

// Sets default values for this component's properties
UInstancedMesh::UInstancedMesh()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	this->USceneComponent::SetMobility(EComponentMobility::Static);
	InitializeMesh();
	// ...
}


// Called when the game starts
void UInstancedMesh::BeginPlay()
{
	Super::BeginPlay();
	// ...
}


// Called every frame
void UInstancedMesh::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInstancedMesh::InitializeMesh()
{
	//静态构造加载资源
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ResourceAsset
		(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (ResourceAsset.Succeeded())
	{
		//创建新的实例静态网格体，然后配置预设
		UInstancedStaticMeshComponent* NewMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(
			TEXT("InstancedMesh"));
		NewMesh->SetStaticMesh(ResourceAsset.Object);
		NewMesh->SetMobility(EComponentMobility::Type::Static);
		NewMesh->SetCollisionProfileName(TEXT("NoCollision"));
		NewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		InstancedMesh = NewMesh;
		InstancedMesh->SetupAttachment(this);
	}
}

void UInstancedMesh::SetTheMaterial(const EInstancedMeshType& MeshType) const
{
	FString Path;	
	switch (MeshType) {
	case StoneMesh:
		Path = TEXT("/Game/Materials/unitCube/Stone_Mat_Inst.Stone_Mat_Inst");
		break;
	case GrassTopMesh:
		Path = TEXT("/Game/Materials/unitCube/Grass_Block_Top_Mat_Inst.Grass_Block_Top_Mat_Inst");
		break;
	case GrassSideMesh:
		Path = TEXT("/Game/Materials/unitCube/Grass_Block_Side_Mat_Inst.Grass_Block_Side_Mat_Inst");
		break;
	case GrassBottomMesh:
		Path = TEXT("/Game/Materials/unitCube/Grass_Block_Bottom_Mat_Inst.Grass_Block_Bottom_Mat_Inst");
		break;
	case BedrockMesh:
		Path = TEXT("/Game/Materials/unitCube/Bedrock_Mat_Inst.Bedrock_Mat_Inst");
		break;
	case OakLogSideMesh:
		Path = TEXT("/Game/Materials/unitCube/Oak_Log_Side_Mat_Inst.Oak_Log_Side_Mat_Inst");
		break;
	case OakLogTopBottomMesh:
		Path = TEXT("/Game/Materials/unitCube/Oak_Log_TopBottom_Mat_Inst.Oak_Log_TopBottom_Mat_Inst");
		break;
	case OakPlanksMesh:
		Path = TEXT("/Game/Materials/unitCube/Oak_Planks_Mat_Inst.Oak_Planks_Mat_Inst");
		break;
	case OakLeavesMesh:
		Path = TEXT("/Game/Materials/unitCube/Oak_Leaves_Mat_Inst.Oak_Leaves_Mat_Inst");
		break;
	default: 
		UE_LOG(LogTemp,Warning,TEXT("Wrong InstancedMesh Type "));
	case Size:
		break;
	}
	const ConstructorHelpers::FObjectFinder<UMaterialInstance> MaterialInstanceFinder(*Path);
	if(MaterialInstanceFinder.Succeeded())
	{
		InstancedMesh->SetMaterial(0,MaterialInstanceFinder.Object);
	}else
	{
		UE_LOG(LogTemp,Warning,TEXT("Wrong MaterialInstance Path: %s"),*Path);
	}
}

void UInstancedMesh::AddMeshWith(AUnitCube* Cube, const EFaceDirection& Direction, const FTransform& NewTransform)
{
	if (Cube && IsValid(Cube))
	{
		FUnitCubeAddData Pair;
		Pair.UnitCube = Cube;
		Pair.Direction = Direction;
		Pair.Transform = NewTransform;
		AddMeshArray.Add(Pair);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Cube DoesnT Exist"));
	}
}

void UInstancedMesh::DelMeshWith(const int32& Index, const FTransform& Transform)
{
	if (Index == AUnitCube::HideIndex)
	{
		UE_LOG(LogTemp, Log, TEXT("Wrong Index num!"));
		return;
	}
	FUnitCubeDelData Data;
	Data.Index = Index;
	Data.Transform = Transform;
	DelMeshArray.Add(Data);
}

void UInstancedMesh::UpdateInstanceTransformation()
{
	for (auto& Data : AddMeshArray)
	{
		if (Data.UnitCube && IsValid(Data.UnitCube))
		{
			//如果有需要添加实例的，从需要删除的下标中获取
			if (DelMeshArray.Num())
			{
				const int32 Index = DelMeshArray.Last().Index;
				Data.UnitCube->FaceIndex[Data.Direction] = Index;
				const bool Ret = InstancedMesh->BatchUpdateInstancesTransform(
					Index, 1, Data.Transform, true, true, true);
				if (!Ret)
				{
					UE_LOG(LogTemp, Log, TEXT("UpdateInstanceTransform faild,index:%d"), DelMeshArray.Last().Index);
				}else
				{
					//UE_LOG(LogTemp, Log, TEXT("Reset succuese,index:%d"), DelMeshArray.Last().Index);
				}
				DelMeshArray.Pop();
			}
			else
			{
				Data.UnitCube->FaceIndex[Data.Direction] = InstancedMesh->AddInstanceWorldSpace(Data.Transform);
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Wrony Cube"));
		}
	}
	AddMeshArray.Empty(EFaceDirection::DirectionSize);
	int32 TmpNum = DelMeshArray.Num();
	//处理剩余的需要删除的实例
	while (TmpNum)
	{
		//FMyWrapper::HidInstancedMesh(InstancedMesh,DelMeshArray.Last());
		FTransform Transform;
		Transform.SetScale3D(FVector(0.0f));
		Transform.SetTranslation(FVector(0.0f));
		const bool Ret = InstancedMesh->BatchUpdateInstancesTransform(DelMeshArray[TmpNum-1].Index, 1, Transform, true,
		                                                              true, true);
		if (!Ret)
		{
			UE_LOG(LogTemp, Log, TEXT("Remove InstanceMesh faild,index:%d"), DelMeshArray[TmpNum-1].Index);
		}else
		{
			//UE_LOG(LogTemp, Log, TEXT("Remove InstanceMesh successful,index:%d"), DelMeshArray[TmpNum-1].Index);
		}
		--TmpNum;
	}
}

void UInstancedMesh::UpdateInstanceTransformation_Version2()
{
	//另一种策略，先删除，后添加
	while (DelMeshArray.Num())
	{
		FTransform NowTransform;
		InstancedMesh->GetInstanceTransform(DelMeshArray.Last().Index, NowTransform, true);
		if (NowTransform.Equals(DelMeshArray.Last().Transform, 0.1))
		{
			const bool Ret = InstancedMesh->RemoveInstance(DelMeshArray.Last().Index);
			if (!Ret)
			{
				UE_LOG(LogTemp, Log, TEXT("Remove InstanceMesh faild,index:%d"), DelMeshArray.Last().Index);
			}
		}
		DelMeshArray.Pop();
	}
	for (auto& Data : AddMeshArray)
	{
		if (Data.UnitCube && IsValid(Data.UnitCube))
		{
			const int32 Index = InstancedMesh->AddInstanceWorldSpace(Data.Transform);
			if (Index == -1)
			{
				UE_LOG(LogTemp, Log, TEXT("Add InstanceMesh faild,Transform:%s"), *Data.Transform.ToString());
			}
			Data.UnitCube->FaceIndex[Data.Direction] = Index;
		}
	}
	AddMeshArray.Empty();
}

FString UInstancedMesh::GetInstancedMeshName(const EInstancedMeshType& Type)
{
	FString Str("Default");
	switch (Type)
	{
	case StoneMesh:
		Str = TEXT("StoneMesh");
		break;
	case GrassTopMesh:
		Str = TEXT("GrassTopMesh");
		break;
	case GrassSideMesh:
		Str = TEXT("GrassSideMesh");
		break;
	case GrassBottomMesh:
		Str = TEXT("GrassBottomMesh");
		break;
	case BedrockMesh:
		Str = TEXT("BedrockMesh");
		break;
	case OakLogSideMesh:
		Str = TEXT("OakLogSideMesh");
		break;
	case OakLogTopBottomMesh:
		Str = TEXT("OakLogTopBottomMesh");
		break;
	case OakPlanksMesh:
		Str = TEXT("OakPlanksMesh");
		break;
	default: ;
	}
	return Str;
}


