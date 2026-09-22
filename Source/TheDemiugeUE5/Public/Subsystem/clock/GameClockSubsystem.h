// Copyright 2026 rechenz

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameClockSubsystem.generated.h"

UENUM()
enum class TimeCategory
{
	TICK, //Tick数
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


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeScaleChanged, float, OldTimeScale, float, NewTimeScale);


/**
 * 
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
	int64 GetTickNum(); //获取当前Tick数

	UFUNCTION(BlueprintCallable)
	float GetTimeScale(); //获取当前时速

	UFUNCTION(BlueprintCallable)
	void ChangeTimeScale(float NewTimeScale); //更改时速,并触发广播

	UFUNCTION(BlueprintCallable)
	float GetGameDelta();

	UFUNCTION(BlueprintCallable)
	float GetRealDelta();
#pragma endregion

	UPROPERTY(BlueprintAssignable)
	FOnTimeScaleChanged OnTimeScaleChanged; //广播时速更改

	UFUNCTION()
	void AdvanceGameTime(double Secs, float NewTimeScale);

#pragma region StaticValue

	UPROPERTY(BlueprintReadWrite)
	int32 DayTime = 86400;

	UPROPERTY(BlueprintReadWrite)
	int32 ClockTimes; //请保证可以把DayTime整除，否则不保证稳定性
#pragma endregion

#pragma region virtualTick
	virtual void Tick(float DeltaTime) override;

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
	double GameTime;
	int64 TickNum;
	float TimeScale;
	float GameDelta;
	float RealDelta;
};
