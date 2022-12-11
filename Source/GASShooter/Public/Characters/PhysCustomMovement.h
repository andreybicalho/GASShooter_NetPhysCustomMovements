// Copyright 2022 Andrey Bicalho.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
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

	UPROPERTY()
	UCharacterMovementComponent* CharacterMovementComponent = nullptr;

	FPhysCustomMovementDelegate OnCustomMovementEnd;

	FPhysCustomMovement()
	{
		CharacterMovementComponent = nullptr;
		MaxSpeed = 999.f;
		CurrentTime = 0.f;
		bIsActive = false;
		MovementName = NAME_None;
	}

	virtual ~FPhysCustomMovement() {}

	virtual bool BeginMovement(ACharacter* inCharacter, UCharacterMovementComponent* inCharacterMovementComponent, const uint8 inCustomModeFlag)
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
		
		// ... and don't forget to clamp to the maximum speed for the movement
		outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());
	};

	// overrides the maximum speed for this movement mode
	virtual float GetMaxSpeed() const
	{
		return MaxSpeed;
	};

	// return the custom movement mode flag that was reserved for this custom movement
	uint8 GetCustomModeFlag() const { return CustomModeFlag; };

	/** @return the CurrentTime - amount of time elapsed so far for this movement */
	float GetTime() const { return CurrentTime; };

	/** True when this movement is running. */
	bool IsActive() const { return bIsActive; };
};


/**
* An example of a simple jump (Launch).
* 
*/
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
		SCOPED_NAMED_EVENT(FPhysCustomMovement_UpdateMovement_Jump, FColor::Yellow);

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

/**
* An example of a simple jet pack.
*
*/
USTRUCT()
struct GASSHOOTER_API FPhysCustomMovement_JetPack : public FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector BaseAcceleration;

	FPhysCustomMovement_JetPack()
	{
	};

	virtual ~FPhysCustomMovement_JetPack() {}

	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override
	{
		SCOPED_NAMED_EVENT(FPhysCustomMovement_UpdateMovement_JetPack, FColor::Yellow);

		CurrentTime += deltaTime;

		if (const ACharacter* character = CharacterMovementComponent->GetCharacterOwner())
		{
			const FVector velocity = (character->GetActorForwardVector() + FVector::UpVector) * BaseAcceleration;

			const FVector acceleration = velocity * deltaTime;
			outVelocity += acceleration;
			outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed()); // prevents to go further than the max speed for the mode
		}
	};

	virtual void EndMovement() override
	{
		FPhysCustomMovement::EndMovement();
	}
};

/**
* An example of a sprint.
*
*/
USTRUCT()
struct GASSHOOTER_API FPhysCustomMovement_Sprint : public FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

	FPhysCustomMovement_Sprint()
	{
	};

	virtual ~FPhysCustomMovement_Sprint() {}

	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override
	{
		SCOPED_NAMED_EVENT(FPhysCustomMovement_UpdateMovement_Sprint, FColor::Yellow);

		CurrentTime += deltaTime;

		if (CharacterMovementComponent)
		{
			outVelocity = CharacterMovementComponent->GetCurrentAcceleration() * GetMaxSpeed();
			outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());
		}
	};
};

/**
* An example of follow the points.
*
*/
USTRUCT()
struct GASSHOOTER_API FPhysCustomMovement_FollowPoints : public FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

	TArray<FVector> PathPoints = {};

	float ConsumePointDistanceThreshold = 50.f;

	FPhysCustomMovement_FollowPoints()
	{
	};

	virtual ~FPhysCustomMovement_FollowPoints() {}

	int32 FindNearestPoint(const FVector& location, FVector& outLocation) const
	{
		float distance = FLT_MAX;
		int32 index = INDEX_NONE;

		for (int32 i = 0; i < PathPoints.Num(); i++)
		{
			const float currentDist = FVector::Distance(location, PathPoints[i]);
			if (currentDist <= distance)
			{
				distance = currentDist;
				outLocation = PathPoints[i];
				index = i;
			}
		}

		return index;
	};

	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override
	{
		SCOPED_NAMED_EVENT(FPhysCustomMovement_UpdateMovement_FollowPoints, FColor::Yellow);

		CurrentTime += deltaTime;

		if (CharacterMovementComponent)
		{
			const FVector currentLocation = CharacterMovementComponent->GetActorFeetLocation();
			FVector nearestPoint;
			const int32 nearestPointIndex = FindNearestPoint(currentLocation, nearestPoint);

			if (!PathPoints.IsEmpty() && nearestPointIndex != INDEX_NONE)
			{
				const FVector direction = (nearestPoint - currentLocation).GetSafeNormal();

				outVelocity = direction * GetMaxSpeed();

				outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());

				const float distance = FVector::Distance(currentLocation, nearestPoint);
				if (distance <= ConsumePointDistanceThreshold)
				{
					PathPoints.RemoveAt(nearestPointIndex);
				}
			}
			else
			{
				CharacterMovementComponent->SetMovementMode(MOVE_Falling);
			}
		}
	};
};
