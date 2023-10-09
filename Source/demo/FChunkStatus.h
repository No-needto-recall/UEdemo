#pragma once

#include "CoreMinimal.h"



class DEMO_API FChunkStatus :public TSharedFromThis<FChunkStatus>
{
public:
	FChunkStatus();

	enum EResourceState 
	{
		None ,
		DataLoaded ,
		CubeAllocated ,
		MeshAllocated , 
	};
	enum EMarkState
	{
		NeedLoad,
		NeedUnload,
		Idle,//对应MeshAllocated
	};
	void MarkToLoad();
	void MarkToUnload();
	void MarkToIdle();
	bool IsMarkedForLoad() const;
	bool IsMarkedForUnload() const;
	bool IsMarkedForIdle() const;
	void MoveToNextResourceState();
	void ResetResourceState();
	EResourceState GetCurrentResourceState() const;
private:
	EResourceState CurrentResourceState;
	EMarkState MarkState;
};


