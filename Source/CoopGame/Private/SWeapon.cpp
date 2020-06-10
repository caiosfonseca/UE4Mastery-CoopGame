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
#include "Net/UnrealNetwork.h"


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

	BulletSpread = 1.25f;

	Recoil = 0.5f;

	bIsReloading = false;
	bShouldUseAmmo = true;

	SetReplicates(true);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}

void ASWeapon::Fire()
{
	if(GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}
	
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

		if(bShouldUseAmmo)
		{
			CurrentAmmo -= 1;
		}
		
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
		
		FVector TraceEnd = EyeLocation + ShotDirection * 10000;
		

		// Particle Target parameter
		FVector TracerEndPoint = TraceEnd; 

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.bReturnPhysicalMaterial = true;
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		
		EPhysicalSurface PhysSurface = SurfaceType_Default;
		
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			//Blocking Hit, process damage
			AActor* HitActor = Hit.GetActor();

			PhysSurface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = BaseDamage;
			if(PhysSurface == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.f;
			}
			
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType );

			PlayImpactEfects(PhysSurface, Hit.ImpactPoint);
			
			TracerEndPoint = Hit.ImpactPoint;
		}

		if(DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.f, 0, 1.f);
		}

		if(GetLocalRole() == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = PhysSurface;
		}
		PlayFireEffects(TracerEndPoint);
		
		LastTimeFired = GetWorld()->TimeSeconds;

		if(IsValid(MyOwner))
		{
			APlayerController* PC = Cast<APlayerController>(MyOwner->GetInstigatorController());
			if(IsValid(PC))
			{
				PC->AddPitchInput(-Recoil);
			}
			
		}
		
	}
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
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
		GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &ASWeapon::OnReloaded, ReloadTime, false, ReloadTime);
	}
	
}

void ASWeapon::OnReloaded()
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

void ASWeapon::PlayImpactEfects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{		
	UParticleSystem* SelectedEffect = nullptr;
	switch(SurfaceType)
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
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());				
	}
}

void ASWeapon::OnRep_HitScanTrace()
{
	// Play Cosmetic effects
	PlayFireEffects(HitScanTrace.TraceTo);

	PlayImpactEfects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::ChangeShouldUseAmmo(bool bNewValue)
{
	bShouldUseAmmo = bNewValue;
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}
