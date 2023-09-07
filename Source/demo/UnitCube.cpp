// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitCube.h"

int32 AUnitCube::HideIndex= -1;


// Sets default values
AUnitCube::AUnitCube()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BoxInitialization();
	ArrayInitialization();

}

// Called when the game starts or when spawned
void AUnitCube::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUnitCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AUnitCube::IsSolid() const
{
	return true;
}

bool AUnitCube::IsTransparent() const
{
	return !IsSolid();
}

void AUnitCube::SetTheCollisionOfTheBoxToBeEnabled(bool Enabled)
{
	if (Enabled)
	{
		BoxCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		BoxCollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AUnitCube::RefreshCollisionEnabled()
{
	if (CheckIsAnyFaceIsVisible())
	{
		SetTheCollisionOfTheBoxToBeEnabled(true);
	}
	else
	{
		SetTheCollisionOfTheBoxToBeEnabled(false);
	}
}

bool AUnitCube::CheckIsAnyFaceIsVisible()
{
	for(const auto& Index:FaceIndex)
	{
		if(Index != HideIndex)
		{
			return true;
		}
	}
	return false;
}

bool AUnitCube::CheckThatAllFacesAreNotVisible()
{
	return !CheckIsAnyFaceIsVisible();
}

void AUnitCube::OnDestroyed()
{
	Destroy();
}

FString AUnitCube::FaceDirectionToFString(const EFaceDirection& FaceDirection)
{
	FString Msg = TEXT("WrongDirection");
	switch (FaceDirection) {
	case Top:
		Msg = TEXT("Top");
		break;
	case Bottom:
		Msg = TEXT("Bottom");
		break;
	case Front:
		Msg = TEXT("Front");
		break;
	case Back:
		Msg = TEXT("Back");
		break;
	case Right:
		Msg = TEXT("Right");
		break;
	case Left:
		Msg = TEXT("Left");
		break;
	default: ;
	}
	return Msg;
}

void AUnitCube::BoxInitialization()
{
	
	//创建并初始化Box碰撞组件
	BoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollisionComponent"));
	//设置为根组件
	RootComponent = BoxCollisionComponent;
	//设置Box尺寸
	BoxCollisionComponent->InitBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	//设置移动性
	BoxCollisionComponent->SetMobility(EComponentMobility::Type::Static);
	//设置碰撞类型
	BoxCollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	//设置碰撞已启用
	BoxCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//设置碰撞细节
	//忽略所有
	BoxCollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//对Pawn设置为阻挡
	BoxCollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	//对射线设置为阻挡
	BoxCollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	BoxCollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,
	                                                     ECollisionResponse::ECR_Block);
}

void AUnitCube::ArrayInitialization()
{
	//初始化索引的数组
	uint32 Size = EFaceDirection::DirectionSize;	
	for(uint32 i = 0; i< Size;++i)
	{
		FaceIndex.Add(AUnitCube::HideIndex);	
	}
}

