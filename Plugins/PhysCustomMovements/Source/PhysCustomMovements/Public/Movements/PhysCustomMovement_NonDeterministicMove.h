// Copyright 2022 Andrey Bicalho.

#pragma once

#include "Movements/PhysCustomMovement.h"
#include "PhysCustomMovement_NonDeterministicMove.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysNonDeterministicMove, Log, All);

/**
* An example of a non deterministic movement in which we have variables that can't be predicted (TimeToWait, MovementDirectionSign and ElapsedTime). 
* During the simulation server and autonomous proxy will end up with different values and thus be out of sync. 
* To fix that we must replicate those variables from client to server and apply them correctly so they still be in sync.
* 
* Check FPMSavedMove, FPMCharacterNetworkMoveData, UPMCharacterMovementComponent and specially UPMCharacterMovementComponent::MoveAutonomous to see how it works.
* 
*/
USTRUCT()
struct PHYSCUSTOMMOVEMENTS_API FPhysCustomMovement_NonDeterministicMove : public FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() // predicted variables requires uproperty otherwise reflection system can't pick it up
	float TimeToWait;

	UPROPERTY()
	float MovementDirectionSign;

	UPROPERTY()
	float ElapsedTime;

	FPhysCustomMovementDelegate OnReachedTime;

	FPhysCustomMovement_NonDeterministicMove();

	virtual ~FPhysCustomMovement_NonDeterministicMove() { }

	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override;

	virtual UScriptStruct* GetScriptStruct() const;

	virtual FPhysCustomMovement* Clone() const;

	virtual void Clear() override;

	// TODO: if we manage to serialize it we could remove the predicted properties and just use reflection to get and set data
	/*virtual void SetupBaseFromCustomMovement(const FPhysCustomMovement& physCustomMovement) override;*/

	/*virtual bool NetSerialize(FArchive& ar, UPackageMap* map, bool& bOutSuccess) override;*/
};

//template<>
//struct TStructOpsTypeTraits<FPhysCustomMovement_NonDeterministicMove> : public TStructOpsTypeTraitsBase2<FPhysCustomMovement_NonDeterministicMove>
//{
//	enum
//	{
//		WithNetSerializer = true,
//		WithCopy = true
//	};
//};
