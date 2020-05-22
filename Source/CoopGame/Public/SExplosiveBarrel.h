// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class USHealthComponent;
class UParticleSystem;
class UStaticMeshComponent;
class UMaterialInterface;
class URadialForceComponent;

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	URadialForceComponent* RadialForceComp;

	UFUNCTION()
    void OnHealthChanged(USHealthComponent* OwningHealtComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	void Explode(AController* InstigatedBy);

	UPROPERTY(BlueprintReadOnly, Category = "Explosion")
    bool bExploded;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
    UMaterialInterface* DefaultMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
    UMaterialInterface* ExplodedMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float LaunchForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
    float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
    float ExplosionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	TSubclassOf<UDamageType> ExplosionDamageType;
};
