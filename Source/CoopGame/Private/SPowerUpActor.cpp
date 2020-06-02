// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/RotatingMovementComponent.h"

// Sets default values
ASPowerUpActor::ASPowerUpActor()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetCanEverAffectNavigation(false);

	LightComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("LightComp"));
	LightComp->SetupAttachment(RootComponent);
	LightComp->SetCastShadows(false);
	LightComp->SetAttenuationRadius(200.f);

	RotatingMovementComp = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovementComp"));
	
	PowerupInterval = 0.f;
	TotalNrOfTicks = 0;
	TicksProcessed = 0;

	SetReplicates(true);
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
