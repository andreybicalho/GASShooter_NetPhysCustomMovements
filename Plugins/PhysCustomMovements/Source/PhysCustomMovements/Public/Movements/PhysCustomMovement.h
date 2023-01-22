// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "PhysCustomMovement.generated.h"

class UCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPhysCustomMovementDelegate);

/**
*	Generalized Physics Custom Movement to a CharacterMovementComponent.
*
*	This movement is based on the FSavedMove_Character and compressed flags approach
*   in which the movement code runs in a deterministic way for both ROLE_Authority and ROLE_AutonomousProxy.
*   If you have any data that carries on information about the movement and can't be predicted, then make sure you
*   replicate them in the FSavedMove_Character and apply them accordingly so you minimize server corrections. 
*   // TODO: add functionality to bind non predicted data and set them on the movement struct.
*
*
*	For most games you will need to 'subclass' and heavily modify this struct.
*	This is just a convenient way to inject any physics based movement that aims to update the Velocity attribute on the
*	CharacterMovementComponent allowing you to use one single custom flag from the CompressedFlags (FLAG_Custom_0, FLAG_Custom_1, etc...)
*	with a lot of different movements.
*
*/
USTRUCT()
struct PHYSCUSTOMMOVEMENTS_API FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

private:
	// Selected custom movement mode flag: this will be set by the character movement component and must match the selected compressed flag
	uint8 CustomModeFlag;

public:
	/** Name of the custom movement. */
	UPROPERTY()
	FName MovementName = NAME_None;

	/** Time elapsed so far for this movement */
	UPROPERTY()
	float CurrentTime = 0.f;

	/** Whether or not this movement is running */
	UPROPERTY()
	bool bIsActive = false;

	/** overrides the maximum speed for this movement mode */
	UPROPERTY()
	float MaxSpeed = 999.f;

	TWeakObjectPtr<UCharacterMovementComponent> CharacterMovementComponent = nullptr;

	FPhysCustomMovementDelegate OnCustomMovementEnd;

	EMovementMode FallbackMovementMode = EMovementMode::MOVE_Falling;

	FPhysCustomMovement();

	virtual ~FPhysCustomMovement() {}

	virtual bool BeginMovement(ACharacter* inCharacter, UCharacterMovementComponent* inCharacterMovementComponent, const uint8 inCustomModeFlag);

	// end current movement; this is where you have a chance to clean up
	virtual void EndMovement();

	// checks if movement can be done; movement mode will change whenever this fails.
	virtual bool CanDoMovement(const float deltaTime) const { return true; };

	// main update function, this is where you override to code your custom movement
	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity)
	{
		// boilerplate code that you may want to follow
		CurrentTime += deltaTime;

		// Do your cool movement here...

		// ... and don't forget to clamp to the maximum speed
		outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());
	};

	// overrides the maximum speed for this movement mode
	virtual float GetMaxSpeed() const { return MaxSpeed; };

	// return the custom movement mode flag that was reserved for this custom movement
	uint8 GetCustomModeFlag() const { return CustomModeFlag; };

	/** @return the CurrentTime - amount of time elapsed so far for this movement */
	float GetTime() const { return CurrentTime; };

	/** True when this movement is running. */
	bool IsActive() const { return bIsActive; };

	virtual UScriptStruct* GetTypeStruct() const;
};
