// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUserWidget.h"

#include "MyCustomLog.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "UIManagerSubsystem.h"

bool UBaseUserWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	return true;
}

void UBaseUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UIManager = GetWorld()->GetGameInstance()->GetSubsystem<UUIManagerSubsystem>();
	if (UIManager == nullptr)
	{
		CUSTOM_LOG_WARNING(TEXT("UIManager为空"));
	}
}

void UBaseUserWidget::Onshow()
{
	SynchronizationUI();
}

void UBaseUserWidget::OnHide()
{
}

void UBaseUserWidget::OnDispose()
{
}

void UBaseUserWidget::SynchronizationUI()
{
	//关联的UI
	for (const auto& UI : AssociationUI)
	{
		const auto RealUI = UIManager->GetUIWidgetWithType(UI);
		if (RealUI)
		{
			if (!(RealUI->Attribute.IsShowing))
			{
				UIManager->ShowWidgetWithType(UI);
			}
		}else
		{
				UIManager->ShowWidgetWithType(UI);
		}
	}
	//互斥的UI
	for (const auto& UI : MutualExclusionUI)
	{
		const auto RealUI = UIManager->GetUIWidgetWithType(UI);
		if (RealUI)
		{
			if (RealUI->Attribute.IsShowing)
			{
				UIManager->HideWidgetWithType(UI);
			}
		}
	}
}
