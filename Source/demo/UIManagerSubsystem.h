// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyCustomLog.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UIManagerSubsystem.generated.h"

/**
 * 
 */

//控件基类的前置声明
class UBaseUserWidget;

UCLASS()
class DEMO_API UUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//对外接口
	
	//根据类引用展示UI
	UFUNCTION(BlueprintCallable,Category = "UIManagerCPP")
	void ShowWidgetWithType(TSubclassOf<UBaseUserWidget> Type);
	//根据类引用隐藏UI
	UFUNCTION(BlueprintCallable,Category = "UIManagerCPP")
	void HideWidgetWithType(TSubclassOf<UBaseUserWidget> Type);
	//根据类引用切换UI状态
	UFUNCTION(BlueprintCallable,Category = "UIManagerCPP")
	bool SwitchWidgetShowOrHideWithType(TSubclassOf<UBaseUserWidget> Type);
	//隐藏所有UI
	UFUNCTION(BlueprintCallable,Category = "UIManagerCPP")
	void HideAllWidget();
	//隐藏UI
	void HideWidget(UBaseUserWidget* Widget);
	//添加UI到视口
	void AddWidgetToView(UBaseUserWidget* Widget);

	//通过类引用获取UI
	UFUNCTION(BlueprintCallable,Category = "UIManagerCPP")
	UBaseUserWidget* GetUIWidgetWithType(TSubclassOf<UBaseUserWidget> Type);

private:
	//注意保存是指针
	TMap<UClass*,UBaseUserWidget*> MapForWidget;	
};
