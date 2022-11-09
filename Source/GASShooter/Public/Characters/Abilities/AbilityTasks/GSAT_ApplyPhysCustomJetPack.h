// Copyright 2022 Andrey Bicalho.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/AbilityTasks/GSAT_ApplyPhysCustomMovementBase.h"
#include "Characters/PhysCustomMovement.h"
#include "GameFramework/Character.h"
#include "GSAT_ApplyPhysCustomJetPack.generated.h"

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
		CurrentTime += deltaTime;

		outVelocity = oldVelocity;
		
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
 * 
 */
UCLASS()
class GASSHOOTER_API UGSAT_ApplyPhysCustomJetPack : public UGSAT_ApplyPhysCustomMovementBase
{
	GENERATED_BODY()

	FVector JetPackAcceleration = FVector::ZeroVector;

	FPhysCustomMovement_JetPack PhysJetPackMovement;

public:
	UGSAT_ApplyPhysCustomJetPack(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_ApplyPhysCustomJetPack* PhysJetPack(UGameplayAbility* OwningAbility, FName TaskInstanceName, const FVector& inJetPackAcceleration);

	virtual void Activate() override;

	virtual void OnDestroy(bool AbilityIsEnding) override;

	virtual void Finish() override;

protected:
	virtual void InitAndApply() override;

	UFUNCTION()
	void OnPhysJetPackEnded();
};
