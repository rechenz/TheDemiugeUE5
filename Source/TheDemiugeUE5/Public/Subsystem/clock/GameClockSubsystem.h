// Copyright 2026 rechenz

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameClockSubsystem.generated.h"

UENUM()
enum class ETimeCategory
{
	TICK, //逻辑帧数
	TIME, //以秒为单位的时间
	FORMATTIME, //以小时，分钟，秒的格式
};

USTRUCT(BlueprintType)
struct FFormatTime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock")
	int32 Days = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock")
	int32 Hours = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock")
	int32 Minutes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock")
	int32 Seconds = 0;

	FFormatTime() = default;

	explicit FFormatTime(double sec, int32 SecondsPerDay = 86400)
	{
		int32 secs = static_cast<int32>(sec);
		Days = secs / SecondsPerDay;
		secs -= Days * SecondsPerDay;
		Hours = secs / 3600;
		Minutes = (secs % 3600) / 60;
		Seconds = secs % 60;
	}
};

USTRUCT()
struct FTimeEvent
{
	GENERATED_BODY()

	int32 Id = INDEX_NONE;

	int32 TriggerSlot = INDEX_NONE;

	int32 Priority_Level = 0;

	TFunction<void(int32)> CallBack;

	FTimeEvent() = default;

	FTimeEvent(int32 id, int32 slot, int32 priLevel, const TFunction<void(int32)>& func) : Id(id), TriggerSlot(slot),
		Priority_Level(priLevel)
	{
		CallBack = func;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeScaleChanged, float, OldTimeScale, float, NewTimeScale);


/**
 *此文件用于管理游戏中的时间系统，包括逻辑时间、现实时间和事件触发等
 *下方暴露的所有接口都可以在蓝图中使用
 *请在查看函数功能后调用
 *保证你知道你在干什么
 */
UCLASS()
class THEDEMIUGEUE5_API UGameClockSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UGameClockSubsystem();

#pragma region Basic
	UFUNCTION(BlueprintCallable)
	double GetTime(); //获取当前时间秒

	UFUNCTION(BlueprintCallable)
	FFormatTime GetFormatTime(); //获取格式化之后的时间

	UFUNCTION(BlueprintCallable)
	int64 GetTickNum(); //获取当前逻辑帧数量

	UFUNCTION(BlueprintCallable)
	float GetTimeScale(); //获取当前时速

	UFUNCTION(BlueprintCallable)
	void ChangeTimeScale(float NewTimeScale); //更改时速,并触发广播

	UFUNCTION(BlueprintCallable)
	float GetGameDelta(); //获取游戏帧时间

	UFUNCTION(BlueprintCallable)
	float GetRealDelta(); //获取真实帧时间

	UPROPERTY(BlueprintReadWrite)
	double Accumulator = 0; //记录当前经过时间
#pragma endregion

	UPROPERTY(BlueprintAssignable)
	FOnTimeScaleChanged OnTimeScaleChanged; //广播时速更改

	UFUNCTION(BlueprintCallable)
	void AdvanceGameTime(double Secs); //跳过人生功能

	UFUNCTION(BlueprintCallable)
	void SetGameTime(double NewGameTime); //手动设置当前游戏时间，使用之前请保证你知道你在做什么！！！

#pragma region StaticValue

	UPROPERTY(BlueprintReadWrite)
	int32 DayTime = 86400; //请不要动态更改这个值

	UPROPERTY(BlueprintReadWrite)
	int32 ClockTimes = 24; //请保证可以把DayTime整除，否则不保证稳定性，请不要更改这个值

	UPROPERTY(BlueprintReadWrite)
	double FixStep = 0.05; //逻辑步长

#pragma endregion

#pragma region Slot

	UPROPERTY(BlueprintReadOnly)
	int32 Slot = 1; //游戏总共走了多少时间刻	

	UFUNCTION(BlueprintCallable)
	double GetSlotTime();

	UFUNCTION(BlueprintCallable)
	void FireSlot(int32 curSlot); //触发这个时间刻的所有监听

	int32 TimeEventRegister(int32 TriggerSlot, int32 PriorityLevel, const TFunction<void(int32)>& Callback);

#pragma endregion

	UFUNCTION(BlueprintCallable)
	void AlignToGameTime();

#pragma region virtualTick
	virtual void Tick(float DeltaTime) override;

	void FixTick();

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UGameClockSubsystem, STATGROUP_Tickables);
	}


	virtual bool IsTickable() const override
	{
		return GetGameInstance() != nullptr; //保证不会在游戏进程之前Tick
	}

	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual bool IsTickableInEditor() const override { return false; }

	virtual UWorld* GetTickableGameObjectWorld() const override
	{
		return nullptr; // 全局对象,不属于任何 World
	}
#pragma endregion

private:
	double CurrentSlotTime;

	double SlotTime;

	double GameTime;

	int64 TickNum = 1;

	float TimeScale;

	float GameDelta;

	float RealDelta;

#pragma region privateSlot

	void OnSlotChanged(int32 curSlot);

	void EnsureTimeSetAvailable() const;

	int32 TimeEventId = 0;

	TMultiMap<int32, FTimeEvent> EventContainer;

#pragma endregion
};
