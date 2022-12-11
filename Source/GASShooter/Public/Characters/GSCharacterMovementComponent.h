// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysCustomMovement.h"
#include "GSCharacterMovementComponent.generated.h"


class FGSSavedMove : public FSavedMove_Character
{
public:

	typedef FSavedMove_Character Super;

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
};

class FGSNetworkPredictionData_Client : public FNetworkPredictionData_Client_Character
{
public:
	FGSNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement);

	typedef FNetworkPredictionData_Client_Character Super;

	///@brief Allocates a new copy of our custom saved move
	virtual FSavedMovePtr AllocateNewMove() override;
};


/**
 * 
 */
UCLASS()
class GASSHOOTER_API UGSCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend class FGSSavedMove;

	TSharedPtr<FPhysCustomMovement> PhysCustomMovement;

public:
	UGSCharacterMovementComponent();

	uint8 bWantsPhysCustomMovement : 1;

	virtual float GetMaxSpeed() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	// Physics Custom Movement API
	/** Starts the Phys Custom Movement. */
	virtual bool StartPhysCustomMovement(TSharedPtr<FPhysCustomMovement> inPhysCustomMovement);
	/** Called when ending Phys Custom Movement from UCharacterMovementComponent::SetMovementMode. */
	virtual void OnPhysCustomMovementEnd();
	UFUNCTION(BlueprintCallable, Category = "Phys Custom Movement")
	virtual bool IsPhysCustomMovementActive() const;
	// NOTE: this MUST match the selected custom flag for the 'bWantsPhysCustomMovement' in FGSSavedMove::GetCompressedFlags
	virtual uint8 GetPhysCustomMovementModeFlag() const;
	// ~Physics Custom Movement API

protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
};
