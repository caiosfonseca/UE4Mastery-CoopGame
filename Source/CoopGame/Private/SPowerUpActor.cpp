// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Net/UnrealNetwork.h"

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
	bIsPowerupActive = false;

	SetReplicates(true);
}

void ASPowerUpActor::ActivatePowerup(AActor* ActivateFor)
{
	OnActivated(ActivateFor);

	bIsPowerupActive = true;
	OnRep_PowerupActive();
	
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

		bIsPowerupActive = false;
		OnRep_PowerupActive();
		
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerUpTick);
	}
}

void ASPowerUpActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void ASPowerUpActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerUpActor, bIsPowerupActive);
}
