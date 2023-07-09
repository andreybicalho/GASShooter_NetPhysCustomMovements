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

public:

	TWeakObjectPtr<UCharacterMovementComponent> CharacterMovementComponent;

	/** Name of the custom movement. */
	UPROPERTY()
	FName MovementName;

	/** Time elapsed so far for this movement */
	UPROPERTY()
	float CurrentTime;

	/** Whether or not this movement is running */
	UPROPERTY()
	bool bIsActive;

	/** overrides the maximum speed for this movement mode */
	UPROPERTY()
	float MaxSpeed;

	/** overrides the maximum acceleration for this movement mode */
	UPROPERTY()
	float MaxAcceleration;

	/** overrides the maximum braking deceleration for this movement mode */
	UPROPERTY()
	float MaxBrakingDeceleration;

	EMovementMode FallbackMovementMode = EMovementMode::MOVE_Falling;

	FPhysCustomMovementDelegate OnCustomMovementEnd;

	int32 UpdateSkipCount;

	int32 MaxNumUpdateSkips;

	bool bSkipMovementUpdates;

	float TimeSkippingMovement;

	float LocationErrorToleranceThreshold;

private:

	// Selected custom movement mode flag: this will be set by the character movement component and must match the selected compressed flag
	uint8 CustomModeFlag;

public:

	FPhysCustomMovement()
		: CharacterMovementComponent(nullptr)
		, MovementName(NAME_None)
		, CurrentTime(0.f)
		, bIsActive(false)
		, MaxSpeed(999.f)
		, MaxAcceleration(999.f)
		, MaxBrakingDeceleration(0.f)
		, FallbackMovementMode(EMovementMode::MOVE_Falling)
		, UpdateSkipCount(0)
		, MaxNumUpdateSkips(16)
		, bSkipMovementUpdates(false)
		, TimeSkippingMovement(0.f)
		, LocationErrorToleranceThreshold(20.f)
	{}

	virtual ~FPhysCustomMovement() {}

	virtual bool BeginMovement(ACharacter* inCharacter, UCharacterMovementComponent* inCharacterMovementComponent, const uint8 inCustomModeFlag);

	// end current movement; this is where you have a chance to clean up
	virtual void EndMovement();

	// checks if movement can be done; movement mode will change whenever this fails.
	virtual bool CanDoMovement(const float deltaTime) const { return true; }

	// main update function, this is where you override to code your custom movement
	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity)
	{
		// boilerplate code that you may want to follow
		CurrentTime += deltaTime;

		// Do your cool movement here...

		// ... and don't forget to clamp to the maximum speed
		outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());
	}

	// overrides the maximum speed for this movement mode
	virtual float GetMaxSpeed() const { return MaxSpeed; }

	// overrides the maximum acceleration for this movement mode
	virtual float GetMaxAcceleration() const { return MaxAcceleration; }

	// overrides the maximum braking deceleration for this movement mode
	virtual float GetMaxBrakingDeceleration() const { return MaxBrakingDeceleration; }

	// return the custom movement mode flag that was reserved for this custom movement
	uint8 GetCustomModeFlag() const { return CustomModeFlag; }

	/** @return the CurrentTime - amount of time elapsed so far for this movement */
	float GetTime() const { return CurrentTime; }

	/** True when this movement is running. */
	bool IsActive() const { return bIsActive; }

	virtual UScriptStruct* GetTypeStruct() const;

	bool SkipThisUpdate(const float deltaTime);

	void HoldMovementUpdates();

	void ResetUpdateSkipping();
};
