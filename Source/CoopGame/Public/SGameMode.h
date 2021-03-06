// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

enum class EWaveState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor, AController*, KillerController);

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ASGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(BlueprintAssignable, Category = "GameMode")
    FOnActorKilled OnActorKilled;

protected:

	// Time to prepare between waves
	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
    float TimeBetweenWaves;
	
	// Bots to spawn in current wave
    int32 NrOfBotsToSpawn;

	// Counter to keep track of the current wave
	int32 WaveCount;

	// Timerhandle that will call the function to create bots
	FTimerHandle TimerHandle_BotSpawner;

	// Timerhandle to start next wave
	FTimerHandle TimerHandle_NextWaveStart;

	// Hook BP to spawn a single bot
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBotTimerElapsed();
	
	// Start spawning Bots
	void StartWave();

	// Stop Spawning Bots
	void EndWave();

	// Set timer for next startwave
	void PrepareForNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);

	void RestartDeadPlayers();
};
