// Copyright 2022 Andrey Bicalho.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "PhysCustomMovement.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPhysCustomMovementDelegate);

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
USTRUCT()
struct GASSHOOTER_API FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()
		
	/** Name of the custom movement. */
	UPROPERTY()
	FName MovementName;

	// Selected custom movement mode flag: this will be set by the character movement component and must match the selected compressed flag
	uint8 CustomModeFlag;

	/** Time elapsed so far for this movement */
	UPROPERTY()
	float CurrentTime = 0.f;

	/** Whether or not this movement is running */
	UPROPERTY()
	bool bIsActive = false;

	/** overrides the maximum speed for this movement mode */
	UPROPERTY()
	float MaxSpeed = 999.f;

	UPROPERTY()
	UCharacterMovementComponent* CharacterMovementComponent = nullptr;

	FPhysCustomMovementDelegate OnCustomMovementEnd;

	FPhysCustomMovement()
	{
		CharacterMovementComponent = nullptr;
		MaxSpeed = 999.f;
		CurrentTime = 0.f;
		bIsActive = false;
	}

	virtual ~FPhysCustomMovement() {}

	virtual bool BeginMovement(ACharacter* inCharacter, UCharacterMovementComponent* inCharacterMovementComponent, const uint8 inCustomModeFlag)
	{
		bool bResult = false;

		if (inCharacterMovementComponent)
		{
			CustomModeFlag = inCustomModeFlag;
			CurrentTime = 0.f;
			bIsActive = true;
			CharacterMovementComponent = inCharacterMovementComponent;
			CharacterMovementComponent->SetMovementMode(MOVE_Custom, CustomModeFlag);

			bResult = true;
		}

		return bResult;
	};

	// end current movement; this is where you have a chance to clean up
	virtual void EndMovement()
	{
		bIsActive = false;
		CharacterMovementComponent = nullptr;

		OnCustomMovementEnd.Broadcast();
	};

	// checks if movement can be done
	virtual bool CanDoMovement(const float deltaTime) const { return true; };

	// main update function, this is where you override to code your custom movement
	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) 
	{
		// boilerplate code that you may want to follow
		CurrentTime += deltaTime;

		// Do your cool movement...
	};

	// overrides the maximum speed for this movement mode
	virtual float GetMaxSpeed() const
	{
		return MaxSpeed;
	};

	// return the custom movement mode flag that was reserved for this custom movement
	virtual uint8 GetCustomModeFlag() const { return CustomModeFlag; };

	/** @return the CurrentTime - amount of time elapsed so far for this movement */
	float GetTime() const { return CurrentTime; };

	/** True when this movement is running. */
	bool IsActive() const { return bIsActive; };
};


USTRUCT()
struct GASSHOOTER_API FPhysCustomMovement_Jump : public FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector LaunchVelocity = FVector::ZeroVector;

	UPROPERTY()
	bool bXYOverride = false;

	UPROPERTY()
	bool bZOverride = false;

	FPhysCustomMovement_Jump()
	{
		LaunchVelocity = FVector::ZeroVector;
		bXYOverride = false;
		bZOverride = false;
	};

	virtual ~FPhysCustomMovement_Jump() {}

	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override
	{
		CurrentTime += deltaTime;

		outVelocity = oldVelocity;

		if (!bXYOverride)
		{
			outVelocity.X += LaunchVelocity.X;
			outVelocity.Y += LaunchVelocity.Y;
		}
		else
		{
			outVelocity.X = LaunchVelocity.X;
			outVelocity.Y = LaunchVelocity.Y;
		}

		if (!bZOverride)
		{
			outVelocity.Z += LaunchVelocity.Z;
		}
		else
		{
			outVelocity.Z = LaunchVelocity.Z;
		}

		outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());

		if (CharacterMovementComponent)
		{
			CharacterMovementComponent->SetMovementMode(MOVE_Falling);
		}
	};

	virtual void EndMovement() override 
	{
		LaunchVelocity = FVector::ZeroVector;

		FPhysCustomMovement::EndMovement();
	}
};
