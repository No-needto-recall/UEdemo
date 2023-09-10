// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitCubeType.h"

#include "InstancedMesh.h"

UUnitCubeType* UUnitCubeType::BuildUnitCubeType(const EUnitCubeType& Type)
{
	UUnitCubeType* Result = nullptr;
	switch (Type) {
	case EUnitCubeType::Stone:
		Result = NewObject<UUnitCubeType_Stone>();
		break;
	default:
		UE_LOG(LogTemp,Log,TEXT("Wrong Cube Type"));
		;
	}
	return Result;
}

EInstancedMeshType UUnitCubeType::GetMeshType(const EFaceDirection& Direction)
{
	return EInstancedMeshType::StoneMesh;
}

EInstancedMeshType UUnitCubeType_Stone::GetMeshType(const EFaceDirection& Direction)
{
	return Super::GetMeshType(Direction);
}
