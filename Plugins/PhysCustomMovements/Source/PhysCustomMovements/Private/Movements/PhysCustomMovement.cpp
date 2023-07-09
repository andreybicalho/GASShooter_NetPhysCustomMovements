// Copyright 2022 Andrey Bicalho.

#include "Movements/PhysCustomMovement.h"
#include "GameFramework/CharacterMovementComponent.h"

bool FPhysCustomMovement::BeginMovement(ACharacter* inCharacter, UCharacterMovementComponent* inCharacterMovementComponent, const uint8 inCustomModeFlag)
{
	if (inCharacterMovementComponent)
	{
		CharacterMovementComponent = inCharacterMovementComponent;
		CustomModeFlag = inCustomModeFlag;
		CurrentTime = 0.f;
		bIsActive = true;
		CharacterMovementComponent->MaxCustomMovementSpeed = MaxSpeed;
		CharacterMovementComponent->SetMovementMode(MOVE_Custom, CustomModeFlag);

		return true;
	}

	return false;
}

void FPhysCustomMovement::EndMovement()
{
	bIsActive = false;
	CharacterMovementComponent = nullptr;

	OnCustomMovementEnd.Broadcast();
}

UScriptStruct* FPhysCustomMovement::GetTypeStruct() const
{
	return FPhysCustomMovement::StaticStruct();
}

bool FPhysCustomMovement::SkipThisUpdate(const float deltaTime)
{
	if (bSkipMovementUpdates)
	{
		UpdateSkipCount++;
		TimeSkippingMovement += deltaTime;

		if (UpdateSkipCount > MaxNumUpdateSkips) // TODO: find a better way of setting this threshold, maybe based on the time we get updates from the server
		{
			ResetUpdateSkipping();
		}
	}

	return bSkipMovementUpdates;
}

void FPhysCustomMovement::HoldMovementUpdates()
{
	bSkipMovementUpdates = true;
}

void FPhysCustomMovement::ResetUpdateSkipping()
{
	TimeSkippingMovement = 0.f;
	UpdateSkipCount = 0;
	bSkipMovementUpdates = false;
}
