// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	//静态网格组件
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere,Category=Movement)
	float PitchValue;
	
	UPROPERTY(EditAnywhere,Category=Movement)
	float YawValue;
	
	UPROPERTY(EditAnywhere,Category=Movement)
	float RollValue;

	void Rotate();

	void FunctionReapting();

	void DebugInfo();

	void SimpleHarmonicMotion();
	
	void MoveRotation();

	void OnCompontHit(FComponentHitSignature, UPrimitiveComponent OnComponentHit, UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse ,const FHitResult& Hit );

	
	void LineTrace();

};
