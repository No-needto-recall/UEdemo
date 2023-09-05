// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCube.h"

// Sets default values
ABaseCube::ABaseCube()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BoxInitialization();
	MeshInitialization();
}

// Called when the game starts or when spawned
void ABaseCube::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABaseCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ABaseCube::IsSolid() const
{
	return true;
}

void ABaseCube::SetFaceVisibility(const FIntVector& Direction, bool Visibility)
{
	if (Direction == FIntVector(1, 0, 0))
	{
		FrontMesh->SetVisibility(Visibility);
	}
	else if (Direction == FIntVector(-1, 0, 0))
	{
		BackMesh->SetVisibility(Visibility);
	}
	else if (Direction == FIntVector(0, 1, 0))
	{
		RightMesh->SetVisibility(Visibility);
	}
	else if (Direction == FIntVector(0, -1, 0))
	{
		LeftMesh->SetVisibility(Visibility);
	}
	else if (Direction == FIntVector(0, 0, 1))
	{
		UpMesh->SetVisibility(Visibility);
	}
	else if (Direction == FIntVector(0, 0, -1))
	{
		DownMesh->SetVisibility(Visibility);
	}
}

void ABaseCube::SetTheCollisionOfTheBoxToBeEnabled(bool Enabled)
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

bool ABaseCube::CheckIsAnyFaceIsVisible()
{
	return UpMesh->IsVisible() ||
		DownMesh->IsVisible() ||
		FrontMesh->IsVisible() ||
		BackMesh->IsVisible() ||
		RightMesh->IsVisible() ||
		LeftMesh->IsVisible();
}

bool ABaseCube::CheckThatAllFacesAreNotVisible()
{
	return !CheckIsAnyFaceIsVisible();
}

void ABaseCube::RefreshCollisionEnabled()
{
	if(CheckIsAnyFaceIsVisible())
	{
		SetTheCollisionOfTheBoxToBeEnabled(true);
	}else
	{
		SetTheCollisionOfTheBoxToBeEnabled(false);
	}
}

void ABaseCube::BoxInitialization()
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

void ABaseCube::MeshInitialization()
{
	//初始化静态网格体组件
	UpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UpMesh"));
	DownMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DownMesh"));
	FrontMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontMesh"));
	BackMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackMesh"));
	RightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightMesh"));
	LeftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftMesh"));
	//添加到Box下
	UpMesh->SetupAttachment(BoxCollisionComponent);
	DownMesh->SetupAttachment(BoxCollisionComponent);
	FrontMesh->SetupAttachment(BoxCollisionComponent);
	BackMesh->SetupAttachment(BoxCollisionComponent);
	RightMesh->SetupAttachment(BoxCollisionComponent);
	LeftMesh->SetupAttachment(BoxCollisionComponent);
	//静态构造加载资源
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ResourceAsset
		(TEXT("/Engine/BasicShapes/Plane.Plane"));
	//判断资源是否加载成功
	if (ResourceAsset.Succeeded())
	{
		//设置静态网格体资源
		UpMesh->SetStaticMesh(ResourceAsset.Object);
		DownMesh->SetStaticMesh(ResourceAsset.Object);
		FrontMesh->SetStaticMesh(ResourceAsset.Object);
		BackMesh->SetStaticMesh(ResourceAsset.Object);
		RightMesh->SetStaticMesh(ResourceAsset.Object);
		LeftMesh->SetStaticMesh(ResourceAsset.Object);
		//设置移动性
		UpMesh->SetMobility(EComponentMobility::Type::Static);
		DownMesh->SetMobility(EComponentMobility::Type::Static);
		FrontMesh->SetMobility(EComponentMobility::Type::Static);
		BackMesh->SetMobility(EComponentMobility::Type::Static);
		RightMesh->SetMobility(EComponentMobility::Type::Static);
		LeftMesh->SetMobility(EComponentMobility::Type::Static);
		//设置碰撞预设
		UpMesh->SetCollisionProfileName(TEXT("NoCollision"));
		DownMesh->SetCollisionProfileName(TEXT("NoCollision"));
		FrontMesh->SetCollisionProfileName(TEXT("NoCollision"));
		BackMesh->SetCollisionProfileName(TEXT("NoCollision"));
		RightMesh->SetCollisionProfileName(TEXT("NoCollision"));
		LeftMesh->SetCollisionProfileName(TEXT("NoCollision"));
		//设置碰撞已启用
		UpMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DownMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FrontMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BackMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		LeftMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//根据方位，配置位置和旋转
		UpMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
		UpMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

		DownMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f));
		DownMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 180.f));

		FrontMesh->SetRelativeLocation(FVector(50.0f, 0.0f, 0.0f));
		FrontMesh->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));

		BackMesh->SetRelativeLocation(FVector(-50.0f, 0.0f, 0.0f));
		BackMesh->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));

		RightMesh->SetRelativeLocation(FVector(0.0f, 50.0f, 0.0f));
		RightMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));

		LeftMesh->SetRelativeLocation(FVector(0.0f, -50.0f, 0.0f));
		LeftMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, -90.0f));
	}
}
