// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UCameraShake;

USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;
	
	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Fire();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Weapon")
    virtual void ServerFire();

	FTimerHandle TimerHandle_TimeBetweenShots;
	FTimerHandle TimerHandle_Reload;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Reload();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void OnReload();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    int32 CurrentAmmo;

protected:

	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	void PlayFireEffects(FVector TracerEndPoint);

	void PlayImpactEfects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float BaseDamage;

	float LastTimeFired;

	/* RPM - Bullets per minute fired */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	float TimeBetweenShots;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    int32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ReloadTime;

	bool bIsReloading;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float PitchSpreadAngle;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float YawSpreadAngle;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float Recoil;
};
