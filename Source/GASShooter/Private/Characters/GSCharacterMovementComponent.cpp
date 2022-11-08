// Copyright 2022 Andrey Bicalho.


#include "Characters/GSCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Characters/Abilities/GSAbilitySystemGlobals.h"
#include "Characters/GSCharacterBase.h"
#include "GameplayTagContainer.h"

UGSCharacterMovementComponent::UGSCharacterMovementComponent()
{
	SprintSpeedMultiplier = 1.4f;
	ADSSpeedMultiplier = 0.8f;
	KnockedDownSpeedMultiplier = 0.4f;

	KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
	InteractingTag = FGameplayTag::RequestGameplayTag("State.Interacting");
	InteractingRemovalTag = FGameplayTag::RequestGameplayTag("State.InteractingRemoval");
}

float UGSCharacterMovementComponent::GetMaxSpeed() const
{
	AGSCharacterBase* Owner = Cast<AGSCharacterBase>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() No Owner"), *FString(__FUNCTION__));
		return Super::GetMaxSpeed();
	}

	if (!Owner->IsAlive())
	{
		return 0.0f;
	}

	// Don't move while interacting or being interacted on (revived)
	if (Owner->GetAbilitySystemComponent() && Owner->GetAbilitySystemComponent()->GetTagCount(InteractingTag)
		> Owner->GetAbilitySystemComponent()->GetTagCount(InteractingRemovalTag))
	{
		return 0.0f;
	}

	if (Owner->GetAbilitySystemComponent() && Owner->GetAbilitySystemComponent()->HasMatchingGameplayTag(KnockedDownTag))
	{
		return Owner->GetMoveSpeed() * KnockedDownSpeedMultiplier;
	}

	if (RequestToStartSprinting)
	{
		return Owner->GetMoveSpeed() * SprintSpeedMultiplier;
	}

	if (RequestToStartADS)
	{
		return Owner->GetMoveSpeed() * ADSSpeedMultiplier;
	}

	if (RequestToStartPhysCustomMovement && PhysCustomMovement && PhysCustomMovement->IsActive())
	{
		UE_LOG(LogTemp, Display, TEXT("%s: %s"), *FString(__FUNCTION__), GET_ACTOR_ROLE_FSTRING(GetCharacterOwner()));
		// TODO: could use attribute set to hold a multiplier for the custom movement mode so different characters have different speeds
		//return PhysCustomMovement->GetMaxSpeed() * attributeSetMultiplier;
		return PhysCustomMovement->GetMaxSpeed();
	}

	return Owner->GetMoveSpeed();
}

void UGSCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	//The Flags parameter contains the compressed input flags that are stored in the saved move.
	//UpdateFromCompressed flags simply copies the flags from the saved move into the movement component.
	//It basically just resets the movement component to the state when the move was made so it can simulate from there.
	RequestToStartSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;

	RequestToStartADS = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;

	RequestToStartPhysCustomMovement = (Flags & GetPhysCustomMovementModeFlag()) != 0;
}

FNetworkPredictionData_Client* UGSCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);

	if (!ClientPredictionData)
	{
		UGSCharacterMovementComponent* MutableThis = const_cast<UGSCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FGSNetworkPredictionData_Client(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}


void UGSCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	// Phys* functions should only run for characters with ROLE_Authority or ROLE_AutonomousProxy. However, Unreal calls PhysCustom in
	// two separate locations, one of which doesn't check the role, so we must check it here to prevent this code from running on simulated proxies.
	if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}

	if (PhysCustomMovement && CustomMovementMode == GetPhysCustomMovementModeFlag())
	{
		UE_LOG(LogTemp, Display, TEXT("%s: %s"), *FString(__FUNCTION__), GET_ACTOR_ROLE_FSTRING(GetCharacterOwner()));
		if (!PhysCustomMovement->IsActive())
		{
			StopPhysCustomMovement();
			Super::PhysCustom(deltaTime, Iterations);
			return;
		}

		FVector newVelocity = FVector::ZeroVector;
		PhysCustomMovement->UpdateMovement(deltaTime, Velocity, newVelocity);
		Velocity = newVelocity;

		const FVector adjustedVelocity = Velocity * deltaTime;
		FHitResult hit(1.f);
		SafeMoveUpdatedComponent(adjustedVelocity, UpdatedComponent->GetComponentQuat(), true, hit);
	}

	// Not sure if this is needed
	Super::PhysCustom(deltaTime, Iterations);
}

void UGSCharacterMovementComponent::StartPhysCustomMovement(FPhysCustomMovement& inPhysCustomMovement)
{
	PhysCustomMovement = &inPhysCustomMovement;
	
	if (PhysCustomMovement)
	{
		UE_LOG(LogTemp, Display, TEXT("%s: %s: Requested To Start Custom Movement: %s"),
			*FString(__FUNCTION__), GET_ACTOR_ROLE_FSTRING(GetCharacterOwner()),
			*PhysCustomMovement->MovementName.ToString());

		RequestToStartPhysCustomMovement = PhysCustomMovement->BeginMovement(
			GetCharacterOwner(), 
			this, 
			GetPhysCustomMovementModeFlag());
	}
}
//void UGSCharacterMovementComponent::StartPhysCustomMovement(TSharedPtr<FPhysCustomMovement> inPhysCustomMovement)
//{
//	if (ensure(inPhysCustomMovement.IsValid()))
//	{
//		PhysCustomMovement = inPhysCustomMovement.Get();
//
//		RequestToStartPhysCustomMovement = PhysCustomMovement->BeginMovement(
//			GetCharacterOwner(),
//			this,
//			GetPhysCustomMovementModeFlag());
//	}
//}

void UGSCharacterMovementComponent::StopPhysCustomMovement()
{
	RequestToStartPhysCustomMovement = false;
		
	if (PhysCustomMovement && PhysCustomMovement->IsActive())
	{
		UE_LOG(LogTemp, Display, TEXT("%s: %s: Requested To Stop Custom Movement: %s"),
			*FString(__FUNCTION__), 
			GET_ACTOR_ROLE_FSTRING(GetCharacterOwner()),
			*PhysCustomMovement->MovementName.ToString());

		PhysCustomMovement->EndMovement();
		PhysCustomMovement = nullptr;
	}
}

bool UGSCharacterMovementComponent::IsPhysCustomMovementActive() const
{
	return PhysCustomMovement && PhysCustomMovement->IsActive();
}

uint8 UGSCharacterMovementComponent::GetPhysCustomMovementModeFlag() const
{
	// NOTE: this MUST match the selected custom flag for the 'RequestToStartPhysCustomMovement' in FGSSavedMove::GetCompressedFlags
	return FGSSavedMove::FLAG_Custom_2;
}


void UGSCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == GetPhysCustomMovementModeFlag())
	{
		if (PhysCustomMovement && PhysCustomMovement->IsActive())
		{
			//UE_LOG(LogTemp, Display, TEXT("%s: %s"), *FString(__FUNCTION__), GET_ACTOR_ROLE_FSTRING(GetCharacterOwner()));
			StopPhysCustomMovement();
		}
	}
}

void UGSCharacterMovementComponent::StartSprinting()
{
	RequestToStartSprinting = true;
}

void UGSCharacterMovementComponent::StopSprinting()
{
	RequestToStartSprinting = false;
}

void UGSCharacterMovementComponent::StartAimDownSights()
{
	RequestToStartADS = true;
}

void UGSCharacterMovementComponent::StopAimDownSights()
{
	RequestToStartADS = false;
}

void FGSSavedMove::Clear()
{
	Super::Clear();

	SavedRequestToStartSprinting = false;
	SavedRequestToStartADS = false;
	SavedRequestToStartCustomMovement = false;
}

uint8 FGSSavedMove::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (SavedRequestToStartSprinting)
	{
		Result |= FLAG_Custom_0;
	}

	if (SavedRequestToStartADS)
	{
		Result |= FLAG_Custom_1;
	}

	if (SavedRequestToStartCustomMovement)
	{
		Result |= FLAG_Custom_2;
	}

	return Result;
}

bool FGSSavedMove::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	//Set which moves can be combined together. This will depend on the bit flags that are used.
	if (SavedRequestToStartSprinting != ((FGSSavedMove*)NewMove.Get())->SavedRequestToStartSprinting)
	{
		return false;
	}

	if (SavedRequestToStartADS != ((FGSSavedMove*)NewMove.Get())->SavedRequestToStartADS)
	{
		return false;
	}

	if (SavedRequestToStartCustomMovement != ((FGSSavedMove*)NewMove.Get())->SavedRequestToStartCustomMovement)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FGSSavedMove::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UGSCharacterMovementComponent* CharacterMovement = Cast<UGSCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		// Copy values into the saved move
		SavedRequestToStartSprinting = CharacterMovement->RequestToStartSprinting;
		SavedRequestToStartADS = CharacterMovement->RequestToStartADS;

		SavedRequestToStartCustomMovement = CharacterMovement->RequestToStartPhysCustomMovement;
	}
}

void FGSSavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UGSCharacterMovementComponent* CharacterMovement = Cast<UGSCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		// Copy values out of the saved move
		CharacterMovement->RequestToStartPhysCustomMovement = SavedRequestToStartCustomMovement;
	}
}

FGSNetworkPredictionData_Client::FGSNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FGSNetworkPredictionData_Client::AllocateNewMove()
{
	return FSavedMovePtr(new FGSSavedMove());
}

