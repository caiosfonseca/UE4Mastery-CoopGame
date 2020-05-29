// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

class USphereComponent;
class UDecalComponent;
class ASPowerUpActor;

UCLASS()
class COOPGAME_API ASPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPickupActor();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "PickupActor")
	TSubclassOf<ASPowerUpActor> PowerUpClass;

	UFUNCTION()
	void Respawn();

	ASPowerUpActor* PowerUpInstance;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "PickupActor")
	float CooldownDuration;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "PickupActor")
	float ZOffset;
	
	FTimerHandle TimerHandle_RespawnCooldownTimer;
	

};
