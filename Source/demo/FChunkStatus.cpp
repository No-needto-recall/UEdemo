#include "FChunkStatus.h"

// 构造函数，初始化为None和Idle
FChunkStatus::FChunkStatus() : CurrentResourceState(None), MarkState(NeedLoad)
{
}

// 标记为需加载
void FChunkStatus::MarkToLoad()
{
	MarkState = NeedLoad;
}

// 标记为需卸载
void FChunkStatus::MarkToUnload()
{
	MarkState = NeedUnload;
}

// 标记为空闲
void FChunkStatus::MarkToIdle()
{
	MarkState = Idle;
}

// 检查是否标记为需加载
bool FChunkStatus::IsMarkedForLoad() const
{
	return MarkState == NeedLoad;
}

// 检查是否标记为需卸载
bool FChunkStatus::IsMarkedForUnload() const
{
	return MarkState == NeedUnload;
}

// 检查是否标记为空闲
bool FChunkStatus::IsMarkedForIdle() const
{
	return MarkState == Idle;
}

// 进入下一个资源状态，这里只是简单地递增枚举值
void FChunkStatus::MoveToNextResourceState()
{
	if(MarkState == Idle){
		return;
	}
	if(MarkState == NeedLoad)
	{
		if (CurrentResourceState < MeshAllocated) // 检查当前状态是否已经是最大值
		{
			CurrentResourceState = static_cast<EResourceState>(CurrentResourceState + 1);
			if(CurrentResourceState == MeshAllocated)
			{
				MarkState = Idle;
			}
		}
	}else if(MarkState == NeedUnload)
	{
		if (CurrentResourceState > None) // 检查当前状态是否已经是最大值
		{
			CurrentResourceState = static_cast<EResourceState>(CurrentResourceState - 1);
			if (CurrentResourceState == MeshAllocated)
			{
				MarkState = Idle;
			}
		}
	}
}

// 重置资源状态为None
void FChunkStatus::ResetResourceState()
{
	CurrentResourceState = None;
}

// 获取当前资源状态
FChunkStatus::EResourceState FChunkStatus::GetCurrentResourceState() const
{
	return CurrentResourceState;
}

