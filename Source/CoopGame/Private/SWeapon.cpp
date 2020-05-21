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
#include "TimerManager.h"


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

	BaseDamage = 20.f;

	RateOfFire = 600;

	ReloadTime = 2.5f;

	MaxAmmo = 30;
	CurrentAmmo = 30;

	PitchSpreadAngle = 1.25f;
	YawSpreadAngle = 1.25f;

	Recoil = 0.5f;

	bIsReloading = false;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}

void ASWeapon::Fire()
{
	//Trace the world, from pawn eyes to crosshair location
	AActor* MyOwner = GetOwner();
	
	if(IsValid(MyOwner))
	{
		if(bIsReloading)
		{
			StopFire();
			return;
		}
		
		if(!CurrentAmmo)
		{
			StopFire();
			Reload();
			return;
		}

		CurrentAmmo -= 1;
		
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		float RandomYaw = FMath::FRandRange(-YawSpreadAngle, YawSpreadAngle);
		float RandomPitch = FMath::FRandRange(-PitchSpreadAngle, PitchSpreadAngle);

		FRotator SpreadRot = FRotator(RandomPitch, RandomYaw, 0.f);
		
		FVector TraceEnd = SpreadRot.RotateVector(EyeLocation + ShotDirection * 10000);
		

		// Particle Target parameter
		FVector TracerEndPoint = TraceEnd; 

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.bReturnPhysicalMaterial = true;
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			//Blocking Hit, process damage
			AActor* HitActor = Hit.GetActor();

			EPhysicalSurface PhysSurface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = BaseDamage;
			if(PhysSurface == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.f;
			}
			
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType );
			
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
		
		LastTimeFired = GetWorld()->TimeSeconds;

		Cast<APlayerController>(MyOwner->GetInstigatorController())->AddPitchInput(-Recoil);
		
	}
}

void ASWeapon::StartFire()
{
	if(bIsReloading)
	{
		return;
	}
	
	if(CurrentAmmo)
	{
		float FirstDelay = FMath::Max(LastTimeFired + TimeBetweenShots - GetWorld()->TimeSeconds, 0.f);
	
		GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
	}
	else
	{
		Reload();
	}
	
	
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::Reload()
{
	if(CurrentAmmo != MaxAmmo && !bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reloading"));
		bIsReloading = true;
		GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &ASWeapon::OnReload, ReloadTime, false, ReloadTime);
	}
	
}

void ASWeapon::OnReload()
{
	UE_LOG(LogTemp, Warning, TEXT("Reloaded"));
	CurrentAmmo = MaxAmmo;
	bIsReloading = false;
	GetWorldTimerManager().ClearTimer(TimerHandle_Reload);
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

