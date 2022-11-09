// Copyright 2022 Andrey Bicalho.


#include "Characters/GSCharacterMovementComponent.h"
#include "Characters/GSCharacterBase.h"

UGSCharacterMovementComponent::UGSCharacterMovementComponent()
{
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

	if (RequestToStartPhysCustomMovement && PhysCustomMovement && PhysCustomMovement->IsActive())
	{
		//UE_LOG(LogTemp, Display, TEXT("%s: %s"), *FString(__FUNCTION__), GET_ACTOR_ROLE_FSTRING(GetCharacterOwner()));
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
		//UE_LOG(LogTemp, Display, TEXT("%s: %s"), *FString(__FUNCTION__), GET_ACTOR_ROLE_FSTRING(GetCharacterOwner()));
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
	if (PhysCustomMovement && PhysCustomMovement->IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: %s: %s is still active. If you want to start %s, wait till the other movement is done or manually stop it."),
			*FString(__FUNCTION__), 
			GET_ACTOR_ROLE_FSTRING(GetCharacterOwner()),
			*PhysCustomMovement->MovementName.ToString(),
			*inPhysCustomMovement.MovementName.ToString());

		return;
	}

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
	return FGSSavedMove::FLAG_Custom_0;
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

void FGSSavedMove::Clear()
{
	Super::Clear();

	SavedRequestToStartCustomMovement = false;
}

uint8 FGSSavedMove::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (SavedRequestToStartCustomMovement)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

bool FGSSavedMove::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	//Set which moves can be combined together. This will depend on the bit flags that are used.

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

