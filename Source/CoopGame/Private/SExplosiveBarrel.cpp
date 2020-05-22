// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"
#include "Components/SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/StaticMeshComponent.h"
#include "CoopGame/CoopGame.h"
#include "PhysicsEngine/RadialForceComponent.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	if(IsValid(DefaultMaterial))
	{
		MeshComp->SetMaterial(0, DefaultMaterial);
	}
	
	ExplosionRadius = 250.f;
	LaunchForce = 1000.f;
	Damage = 40.f;
	
	
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(RootComponent);
	RadialForceComp->bIgnoreOwningActor = true;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->SetAutoActivate(false);
	RadialForceComp->Radius = ExplosionRadius;
	RadialForceComp->ImpulseStrength = LaunchForce;
	RadialForceComp->AddCollisionChannelToAffect(ECollisionChannel::ECC_PhysicsBody);

	bExploded = false;
}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealtComp, float Health, float HealthDelta,
	const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if(Health <= 0.f && !bExploded)
	{
		bExploded = true;
		Explode(InstigatedBy);
	}
}

void ASExplosiveBarrel::Explode(AController* InstigatedBy)
{
	if(IsValid(ExplosionEffect))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}

	if(IsValid(ExplodedMaterial))
	{
		MeshComp->SetMaterial(0, ExplodedMaterial);
	}

	
	MeshComp->AddImpulse(GetActorUpVector() * LaunchForce, NAME_None, true);

	RadialForceComp->FireImpulse();

	TArray<AActor*> IgnorableActors {this};
	
	UGameplayStatics::ApplyRadialDamage(GetWorld(), Damage, GetActorLocation(), ExplosionRadius, ExplosionDamageType, IgnorableActors, this, InstigatedBy );
}

