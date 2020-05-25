// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameMode.h"
#include "SCharacter.h"

ASGameMode::ASGameMode()
{
    DefaultPawnClass = ASCharacter::StaticClass();
}
