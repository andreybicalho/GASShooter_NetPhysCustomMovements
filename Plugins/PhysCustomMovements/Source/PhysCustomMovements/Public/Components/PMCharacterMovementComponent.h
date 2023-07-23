// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Movements/PhysCustomMovement.h"
#include "PMCharacterMovementComponent.generated.h"

#define GET_ACTOR_LOCAL_ROLE_FSTRING(actor) *UEnum::GetValueAsString(actor->GetLocalRole())

DECLARE_LOG_CATEGORY_EXTERN(LogPhysCustomMovement, Log, All);

class FPMSavedMove : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	FPMSavedMove() : Super() { }

	///@brief Resets all saved variables.
	virtual void Clear() override;

	///@brief Store input commands in the compressed flags.
	virtual uint8 GetCompressedFlags() const override;

	///@brief This is used to check whether or not two moves can be combined into one.
	///Basically you just check to make sure that the saved variables are the same.
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;

	///@brief Sets up the move before sending it to the server. 
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	///@brief Sets variables on character movement component before making a predictive correction.
	virtual void PrepMoveFor(class ACharacter* Character) override;

	// Any custom physics movement
	uint8 bSavedWantsPhysCustomMovement : 1;

	FPhysCustomMovement Saved_PhysCustomMovement;
};

class FPMNetworkPredictionData_Client : public FNetworkPredictionData_Client_Character
{
public:
	//typedef FNetworkPredictionData_Client_Character Super;
	using Super = FNetworkPredictionData_Client_Character; // I can't see the difference from the above

	FPMNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement);

	///@brief Allocates a new copy of our custom saved move
	virtual FSavedMovePtr AllocateNewMove() override;
};

//Network Move DATA
class FPMCharacterNetworkMoveData : public FCharacterNetworkMoveData
{
public:
	typedef FCharacterNetworkMoveData Super;

	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType) override;

	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType) override;

	FPhysCustomMovement MoveData_PhysCustomMovement;
};

class FPMCharacterNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
{
public:
	typedef FCharacterNetworkMoveDataContainer Super;

	FPMCharacterNetworkMoveDataContainer();

	FPMCharacterNetworkMoveData CustomDefaultMoveData[3];
};

/**
 * Provides basic functionality to apply and manage the custom movement.
 */
UCLASS()
class PHYSCUSTOMMOVEMENTS_API UPMCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
	friend class FPMSavedMove;
	friend class FPMCharacterNetworkMoveData;

protected:
	TSharedPtr<FPhysCustomMovement> PhysCustomMovement;
	uint8 bWantsPhysCustomMovement : 1;
	FPMCharacterNetworkMoveDataContainer CustomCharacterNetworkMoveDataContainer;

public:
	UPMCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	// UCharacterMovementComponent API
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual float GetMaxAcceleration() const;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	// ~UCharacterMovementComponent

	// Physics Custom Movement API
	/** Starts the Phys Custom Movement. */
	bool StartPhysCustomMovement(const TSharedPtr<FPhysCustomMovement>& inPhysCustomMovement);

	UFUNCTION(BlueprintCallable, Category = "Phys Custom Movement")
	bool IsPhysCustomMovementActive() const;
	
	/** Stop Phys Custom Movement. Called when ending movement from UCharacterMovementComponent::SetMovementMode. */
	virtual void StopPhysCustomMovement();
	
	// NOTE: this MUST match the selected custom flag for the 'bWantsPhysCustomMovement' in FPMSavedMove::GetCompressedFlags
	uint8 GetPhysCustomMovementModeFlag() const;
	// ~Physics Custom Movement API

protected:
	// UCharacterMovementComponent API
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel);
	/** Event notification when client receives correction data from the server, before applying the data. Base implementation logs relevant data and draws debug info if "p.NetShowCorrections" is not equal to 0. */
	virtual void OnClientCorrectionReceived(class FNetworkPredictionData_Client_Character& ClientData, float TimeStamp, FVector NewLocation, FVector NewVelocity, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode) override;
	// ~UCharacterMovementComponent
};
