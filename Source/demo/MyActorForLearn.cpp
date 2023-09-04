// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActorForLearn.h"

#include <string>

#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

#include "MovieSceneSequenceID.h"
#include "Evaluation/Blending/MovieSceneBlendType.h"

// Sets default values
AMyActorForLearn::AMyActorForLearn()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//初始化一个静态网格体组件
	this->Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MyMesh"));
	//将Mesh设置为根节点
	this->Mesh->SetupAttachment(this->RootComponent);
#if 0
	this->RootComponent = this->Mesh;
#endif
	//构造帮助
	//静态是为了之后不需要重复加载
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ResourcesAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	//判断资源加载是否成功
	if(ResourcesAsset.Succeeded())
	{
		//设置静态网格体
		this->Mesh->SetStaticMesh(ResourcesAsset.Object);
		//额外的设置
		this->Mesh->SetRelativeLocation(FVector(0.0f,0.0f,0.0f));
		this->Mesh->SetRelativeScale3D(FVector(1.0f,1.0f,1.0f));
	}

	this->PitchValue = 0.0f;
	this->YawValue= 0.0f;
	this->RollValue= 0.0f;

	this->Mesh->SetSimulatePhysics(true);
	this->Mesh->SetNotifyRigidBodyCollision(true);
	this->Mesh->BodyInstance.SetCollisionProfileName("BlockAllDynamic");

	//this->Mesh->OnComponentHit.AddDynamic(this,&AMyActorForLearn::OnCompontHit);
}

// Called when the game starts or when spawned
void AMyActorForLearn::BeginPlay()
{
	Super::BeginPlay();
#if 0 
	FTimerHandle Timer1;
	FTimerHandle Timer2;
	FTimerHandle Timer3;
	this->GetWorldTimerManager().SetTimer(Timer1,this,&AMyActorForLearn::FunctionReapting,true,true);
	this->GetWorldTimerManager().SetTimer(Timer2,this,&AMyActorForLearn::SimpleHarmonicMotion,0.017,true);
	this->GetWorldTimerManager().SetTimer(Timer3,this,&AMyActorForLearn::MoveRotation,0.017,true);
#endif
		
}

// Called every frame
void AMyActorForLearn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	LineTrace();
}

void AMyActorForLearn::Rotate()
{
	SetActorRotation(GetActorRotation().Add(PitchValue,YawValue,RollValue));
	//解决传参四元素问题	
	//FQuat quatRotation = FQuat();
}

void AMyActorForLearn::FunctionReapting()
{
	GEngine->AddOnScreenDebugMessage(-1,0.5,FColor::Yellow,TEXT("FunctionCall"));
}

void AMyActorForLearn::DebugInfo()
{
	GEngine->AddOnScreenDebugMessage(-1,100.f,FColor::Red,GetName());		
	GEngine->AddOnScreenDebugMessage(-1,100.f,FColor::Red,GetClass()->GetName());

	FVector Pos1 = UGameplayStatics::GetPlayerPawn(GetWorld(),0)->GetActorLocation();
	FVector Pos2 = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	FVector Pos3 =  UGameplayStatics::GetPlayerController(GetWorld(),0)->GetPawn()->GetActorLocation();

	DrawDebugPoint(GetWorld(),GetActorLocation(),50,FColor::Blue,true);
	DrawDebugLine(GetWorld(),Pos1,GetActorLocation(),FColor::Blue,true);
	DrawDebugDirectionalArrow(GetWorld(),Pos1,GetActorLocation(),100,FColor::Yellow,true);
}

void AMyActorForLearn::SimpleHarmonicMotion()
{
	float TotalTime = GetWorld()->GetTimeSeconds();
	float sinNum = FMath::Sin(TotalTime*5.f);
	FVector Location(0.0f,0.0f,5.0f);
	Location *= sinNum;	
	this->AddActorLocalOffset(Location);
}

void AMyActorForLearn::MoveRotation()
{
	this->AddActorLocalOffset(this->GetActorForwardVector()*10);
	this->AddActorWorldRotation(FRotator(0.0f,1.0f,0.0f));
}

void AMyActorForLearn::OnCompontHit(FComponentHitSignature, UPrimitiveComponent OnComponentHit,
	UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	const FHitResult& Hit)
{
	if(IsValid(OtherActor) && OtherActor != this && OtherComp != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1,0.5,FColor::Yellow,TEXT("FunctionCall"));
	}
}

void AMyActorForLearn::LineTrace()
{
	FVector Start = GetActorLocation();
	Start.X += 50.f;
	Start.Z += 100.f;
	FVector End = GetActorForwardVector()*800.f + Start;
	FHitResult outHit;
	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(this);
	DrawDebugLine(GetWorld(),Start,End,FColor::Blue,false);
	//检测一个对象
	if(ActorLineTraceSingle(outHit,Start,End,ECC_Camera,collisionParams)){
		GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Yellow,outHit.GetActor()->GetName());
		GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Yellow,TEXT(" name "));
	}else
	{
		for(auto rc: GetComponents())
		{
			GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Yellow,TEXT(" com "));
		}
		if(IsValid(outHit.GetActor()))
		{
			GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Yellow,TEXT(" Actor "));
		}
	}
	
}

