// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitCubeType.h"

#include "InstancedMesh.h"
#include "UnitCube.h"

UUnitCubeType* UUnitCubeType::BuildUnitCubeType(const EUnitCubeType& Type)
{
	UUnitCubeType* Result = nullptr;
	switch (Type) {
	case EUnitCubeType::Stone:
		Result = NewObject<UUnitCubeType_Stone>();
		break;
	case EUnitCubeType::Grass:
		Result = NewObject<UUnitCubeType_Grass>();
		break;
	case EUnitCubeType::BedRock:
		Result = NewObject<UUnitCubeType_BedRock>();
		break;
	case EUnitCubeType::OakLog:
		Result = NewObject<UUnitCubeType_OakLog>();
		break;
	case EUnitCubeType::OakPlanks:
		Result = NewObject<UUnitCubeType_OakPlanks>();
		break;
	default:
		UE_LOG(LogTemp,Log,TEXT("Wrong Cube Type"));
	}
	return Result;
}

EInstancedMeshType UUnitCubeType::GetMeshType(const EFaceDirection& Direction)
{
	return EInstancedMeshType::StoneMesh;
}

EInstancedMeshType UUnitCubeType_Stone::GetMeshType(const EFaceDirection& Direction)
{
	return EInstancedMeshType::StoneMesh;
}

EInstancedMeshType UUnitCubeType_Grass::GetMeshType(const EFaceDirection& Direction)
{
	switch (Direction)
	{
	case Top:
		return EInstancedMeshType::GrassTopMesh;
	case Bottom:
		return EInstancedMeshType::GrassBottomMesh;
	case Front:
		return EInstancedMeshType::GrassSideMesh;
	case Back:
		return EInstancedMeshType::GrassSideMesh;
	case Right:
		return EInstancedMeshType::GrassSideMesh;
	case Left:
		return EInstancedMeshType::GrassSideMesh;
	case DirectionSize:
		return Super::GetMeshType(Direction);
	default:
		return  EInstancedMeshType::GrassSideMesh;
	}
}

EInstancedMeshType UUnitCubeType_BedRock::GetMeshType(const EFaceDirection& Direction)
{
	return EInstancedMeshType::BedrockMesh;
}

EInstancedMeshType UUnitCubeType_OakLog::GetMeshType(const EFaceDirection& Direction)
{
	if(Direction == EFaceDirection::Bottom || Direction == EFaceDirection::Top)
	{
		return EInstancedMeshType::OakLogTopBottomMesh;
	}else
	{
		return EInstancedMeshType::OakLogSideMesh;
	}
}

EInstancedMeshType UUnitCubeType_OakPlanks::GetMeshType(const EFaceDirection& Direction)
{
	return  EInstancedMeshType::OakPlanksMesh;
}
