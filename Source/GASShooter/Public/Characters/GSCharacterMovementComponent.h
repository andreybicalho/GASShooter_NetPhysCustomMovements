// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"
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

	// Sprint
	uint8 SavedRequestToStartSprinting : 1;

	// Aim Down Sights
	uint8 SavedRequestToStartADS : 1;

	// Any custom physics movement
	uint8 SavedRequestToStartCustomMovement : 1;
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

	// TODO: refactor to shared pointer
	//TSharedPtr<FPhysCustomMovement> PhysCustomMovement;
	FPhysCustomMovement* PhysCustomMovement = nullptr;

public:
	UGSCharacterMovementComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float SprintSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float ADSSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float KnockedDownSpeedMultiplier;

	uint8 RequestToStartSprinting : 1;
	uint8 RequestToStartADS : 1;
	uint8 RequestToStartPhysCustomMovement : 1;

	FGameplayTag KnockedDownTag;
	FGameplayTag InteractingTag;
	FGameplayTag InteractingRemovalTag;

	virtual float GetMaxSpeed() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	// Sprint
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StartSprinting();
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StopSprinting();

	// Aim Down Sights
	UFUNCTION(BlueprintCallable, Category = "Aim Down Sights")
	void StartAimDownSights();
	UFUNCTION(BlueprintCallable, Category = "Aim Down Sights")
	void StopAimDownSights();

	// Physics Custom Movement API
	// TODO: refactor to shared pointer?
	//virtual void StartPhysCustomMovement(TSharedPtr<FPhysCustomMovement> inPhysCustomMovement);
	//UFUNCTION(BlueprintCallable, Category = "Phys Custom Movement")
	virtual void StartPhysCustomMovement(FPhysCustomMovement& inPhysCustomMovement);
	UFUNCTION(BlueprintCallable, Category = "Phys Custom Movement")
	virtual void StopPhysCustomMovement(const EMovementMode nextMovementMode);
	UFUNCTION(BlueprintCallable, Category = "Phys Custom Movement")
	virtual bool IsPhysCustomMovementActive() const;
	// NOTE: this MUST match the selected custom flag for the 'RequestToStartPhysCustomMovement' in FGSSavedMove::GetCompressedFlags
	virtual uint8 GetPhysCustomMovementModeFlag() const;
	// ~Physics Custom Movement API
};
