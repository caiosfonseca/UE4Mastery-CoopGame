// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShake.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame/CoopGame.h"


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
    TEXT("COOP.DebugWeapons"),
    DebugWeaponDrawing,
    TEXT("Draw Debug lines for Weapons"),
    ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
}

void ASWeapon::Fire()
{
	//Trace the world, from pawn eyes to crosshair location
	AActor* MyOwner = GetOwner();
	
	if(IsValid(MyOwner))
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();
		
		FVector TraceEnd = EyeLocation + ShotDirection * 10000;

		// Partile Target parameter
		FVector TracerEndPoint = TraceEnd; 

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.bReturnPhysicalMaterial = true;
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			//Blocking Hit, process damage
			AActor* HitActor = Hit.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType );


			EPhysicalSurface PhysSurface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			UParticleSystem* SelectedEffect = nullptr;
			switch(PhysSurface)
			{
			case SURFACE_FLESHDEFAULT:
			case SURFACE_FLESHVULNERABLE:
				SelectedEffect = FleshImpactEffect;
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}

			if(IsValid(DefaultImpactEffect))
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());				
			}

			TracerEndPoint = Hit.ImpactPoint;
		}

		if(DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.f, 0, 1.f);
		}

		PlayFireEffects(TracerEndPoint);
		
		
	}
}

void ASWeapon::PlayFireEffects(FVector TracerEndPoint)
{
	if(IsValid(MuzzleEffect))
{
	UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
}

	if( IsValid(TracerEffect) )
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp =  UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if(IsValid(TracerComp))
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if(IsValid(MyOwner))
	{
		APlayerController* PC = Cast<APlayerController>( MyOwner->GetController());
		if(IsValid(PC))
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

