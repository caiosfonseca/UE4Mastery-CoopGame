// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SHealthComponent.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	DefaultHealth = 100;
	// ...
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* MyOwner = GetOwner();
	if(IsValid(MyOwner))
	{
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
	}

	Health = DefaultHealth;
	
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	if(Damage <= 0.f)
	{
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.f, DefaultHealth);

	UE_LOG(LogTemp, Warning, TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health));
}

