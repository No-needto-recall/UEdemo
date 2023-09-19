// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitCube.h"

int32 AUnitCube::HideIndex = -1;


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

void AUnitCube::SetTheCollisionOfTheBoxToBeEnabled(bool Enabled) const
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

bool AUnitCube::RefreshCollisionEnabled()
{
	if (CheckIsAnyFaceIsVisible())
	{
		SetTheCollisionOfTheBoxToBeEnabled(true);
		return true;
	}
	else
	{
		SetTheCollisionOfTheBoxToBeEnabled(false);
		return false;
	}
}

//有bug，当前网格体实例策略不适合检测是否有面是显示的
//尝试在网格体实例更新完成后更新碰撞
bool AUnitCube::CheckIsAnyFaceIsVisible()
{
	for (const auto& Index : FaceIndex)
	{
		if (Index != HideIndex)
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

FTransform AUnitCube::GetFaceTransform(const EFaceDirection& Direction) const
{
	//获取Cube世界坐标、旋转、缩放
	FVector Location = GetActorLocation();
	FRotator Rotator = GetActorRotation();
	FVector Scale = GetActorScale();
	switch (Direction)
	{
	case Top:
		Location += FVector(0.0f, 0.0f, 50.0f);
		Rotator += FRotator(0.0f, 0.0f, 0.0f);
		break;
	case Bottom:
		Location += FVector(0.0f, 0.0f, -50.0f);
		Rotator += FRotator(0.0f, 0.0f, 180.0f);
		break;
	case Front:
		Location += FVector(50.0f, 0.0f, 0.0f);
		Rotator += FRotator(0.0f, 270.0f, 90.0f);
		break;
	case Back:
		Location += FVector(-50.0f, 0.0f, 0.0f);
		Rotator += FRotator(0.0f, 90.0f, 90.0f);
		break;
	case Right:
		Location += FVector(0.0f, 50.0f, 0.0f);
		Rotator += FRotator(0.0f, 0.0f, 90.0f);
		break;
	case Left:
		Location += FVector(0.0f, -50.0f, 0.0f);
		Rotator += FRotator(0.0f, 180.0f, 90.0f);
		break;
	case DirectionSize:
		break;
	default: ;
	}
	//构建对应面的世界变换
	return FTransform(Rotator.Quaternion(),Location,Scale);
}

EFaceDirection AUnitCube::GetFaceDirectionWith(const FIntVector& Direction) const
{
	EFaceDirection FaceDirection(Front);
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
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Direction not valid :%s"), *Direction.ToString());
	}
	return FaceDirection;
}

void AUnitCube::OnDestroyed()
{
	Destroy();
}

FString AUnitCube::FaceDirectionToFString(const EFaceDirection& FaceDirection)
{
	FString Msg = TEXT("WrongDirection");
	switch (FaceDirection)
	{
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
	//设置碰撞 默认为 不开启
	BoxCollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//设置碰撞细节
	//忽略所有
	BoxCollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//对Pawn设置为阻挡
	BoxCollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	//对射线设置为阻挡
	BoxCollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	BoxCollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,
	                                                     ECollisionResponse::ECR_Block);
	//关闭Tick
	this->PrimaryActorTick.bCanEverTick = false;
}

void AUnitCube::ArrayInitialization()
{
	//初始化索引的数组
	const uint32 Size = EFaceDirection::DirectionSize;
	for (uint32 i = 0; i < Size; ++i)
	{
		FaceIndex.Add(AUnitCube::HideIndex);
	}
}
