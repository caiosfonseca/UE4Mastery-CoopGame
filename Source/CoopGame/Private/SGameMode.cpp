// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameMode.h"
#include "SCharacter.h"
#include "SGameState.h"
#include "TimerManager.h"
#include "Components/SHealthComponent.h"
#include "GameFramework/PlayerController.h"

ASGameMode::ASGameMode()
{
    DefaultPawnClass = ASCharacter::StaticClass();
    GameStateClass = ASGameState::StaticClass();
    
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

    CheckAnyPlayerAlive();
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

void ASGameMode::CheckAnyPlayerAlive()
{
    for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
    {
        APlayerController* PC = It->Get();
        if(IsValid(PC) && PC->GetPawn())
        {
            APawn* MyPawn = PC->GetPawn();
            USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));
            if(ensure(IsValid(HealthComp)) && HealthComp->GetHealth() > 0.f)
            {
                // A Player is still alive
                return;
            }
        }
    }

    //No player alive
    GameOver();
}

void ASGameMode::GameOver()
{    
    EndWave();

    // TODO finish up the match, preset "game over" to players.

    UE_LOG(LogTemp, Log, TEXT("Game OVER ! Players died"));
}

void ASGameMode::SetWaveState(EWaveState NewState)
{
    ASGameState* GS = GetGameState<ASGameState>();
    if(ensure(IsValid(GS)))
    {
        GS->WaveState = NewState;
    }
}
