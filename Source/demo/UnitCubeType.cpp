// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitCubeType.h"

#include "InstancedMesh.h"
#include "UnitCube.h"
#include "Chaos/AABB.h"

UUnitCubeType::~UUnitCubeType()
{
}

TSharedPtr<UUnitCubeType> UUnitCubeType::BuildUnitCubeType(const EUnitCubeType& Type)
{
	TSharedPtr<UUnitCubeType> Result = nullptr;
	switch (Type) {
	case EUnitCubeType::Stone:
		Result = MakeShareable(new UUnitCubeType_Stone());
		break;
	case EUnitCubeType::Grass:
		Result = MakeShareable(new UUnitCubeType_Grass());
		break;
	case EUnitCubeType::BedRock:
		Result = MakeShareable(new UUnitCubeType_BedRock());
		break;
	case EUnitCubeType::OakLog:
		Result = MakeShareable(new UUnitCubeType_OakLog());
		break;
	case EUnitCubeType::OakPlanks:
		Result = MakeShareable(new UUnitCubeType_OakPlanks());
		break;
	case EUnitCubeType::OakLeaves:
		Result = MakeShareable(new UUnitCubeType_OakLeaves());
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

EUnitCubeType UUnitCubeType::GetTypeEnum() const
{
	return EUnitCubeType::Stone;
}

bool UUnitCubeType::IsSolid() const
{
	return true;
}

EInstancedMeshType UUnitCubeType_Stone::GetMeshType(const EFaceDirection& Direction)
{
	return EInstancedMeshType::StoneMesh;
}

EUnitCubeType UUnitCubeType_Stone::GetTypeEnum() const
{
	return EUnitCubeType::Stone;
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
	default:
		return  EInstancedMeshType::GrassSideMesh;
	}
}

EUnitCubeType UUnitCubeType_Grass::GetTypeEnum() const
{
	return EUnitCubeType::Grass;
}

EInstancedMeshType UUnitCubeType_BedRock::GetMeshType(const EFaceDirection& Direction)
{
	return EInstancedMeshType::BedrockMesh;
}

EUnitCubeType UUnitCubeType_BedRock::GetTypeEnum() const
{
	return EUnitCubeType::BedRock;
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

EUnitCubeType UUnitCubeType_OakLog::GetTypeEnum() const
{
	return EUnitCubeType::OakLog;
}

EInstancedMeshType UUnitCubeType_OakPlanks::GetMeshType(const EFaceDirection& Direction)
{
	return  EInstancedMeshType::OakPlanksMesh;
}

EUnitCubeType UUnitCubeType_OakPlanks::GetTypeEnum() const
{
	return EUnitCubeType::OakPlanks;
}

EInstancedMeshType UUnitCubeType_OakLeaves::GetMeshType(const EFaceDirection& Direction)
{
	return EInstancedMeshType::OakLeavesMesh;
}

EUnitCubeType UUnitCubeType_OakLeaves::GetTypeEnum() const
{
	return EUnitCubeType::OakLeaves;
}

bool UUnitCubeType_OakLeaves::IsSolid() const
{
	return false;
}
