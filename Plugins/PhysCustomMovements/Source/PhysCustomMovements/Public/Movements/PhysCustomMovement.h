// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PhysCustomMovement.generated.h"

class UCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPhysCustomMovementDelegate);

template<typename T>
struct FPhysPredictedProperty
{
	FName Name;
	T Value;

	bool operator ==(const FPhysPredictedProperty& other) const
	{
		return Name == other.Name;
	}

	friend FArchive& operator<<(FArchive& archive, FPhysPredictedProperty& physPredictedProperty)
	{
		archive << physPredictedProperty.Name;
		archive << physPredictedProperty.Value;
		return archive;
	}
};

/**
*	Generalized Physics Custom Movement to a CharacterMovementComponent.
*
*	This movement is based on the FSavedMove_Character and compressed flags approach
*   in which the movement code runs in a deterministic way for both ROLE_Authority and ROLE_AutonomousProxy.
*   If you have any data that carries on information about the movement and can't be predicted, then make sure you
*   replicate them in the FSavedMove_Character and apply them accordingly so you minimize server corrections. 
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
	FName MovementName;

	/** Time elapsed so far for this movement */
	float CurrentTime;

	/** Whether or not this movement is running */
	bool bIsActive;

	/** overrides the maximum speed for this movement mode */
	float MaxSpeed;

	/** overrides the maximum acceleration for this movement mode */
	float MaxAcceleration;

	/** overrides the maximum braking deceleration for this movement mode */
	float MaxBrakingDeceleration;

	FVector CurrentVelocity;

	EMovementMode FallbackMovementMode = EMovementMode::MOVE_Falling;

	FPhysCustomMovementDelegate OnCustomMovementEnd;

	int32 UpdateSkipCount;

	int32 MaxNumUpdateSkips;

	bool bSkipMovementUpdates;

	float TimeSkippingMovement;

	float LocationErrorToleranceThreshold;

	// predicted properties (they carry on information from the movement, client can predict them and then send them to server)
	TArray<FPhysPredictedProperty<uint8>> PhysBytes;
	TArray<FPhysPredictedProperty<bool>> PhysBooleans;
	TArray<FPhysPredictedProperty<int32>> PhysIntegers;
	TArray<FPhysPredictedProperty<float>> PhysFloats;
	TArray<FPhysPredictedProperty<double>> PhysDoubles;
	TArray<FPhysPredictedProperty<FVector>> PhysVectors;
	TArray<FPhysPredictedProperty<FVector2D>> PhysVectors2D;
	TArray<FPhysPredictedProperty<FVector4>> PhysVectors4;
	TArray<FPhysPredictedProperty<FRotator>> PhysRotators;
	TArray<FPhysPredictedProperty<FQuat>> PhysQuats;
	TArray<FPhysPredictedProperty<FGameplayTag>> PhysGameplayTags;

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
		, CurrentVelocity(FVector::ZeroVector)
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

		CurrentVelocity = outVelocity;
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

	/** @return newly allocated copy of this FPhysCustomMovement. Must be overridden by child classes. */
	virtual FPhysCustomMovement* Clone() const;

	bool SkipThisUpdate(const float deltaTime);

	void HoldMovementUpdates();

	void ResetUpdateSkipping();

	// Bind Predicted Properties
	void BindByteProperty(const FName name);
	void BindBoolProperty(const FName name);
	void BindIntegerProperty(const FName name);
	void BindFloatProperty(const FName name);
	void BindDoubleProperty(const FName name);
	void BindVectorProperty(const FName name);
	void BindVector2DProperty(const FName name);
	void BindVector4Property(const FName name);
	void BindRotatorProperty(const FName name);
	void BindQuatProperty(const FName name);
	void BindGameplayTagProperty(const FName name);

	bool HasBoundPredictedProperties() const;
	void ClearPredictedProperties();
	virtual void Clear();

	virtual void SetupBaseFromCustomMovement(const FPhysCustomMovement& physCustomMovement);
	void SetupPredictedProperties(FPhysCustomMovement& outPhysCustomMovement);
	void ReflectFromOtherPredPropsByMyKeys(const FPhysCustomMovement& otherPhysCustomMovement);
	void ReflectFromOtherPredictedProperties(const FPhysCustomMovement& otherPhysCustomMovement);

	// serialization
	virtual bool NetSerialize(FArchive& ar, UPackageMap* map, bool& bOutSuccess);
	virtual UScriptStruct* GetScriptStruct() const;

	bool operator==(const FPhysCustomMovement& other) const
	{
		return MovementName == other.MovementName;
	}

	bool operator!=(const FPhysCustomMovement& other) const
	{
		return MovementName != other.MovementName;
	}

	friend FArchive& operator<<(FArchive& ar, FPhysCustomMovement& physCustomMovement);
};

template<>
struct TStructOpsTypeTraits<FPhysCustomMovement> : public TStructOpsTypeTraitsBase2<FPhysCustomMovement>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
