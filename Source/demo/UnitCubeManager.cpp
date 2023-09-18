#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

#include "UnitCubeManager.h"
#include "MeshManager.h"
#include "UnitCube.h"
#include "UnitCubeType.h"

// Sets default values
AUnitCubeManager::AUnitCubeManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshManager = nullptr;
}

// Called when the game starts or when spawned
void AUnitCubeManager::BeginPlay()
{
	Super::BeginPlay();
	BuildMeshManager();
	BuildMap();
	BuildAllCubesMesh();
	//更新网格体的实例变换
	MeshManager->UpdateAllInstancedMesh();
	for(int x=0;x<Size.X;++x)
	{
		for(int y=0;y<Size.Y;++y)
		{
			float NoiseValue = 2.5;
			NoiseValue = stb_perlin_fbm_noise3(x*0.1f,y*0.1f,0,8,2.0f,5);
			UE_LOG(LogTemp, Log, TEXT("val:%.5f"), NoiseValue);
		}	
	}
	float Value = stb_perlin_fbm_noise3(1.0f,1.0f, 0, 8, 2.0f, 5);
	UE_LOG(LogTemp, Log, TEXT("after val:%.5f"), Value);
}

// Called every frame
void AUnitCubeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUnitCubeManager::BuildMeshManager()
{
	AMeshManager* NewMeshManager = GetWorld()->SpawnActor<AMeshManager>
		(AMeshManager::StaticClass(), FVector(0.0f), FRotator(0.0f));
	if (NewMeshManager)
	{
		MeshManager = NewMeshManager;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("can't spawn MeshManager"));
	}
}

void AUnitCubeManager::BuildMap()
{
	AUnitCube* NewCube = nullptr;

	for (int i = 0; i < Size.Z; ++i)
	{
		for (int k = 0; k < Size.Y; ++k)
		{
			for (int j = 0; j < Size.X; ++j)
			{
				NewCube = GetWorld()->SpawnActor<AUnitCube>(AUnitCube::StaticClass(),
				                                            MapToScene(FIntVector(j, k, i)), FRotator(0.0f));
				NewCube->CubeType = UUnitCubeType::BuildUnitCubeType(EUnitCubeType::Stone);
				WorldMap.Add(FIntVector(j, k, i), NewCube);
			}
		}
	}
}

void AUnitCubeManager::BuildAllCubesMesh()
{
	//遍历地图
	for (const auto& Pair : WorldMap)
	{
		FIntVector CurrentPosition = Pair.Key;
		AUnitCube* CurrentCube = Pair.Value;
		if (CurrentCube && IsValid(CurrentCube) && CurrentCube->IsSolid()) //检查方块是否存在，且是实体
		{
			int EnabledCollision = 0;
			uint8 MeshType = 0;
			for (const FIntVector& Dir : Directions)
			{
				FIntVector NeighbourPosition = CurrentPosition + Dir;
				AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
				//因为所有cube默认没有静态网格体实例，所以检测存在不为实心||不存在
				if (NeighbourCube && IsValid((*NeighbourCube)))
				{
					if ((*NeighbourCube)->IsTransparent())
					{
						//在对应的Dir添加静态网格体实例
						MeshManager->AddMeshToCubeWith(Dir, CurrentCube);
					}
					else
					{
						++EnabledCollision;
					}
				}
				else
				{
					MeshManager->AddMeshToCubeWith(Dir, CurrentCube);
				}
				++MeshType;
			}
			//循环结束后，如果方块没被实心体包围，则开启碰撞
			if (EnabledCollision != 6)
			{
				CurrentCube->SetTheCollisionOfTheBoxToBeEnabled(true);
			}
		}
	}
}

void AUnitCubeManager::UpDateCubeMeshWith(const FIntVector& Key)
{
	AUnitCube** CurrentCube = WorldMap.Find(Key);
	//如果Key存在
	if (CurrentCube && IsValid((*CurrentCube)))
	{
		uint8 MeshType = 0;
		for (const FIntVector& Dir : Directions)
		{
			FIntVector NeighbourPosition = Key + Dir;
			AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
			if (NeighbourCube && IsValid((*NeighbourCube)))
			{
				//邻居存在
				if ((*NeighbourCube)->IsSolid())
				{
					//邻居是实心的
					//设置对应的面不可见
					MeshManager->DelMeshToCubeWith(Dir, (*CurrentCube));
				}
				else
				{
					MeshManager->AddMeshToCubeWith(Dir, (*CurrentCube));
				}
			}
			else
			{
				//邻居不存在
				MeshManager->AddMeshToCubeWith(Dir, (*CurrentCube));
			}
			++MeshType;
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("The location does not exist in the map: (%s)"), *Key.ToString());
	}
}

void AUnitCubeManager::UpDateCubeMeshWith(AUnitCube* Cube)
{
	if (Cube && IsValid(Cube))
	{
		const FIntVector Key = SceneToMap(Cube->GetActorLocation());
		UpDateCubeMeshWith(Key);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("This Actor does not exist"));
	}
}

FVector AUnitCubeManager::MapToScene(const FIntVector& MapCoord)
{
	return FVector(100 * MapCoord.X, 100 * MapCoord.Y, 100 * MapCoord.Z);
}

FIntVector AUnitCubeManager::SceneToMap(const FVector& Scene)
{
	return FIntVector(Scene.X / 100, Scene.Y / 100, Scene.Z / 100);
}

void AUnitCubeManager::AddCubeWith(const FVector& Scene, const int& Type)
{
	if (!IsLock)
	{
		IsLock = true;
		FIntVector Key = SceneToMap(Scene);
		auto NewCube = GetWorld()->SpawnActor<AUnitCube>(AUnitCube::StaticClass(),
		                                                 MapToScene(Key), FRotator(0.0f));
		NewCube->CubeType = UUnitCubeType::BuildUnitCubeType(static_cast<EUnitCubeType>(Type));
		//添加后配置自身的可视性
		WorldMap.Add(Key, NewCube);
		UpDateCubeMeshWith(Key);
		//检查以刚方块为中心的方块。
		for (const FIntVector& Dir : Directions)
		{
			FIntVector NeighbourPosition = Key + Dir;
			AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
			if (NeighbourCube)
			{
				//如果邻居存在，更新邻居的隐藏配置
				UpDateCubeMeshWith(NeighbourPosition);
				//更新邻居的碰撞
				(*NeighbourCube)->RefreshCollisionEnabled();
			}
		}
		MeshManager->UpdateAllInstancedMesh();
		NewCube->RefreshCollisionEnabled();
		IsLock = false;
	}
}

void AUnitCubeManager::DelCubeWith(const FVector& Scene)
{
	FIntVector Key = SceneToMap(Scene);
	AUnitCube** Cube = WorldMap.Find(Key);
	if (Cube) //如果有找到的
	{
		//移除
		WorldMap.Remove(Key);
		HiedCubeAllFace(*Cube);
		//刷新周围
		for (const FIntVector& Dir : Directions)
		{
			FIntVector NeighbourPosition = Key + Dir;
			AUnitCube** NeighbourCube = WorldMap.Find(NeighbourPosition);
			if (NeighbourCube)
			{
				//如果邻居存在，更新隐藏配置
				UpDateCubeMeshWith(NeighbourPosition);
				//更新邻居的碰撞
				(*NeighbourCube)->RefreshCollisionEnabled();
			}
		}
		//Cube销毁
		(*Cube)->OnDestroyed();
		//刷新遮挡
		FlushRenderingCommands();
		MeshManager->UpdateAllInstancedMesh();
	}
	else
	{
		//Key不存在：
		UE_LOG(LogTemp, Log, TEXT("The location does not exist in the map: (%s)"), *Key.ToString());
	}
}

void AUnitCubeManager::HiedCubeAllFace(AUnitCube* Cube)
{
	if(Cube && IsValid(Cube))
	{
		for(const FIntVector&Dir :Directions)
		{
			MeshManager->DelMeshToCubeWith(Dir,Cube);
		}
	}
}
