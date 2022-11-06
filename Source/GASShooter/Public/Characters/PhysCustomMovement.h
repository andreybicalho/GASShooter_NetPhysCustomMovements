// Copyright 2022 Andrey Bicalho.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "PhysCustomMovement.generated.h"

/**
*	Generalized Physics Custom Movement to a CharacterMovementComponent.
*
*	This movement is based on the FSavedMove_Character with the compressed flags
*	and it's meant to be executed by ROLE_Authority or ROLE_AutonomousProxy only.
*
*
*	For most games you will need to 'subclass' and heavily modify this struct.
*	It is just a convenient way to inject any physics movement that aims to update Velocity on the
*	CharacterMovementComponent allowing you to use one single custom flag from the CompressedFlags (FLAG_Custom_0, FLAG_Custom_1, etc...)
*	with a lot of different movements.
*
*/
USTRUCT(BlueprintType)
struct GASSHOOTER_API FPhysCustomMovement
{
	GENERATED_BODY()

	/** Time elapsed so far for this movement */
	UPROPERTY()
	float CurrentTime = 0.f;

	/** Whether or not this movement is running. */
	UPROPERTY()
	bool bIsActive = false;

	uint8 CustomModeFlag;

	UPROPERTY()
	UCharacterMovementComponent* CharacterMovementComponent = nullptr;

	UPROPERTY()
	ACharacter* Character = nullptr;

	FPhysCustomMovement()
	{
		CharacterMovementComponent = nullptr;
		Character = nullptr;
		bIsActive = false;
		CurrentTime = 0.f;
	}

	FPhysCustomMovement(
		UCharacterMovementComponent* inCharacterMovementComponent,
		ACharacter* inCharacter
	)
		: CharacterMovementComponent(inCharacterMovementComponent)
		, Character(inCharacter)
	{
		bIsActive = false;
		CurrentTime = 0.f;
	}

	virtual bool BeginPhysCustomMovement(ACharacter* inCharacter, UCharacterMovementComponent* inCharacterMovementComponent, const uint8 inCustomModeFlag)
	{
		bool bResult = false;

		if (inCharacterMovementComponent)
		{
			CustomModeFlag = inCustomModeFlag;
			CurrentTime = 0.f;
			Character = inCharacter;
			bIsActive = true;
			CharacterMovementComponent = inCharacterMovementComponent;
			CharacterMovementComponent->SetMovementMode(MOVE_Custom, CustomModeFlag);

			bResult = true;
		}

		return bResult;
	};

	// end current movement; this is where you have a chance to clean up
	virtual void EndPhysCustomMovement(const EMovementMode nextMovementMode)
	{
		if (CharacterMovementComponent)
		{
			// TODO: add support to end using another custom mode
			CharacterMovementComponent->SetMovementMode(nextMovementMode);
		}

		bIsActive = false;
	};

	// checks if movement can be done
	virtual bool CanDoPhysCustomMovement(const float deltaTime) const { return true; };

	// main update function, this is where you override to code your custom movement
	virtual void UpdatePhysCustomMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) 
	{
		// boilerplate code that you may want to follow
		CurrentTime += deltaTime;

		if (!CanDoPhysCustomMovement(deltaTime))
		{
			EndPhysCustomMovement(MOVE_Falling);
		}
	};

	// return the custom movement mode flag that was reserved for this custom movement
	virtual uint8 GetCustomModeFlag() const { return CustomModeFlag; };

	/** @return the CurrentTime - amount of time elapsed so far for this movement */
	float GetTime() const { return CurrentTime; };

	/** True when this movement is running. */
	bool IsActive() const { return bIsActive; };
};


USTRUCT(BlueprintType)
struct GASSHOOTER_API FPhysCustomMovement_Jump : public FPhysCustomMovement
{
	GENERATED_BODY()

	UPROPERTY()
	FVector LaunchVelocity = FVector::ZeroVector;

	UPROPERTY()
	bool bXYOverride = false;

	UPROPERTY()
	bool bZOverride = false;

	virtual void SetLaunchVelocity(const FVector inLaunchVelocity, const bool bInXYOverride, const bool bInZOverride)
	{
		LaunchVelocity = inLaunchVelocity;
		bXYOverride = bInXYOverride;
		bZOverride = bInZOverride;
	};

	virtual void UpdatePhysCustomMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override
	{
		CurrentTime += deltaTime;

		outVelocity = oldVelocity;

		if (!bXYOverride)
		{
			outVelocity.X += LaunchVelocity.X;
			outVelocity.Y += LaunchVelocity.Y;
		}
		if (!bZOverride)
		{
			outVelocity.Z += LaunchVelocity.Z;
		}

		EndPhysCustomMovement(MOVE_Falling);
	};

	virtual void EndPhysCustomMovement(const EMovementMode nextMovementMode) override 
	{
		LaunchVelocity = FVector::ZeroVector;

		FPhysCustomMovement::EndPhysCustomMovement(nextMovementMode);
	}
};
