// Fill out your copyright notice in the Description page of Project Settings.


#include "UIManagerSubsystem.h"
#include "BaseUserWidget.h"
#include "MyCustomLog.h"

void UUIManagerSubsystem::ShowWidgetWithType(TSubclassOf<UBaseUserWidget> Type)
{
	FString Tmp = Type->GetName() + "need show";
	CUSTOM_LOG_INFO(TEXT("%s"),*Tmp);
	UBaseUserWidget** Search = MapForWidget.Find(Type);
	if (Search == nullptr)
	{
		CUSTOM_LOG_INFO(TEXT("展示新的控件"));
		//这个需要展示的控件是新的，需要被创建然后添加。
		UBaseUserWidget* NewWidget = CreateWidget<UBaseUserWidget>
			(GetWorld(), Type);

		if (NewWidget != nullptr)
		{
			//增加键值对
			MapForWidget.Add(Type, NewWidget);
			CUSTOM_LOG_INFO(TEXT("创建新的控件"));
			AddWidgetToView(NewWidget);
		}else
		{
			CUSTOM_LOG_WARNING(TEXT("can't CreateWidget"));
		}
	}
	else
	{
		CUSTOM_LOG_INFO(TEXT("展示已有的控件"));
		AddWidgetToView(*Search);
	}
}

void UUIManagerSubsystem::HideWidgetWithType(TSubclassOf<UBaseUserWidget> Type)
{
	UBaseUserWidget** Search = MapForWidget.Find(Type);
	if (Search != nullptr)
	{
		HideWidget(*Search);
	}
	else
	{
		CUSTOM_LOG_WARNING(TEXT("尝试隐藏widget为不在Map中"));
	}
}

bool UUIManagerSubsystem::SwitchWidgetShowOrHideWithType(TSubclassOf<UBaseUserWidget> Type)
{
	auto Widget = GetUIWidgetWithType(Type);
	if (Widget)
	{
		if (Widget->Attribute.IsShowing)
		{
			HideWidget(Widget);
		}
		else
		{
			AddWidgetToView(Widget);
		}
		return Widget->Attribute.IsShowing;
	}
	else
	{
		ShowWidgetWithType(Type);
		CUSTOM_LOG_ERROR(TEXT("SwitchWidget widget=nullptr"));
		return true;
	}
}

void UUIManagerSubsystem::AddWidgetToView(UBaseUserWidget* Widget)
{
	if (Widget)
	{
		if (Widget->Attribute.IsHideOther)
		{
			HideAllWidget();
		}
		Widget->AddToViewport();
		Widget->Attribute.IsShowing = true;
		Widget->Onshow();
	}
	else
	{
		CUSTOM_LOG_ERROR(TEXT("尝试添加到视口的widget为nullptr"));
	}
}

UBaseUserWidget* UUIManagerSubsystem::GetUIWidgetWithType(TSubclassOf<UBaseUserWidget> Type)
{
	UBaseUserWidget** Search = MapForWidget.Find(Type);
	if (Search != nullptr)
	{
		return *Search;
	}
	CUSTOM_LOG_ERROR(TEXT("尝试获取的widget为不在Map中"));
	UBaseUserWidget* NewWidget = CreateWidget<UBaseUserWidget>(GetWorld(), Type);
	if (NewWidget == nullptr)
	{
		CUSTOM_LOG_WARNING(TEXT("can't CreateWidget"));
		return nullptr;
	}
	MapForWidget.Add(Type, NewWidget);
	return NewWidget;
}

void UUIManagerSubsystem::HideAllWidget()
{
	for (auto& widget : MapForWidget)
	{
		HideWidget(widget.Value);
	}
}

void UUIManagerSubsystem::HideWidget(UBaseUserWidget* Widget)
{
	if (Widget)
	{
		//如果不是持久化UI
		if (!Widget->Attribute.IsPersistent)
		{
			//如果UI是showing状态，则从父项中移除
			if (Widget->Attribute.IsShowing)
			{
				Widget->RemoveFromViewport();
				Widget->Attribute.IsShowing = false;
				Widget->OnHide();
			}
		}
	}
	else
	{
		CUSTOM_LOG_ERROR(TEXT("尝试隐藏的widget为nullptr"));
	}
}
