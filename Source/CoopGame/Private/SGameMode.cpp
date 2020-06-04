// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameMode.h"
#include "SCharacter.h"
#include "TimerManager.h"
#include "Components/SHealthComponent.h"

ASGameMode::ASGameMode()
{
    DefaultPawnClass = ASCharacter::StaticClass();
    WaveCount = 0;
    TimeBetweenWaves = 2.f;

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.f;
}



void ASGameMode::StartPlay()
{
    Super::StartPlay();
    PrepareForNextWave();
}

void ASGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    CheckWaveState();
}

void ASGameMode::SpawnBotTimerElapsed()
{
    SpawnNewBot();

    NrOfBotsToSpawn--;
    
    if(NrOfBotsToSpawn <= 0)
    {
        EndWave();
    }
}

void ASGameMode::StartWave()
{
    WaveCount++;
    NrOfBotsToSpawn = 2 * WaveCount;
    GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.f, true, 0.f);
}

void ASGameMode::EndWave()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
}

void ASGameMode::PrepareForNextWave()
{
    GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);
}

void ASGameMode::CheckWaveState()
{
    bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);
    
    if(NrOfBotsToSpawn > 0 || bIsPreparingForWave)
    {
        return;
    }
    
    bool bIsAnyBotAlive = false;
    
    for(FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
    {
        APawn* TestPawn = It->Get();
        if(!IsValid(TestPawn) || TestPawn->IsPlayerControlled())
        {
            continue;
        }

        USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
        if(IsValid(HealthComp) && HealthComp->GetHealth() >= 0.0f)
        {
            bIsAnyBotAlive = true;
            break;
        }
    }

    if(!bIsAnyBotAlive)
    {
        PrepareForNextWave();
    }
}
