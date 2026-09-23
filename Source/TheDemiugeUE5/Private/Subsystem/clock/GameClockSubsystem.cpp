// Copyright 2026 rechenz


#include "Subsystem/clock/GameClockSubsystem.h"


UGameClockSubsystem::UGameClockSubsystem()
{
	GameTime = 0;
	TickNum = 0;
	TimeScale = 1;
	CurrentSlotTime = 0;
}

void UGameClockSubsystem::Tick(float DeltaTime)
{
	GameDelta = DeltaTime * TimeScale;
	RealDelta = DeltaTime;

	Accumulator += DeltaTime * TimeScale;
	while (Accumulator >= FixStep)
	{
		FixTick();
		Accumulator -= FixStep;
	}
	GameTime = TickNum * FixStep;

	CurrentSlotTime += DeltaTime * TimeScale;
	while (CurrentSlotTime >= SlotTime)
	{
		Slot++;
		CurrentSlotTime -= SlotTime;
		OnSlotChanged(Slot);
	}
}

void UGameClockSubsystem::FixTick()
{
	TickNum++;
}

double UGameClockSubsystem::GetTime()
{
	return GameTime;
}

int64 UGameClockSubsystem::GetTickNum()
{
	return TickNum;
}

FFormatTime UGameClockSubsystem::GetFormatTime()
{
	return FFormatTime(GameTime);
}

void UGameClockSubsystem::ChangeTimeScale(float NewTimeScale)
{
	if (NewTimeScale == TimeScale)
	{
		return;
	}
	float OldTimeScale = TimeScale;
	TimeScale = NewTimeScale;
	OnTimeScaleChanged.Broadcast(OldTimeScale, NewTimeScale);
}

float UGameClockSubsystem::GetTimeScale()
{
	return TimeScale;
}

float UGameClockSubsystem::GetGameDelta()
{
	return GameDelta;
}

float UGameClockSubsystem::GetRealDelta()
{
	return RealDelta;
}

void UGameClockSubsystem::FireSlot(int32 curSlot)
{
	TArray<FTimeEvent> TempArray;
	EventContainer.MultiFind(curSlot, TempArray);
	for (FTimeEvent& Event : TempArray)
	{
		Event.CallBack(curSlot);
	}
	EventContainer.Remove(curSlot);
	UE_LOG(LogTemp, Log, TEXT("Fire 了 %d 个注册事件"), TempArray.Num());
}

int32 UGameClockSubsystem::TimeEventRegister(int32 TriggerSlot, int32 PriorityLevel,
                                             const TFunction<void(int32)>& Callback)
{
	int32 NewId = ++TimeEventId;
	EventContainer.Emplace(
		TriggerSlot,
		FTimeEvent(NewId, TriggerSlot, PriorityLevel, MoveTemp(Callback))
	);
	return NewId;
}

void UGameClockSubsystem::OnSlotChanged(int32 curSlot)
{
	FireSlot(curSlot);
}

double UGameClockSubsystem::GetSlotTime()
{
	return SlotTime;
}

void UGameClockSubsystem::SetGameTime(double NewGameTime)
{
	if (NewGameTime >= GameTime)
	{
		UE_LOG(LogTemp, Error, TEXT("你应该使用 AdvanceGameTIme 来推进时间"));
		return;
	}

	GameTime = NewGameTime;
	AlignToGameTime();
}

void UGameClockSubsystem::AlignToGameTime()
{
	UE_LOG(LogTemp, Warning, TEXT("GameTime被更改，正在尝试对齐"));
	/*需要对齐的变量如下
	 * CurrentSlotTime done
	 * Accumulator
	 * Slot done
	 * TickNum
	 */

	Slot = FMath::CeilToInt32(GameTime / SlotTime);
	CurrentSlotTime = FMath::Fmod(GameTime, SlotTime);

	TickNum = FMath::CeilToInt64(GameTime / FixStep);
	Accumulator = FMath::Fmod(GameTime, FixStep);
}

void UGameClockSubsystem::AdvanceGameTime(double Secs)
{
	while (Secs >= SlotTime)
	{
		Secs -= SlotTime;
		Slot++;
		FireSlot(Slot);
	}
}

void UGameClockSubsystem::EnsureTimeSetAvailable() const
{
	if (DayTime % ClockTimes != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("DayTime 必须可以被 ClockTimes 整除，否则不保证稳定性"));
	}
}
