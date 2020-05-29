// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor.h"

// Sets default values
ASPowerUpActor::ASPowerUpActor()
{
	PowerupInterval = 0.f;
	TotalNrOfTicks = 0;
	TicksProcessed = 0;
}

void ASPowerUpActor::ActivatePowerup()
{
	OnActivated();
	
	if(PowerupInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerUpTick, this, &ASPowerUpActor::OnTickPowerup,
			PowerupInterval, true);
	}
	else
	{
		OnTickPowerup();
	}
}

// Called when the game starts or when spawned
void ASPowerUpActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASPowerUpActor::OnTickPowerup()
{
	TicksProcessed++;

	OnPowerUpTicked();

	if(TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerUpTick);
	}
}
