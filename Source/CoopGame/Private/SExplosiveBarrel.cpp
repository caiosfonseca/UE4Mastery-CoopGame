// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"
#include "Components/SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/StaticMeshComponent.h"
#include "CoopGame/CoopGame.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"

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

	SetReplicates(true);
	SetReplicateMovement(true);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealtComp, float Health, float HealthDelta,
	const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if(Health <= 0.f && !bExploded)
	{
		bExploded = true;
		Explode(InstigatedBy);
		OnRep_Exploded();
	}
}

void ASExplosiveBarrel::Explode(AController* InstigatedBy)
{
	MeshComp->AddImpulse(GetActorUpVector() * LaunchForce, NAME_None, true);

	RadialForceComp->FireImpulse();

	TArray<AActor*> IgnorableActors {this};
	
	UGameplayStatics::ApplyRadialDamage(GetWorld(), Damage, GetActorLocation(), ExplosionRadius, ExplosionDamageType, IgnorableActors, this, InstigatedBy );
}

void ASExplosiveBarrel::OnRep_Exploded()
{
	if(IsValid(ExplosionEffect))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}

	if(IsValid(ExplodedMaterial))
	{
		MeshComp->SetMaterial(0, ExplodedMaterial);
	}
}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
	DOREPLIFETIME(ASExplosiveBarrel, ExplosionEffect);
	DOREPLIFETIME(ASExplosiveBarrel, ExplodedMaterial);
}

