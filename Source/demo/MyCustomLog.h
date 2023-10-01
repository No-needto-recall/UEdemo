// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
/**
 * 
 */

DECLARE_LOG_CATEGORY_EXTERN(MyCustomLog,Log,All);

#define CUSTOM_LOG(Verbosity, Format, ...) \
UE_LOG(MyCustomLog, Verbosity, TEXT("%s %s %d: " Format), \
ANSI_TO_TCHAR(__FILE__), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, ##__VA_ARGS__)

#define CUSTOM_LOG_WARNING(Format, ...) CUSTOM_LOG(Log, Format, ##__VA_ARGS__)
#define CUSTOM_LOG_ERROR(Format, ...) CUSTOM_LOG(Error, Format, ##__VA_ARGS__)
