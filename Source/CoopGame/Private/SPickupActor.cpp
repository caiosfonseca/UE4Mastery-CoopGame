// Fill out your copyright notice in the Description page of Project Settings.


#include "SPickupActor.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "SPowerUpActor.h"
#include "TimerManager.h"

// Sets default values
ASPickupActor::ASPickupActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.f);
	RootComponent = SphereComp;
	
	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetupAttachment(RootComponent);
	DecalComp->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	DecalComp->DecalSize = FVector(64.f, 75.f, 75.f);

	ZOffset = 20.f;

	SetReplicates(true);
}

void ASPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if(GetLocalRole() == ROLE_Authority)
	{
		if(IsValid(PowerUpInstance) && IsValid(OtherActor->GetNetOwningPlayer()))
		{
			PowerUpInstance->ActivatePowerup(OtherActor);
			PowerUpInstance = nullptr;

			// Set timer to respawn
			GetWorldTimerManager().SetTimer(TimerHandle_RespawnCooldownTimer, this, &ASPickupActor::Respawn,
                CooldownDuration);
		}
	}
}

// Called when the game starts or when spawned
void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();
	
	if(GetLocalRole() == ROLE_Authority)
	{
		Respawn();
	}
}

void ASPickupActor::Respawn()
{
	if(!IsValid(PowerUpClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerUpClass is nullptr in %s. Please update your blueprint"), *GetName());
		return;
	}
	FActorSpawnParameters Spawnparams;
	Spawnparams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FTransform NewTransform = GetTransform();
	NewTransform.SetLocation(NewTransform.GetLocation() + FVector(0.f, 0.f, ZOffset));
	
	PowerUpInstance = GetWorld()->SpawnActor<ASPowerUpActor>(PowerUpClass, NewTransform, Spawnparams);
}
