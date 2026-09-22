// Copyright 2026 rechenz


#include "Subsystem/clock/GameClockSubsystem.h"


UGameClockSubsystem::UGameClockSubsystem()
{
	GameTime = 0;
	TickNum = 0;
	TimeScale = 1;
}

void UGameClockSubsystem::Tick(float DeltaTime)
{
	TickNum++;
	GameTime += DeltaTime * TimeScale;

	GameDelta = DeltaTime * TimeScale;
	RealDelta = DeltaTime;
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

void UGameClockSubsystem::AdvanceGameTime(double Secs, float NewTimeScale = 5)
{
	ChangeTimeScale(NewTimeScale);
}
