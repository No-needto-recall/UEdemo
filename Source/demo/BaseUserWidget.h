// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseUserWidget.generated.h"

/**
 * 
 */

//这是一个UI控件的大基类

class UUIManagerSubsystem;
//控件属性的结构体
USTRUCT(BlueprintType)
struct FUIAttribute  
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool IsHideOther = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool IsPersistent = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool IsShowing = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool DontDispose = false;
};

UCLASS()
class DEMO_API UBaseUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual  bool Initialize() override;
	virtual  void NativeConstruct() override;
	
	//基类状态回调函数
	virtual  void Onshow();
	virtual  void OnHide();
	virtual  void OnDispose();
	void SynchronizationUI();
	
public:
	//UPROPERTY(Meta = (BindWidget))
	//class UButton* Button1;

	//UPROPERTY(Meta = (BindWidget))
	//class UTextBlock* TextBlock1;

	//UI的共有属性
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FUIAttribute Attribute;

	//关联的UI
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<TSubclassOf<UBaseUserWidget>> AssociationUI;

	//互斥的UI
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<TSubclassOf<UBaseUserWidget>> MutualExclusionUI;

	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	UUIManagerSubsystem* UIManager;
};
