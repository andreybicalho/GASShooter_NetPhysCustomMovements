// Copyright 2022 Andrey Bicalho.

#include "Movements/PhysCustomMovement.h"
#include "GameFramework/CharacterMovementComponent.h"

FPhysCustomMovement::FPhysCustomMovement()
{
	CharacterMovementComponent = nullptr;
	MaxSpeed = 999.f;
	CurrentTime = 0.f;
	bIsActive = false;
	MovementName = NAME_None;
	FallbackMovementMode = EMovementMode::MOVE_Falling;
}

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
