// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/Actor.h"
#include "MyActorForLearn.generated.h"



UCLASS()
class DEMO_API AMyActorForLearn : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMyActorForLearn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	//无参数，无返回委托
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegate_1);
	UPROPERTY(BlueprintAssignable)
	FDelegate_1 Fdel1;
	
	//单播
	DECLARE_DELEGATE(FDelegate_2);
	FDelegate_2 Fdel2;
	
	//静态网格组件
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category=Movement)
	float PitchValue;

	UPROPERTY(EditAnywhere, Category=Movement)
	float YawValue;

	UPROPERTY(EditAnywhere, Category=Movement)
	float RollValue;

	void Rotate();

	void FunctionReapting();

	void DebugInfo();

	void SimpleHarmonicMotion();

	void MoveRotation();

	void OnCompontHit(FComponentHitSignature, UPrimitiveComponent OnComponentHit, UPrimitiveComponent* HitComponent,
	                  AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	void LineTrace();

	void LearnFunc_FindObject();

	void SharedPtrFunc();

	FStreamableManager MyStreamableManager;

	void LoadMesh();
};


#if 1
class TestClass
{
public:
	TestClass(): A(1)
	{
	}

	~TestClass()
	{
	}

	int A;
};
#endif
