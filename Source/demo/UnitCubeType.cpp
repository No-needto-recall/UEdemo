// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitCubeType.h"

#include "InstancedMesh.h"
#include "MyCustomLog.h"
#include "UnitCube.h"
#include "Chaos/AABB.h"

TSharedPtr<FUnitCubeType> FUnitCubeTypeManager::GetUnitCubeType(const EUnitCubeType& Type)
{
	auto Search = TypeMap.Find(Type);
	if(Search)
	{
		return *Search;
	}else
	{
		auto Ptr = FUnitCubeType::BuildUnitCubeType(Type);
		TypeMap.Add(Type,Ptr);
		return Ptr;
	}
}

FUnitCubeType::~FUnitCubeType()
{
}

TSharedPtr<FUnitCubeType> FUnitCubeType::BuildUnitCubeType(const EUnitCubeType& Type)
{
	TSharedPtr<FUnitCubeType> Result = nullptr;
	switch (Type) {
	case EUnitCubeType::Stone:
		Result = MakeShareable(new FUnitCubeType_Stone());
		break;
	case EUnitCubeType::Grass:
		Result = MakeShareable(new FUnitCubeType_Grass());
		break;
	case EUnitCubeType::BedRock:
		Result = MakeShareable(new FUnitCubeType_BedRock());
		break;
	case EUnitCubeType::OakLog:
		Result = MakeShareable(new FUnitCubeType_OakLog());
		break;
	case EUnitCubeType::OakPlanks:
		Result = MakeShareable(new FUnitCubeType_OakPlanks());
		break;
	case EUnitCubeType::OakLeaves:
		Result = MakeShareable(new FUnitCubeType_OakLeaves());
		break;
	default:
		CUSTOM_LOG_INFO(TEXT("Wrong Cube Type"));
	}
	return Result;
}

bool FUnitCubeType::IsTransparent(const EUnitCubeType& Type)
{
	return Type == EUnitCubeType::OakLeaves;
}

EInstancedMeshType FUnitCubeType::GetMeshType(const EFaceDirection& Direction)
{
	return EInstancedMeshType::StoneMesh;
}

EUnitCubeType FUnitCubeType::GetTypeEnum() const
{
	return EUnitCubeType::Stone;
}

bool FUnitCubeType::IsSolid() const
{
	return true;
}

EInstancedMeshType FUnitCubeType_Stone::GetMeshType(const EFaceDirection& Direction)
{
	return EInstancedMeshType::StoneMesh;
}

EUnitCubeType FUnitCubeType_Stone::GetTypeEnum() const
{
	return EUnitCubeType::Stone;
}


EInstancedMeshType FUnitCubeType_Grass::GetMeshType(const EFaceDirection& Direction)
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

EUnitCubeType FUnitCubeType_Grass::GetTypeEnum() const
{
	return EUnitCubeType::Grass;
}

EInstancedMeshType FUnitCubeType_BedRock::GetMeshType(const EFaceDirection& Direction)
{
	return EInstancedMeshType::BedrockMesh;
}

EUnitCubeType FUnitCubeType_BedRock::GetTypeEnum() const
{
	return EUnitCubeType::BedRock;
}

EInstancedMeshType FUnitCubeType_OakLog::GetMeshType(const EFaceDirection& Direction)
{
	if(Direction == EFaceDirection::Bottom || Direction == EFaceDirection::Top)
	{
		return EInstancedMeshType::OakLogTopBottomMesh;
	}else
	{
		return EInstancedMeshType::OakLogSideMesh;
	}
}

EUnitCubeType FUnitCubeType_OakLog::GetTypeEnum() const
{
	return EUnitCubeType::OakLog;
}

EInstancedMeshType FUnitCubeType_OakPlanks::GetMeshType(const EFaceDirection& Direction)
{
	return  EInstancedMeshType::OakPlanksMesh;
}

EUnitCubeType FUnitCubeType_OakPlanks::GetTypeEnum() const
{
	return EUnitCubeType::OakPlanks;
}

EInstancedMeshType FUnitCubeType_OakLeaves::GetMeshType(const EFaceDirection& Direction)
{
	return EInstancedMeshType::OakLeavesMesh;
}

EUnitCubeType FUnitCubeType_OakLeaves::GetTypeEnum() const
{
	return EUnitCubeType::OakLeaves;
}

bool FUnitCubeType_OakLeaves::IsSolid() const
{
	return false;
}
