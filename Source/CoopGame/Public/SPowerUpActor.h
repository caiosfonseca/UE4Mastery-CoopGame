// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerUpActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class URotatingMovementComponent;


UCLASS()
class COOPGAME_API ASPowerUpActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPowerUpActor();

	UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
	void OnActivated(AActor* ActivateFor);

	UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
	void OnExpired();

	UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
	void OnPowerUpTicked();

	void ActivatePowerup(AActor* ActivateFor);

protected:

	/* Time between the power up ticks */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerupInterval;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* LightComp;
    
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    URotatingMovementComponent* RotatingMovementComp;

	/* Total times we apply the power up effect */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 TotalNrOfTicks;

	int32 TicksProcessed;

	FTimerHandle TimerHandle_PowerUpTick;

	UFUNCTION()
	void OnTickPowerup();

	// Keeps state of the powerup
	UPROPERTY(ReplicatedUsing=OnRep_PowerupActive)
	bool bIsPowerupActive;

	UFUNCTION()
	void OnRep_PowerupActive();

	UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
	void OnPowerupStateChanged(bool bNewIsActive);

};
