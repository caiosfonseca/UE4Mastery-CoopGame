// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
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
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			//Blocking Hit, process damage
			AActor* HitActor = Hit.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType );

			if(IsValid(ImpactEffect))
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());				
			}

			TracerEndPoint = Hit.ImpactPoint;
		}

		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.f, 0, 1.f);

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
		
	}
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

