// Copyright 2022 Andrey Bicalho.


#include "Components/PMCharacterMovementComponent.h"
#include "GameFramework/Character.h"
//#include "Movements/PhysCustomMovement_NonDeterministicMove.h" // TODO: remove this when figure out how to bind non predicted data dynamically

DEFINE_LOG_CATEGORY(LogPhysCustomMovement);

DECLARE_CYCLE_STAT(TEXT("Char PhysCustom"), STAT_UPMCharacterMovementComponent_PhysCustom, STATGROUP_Character);

UPMCharacterMovementComponent::UPMCharacterMovementComponent()
{
}

float UPMCharacterMovementComponent::GetMaxSpeed() const
{
	// TODO: this only returns for Autonomous Proxy and Authority.
	// since SimulatedProxies don't have a copy of the movement if we ever need that max speed we should replicate the max speed
	if (bWantsPhysCustomMovement && PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive())
	{
		//UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s"), *FString(__FUNCTION__), GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()));
		return MaxCustomMovementSpeed;
		//return PhysCustomMovement->GetMaxSpeed();
	}

	return Super::GetMaxSpeed();
}

void UPMCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	//This is called on the server when processing a move with a given timestamp.
	//The Flags parameter contains the compressed input flags that are stored in the saved move.
	//UpdateFromCompressed flags simply copies the flags from the saved move into the movement component.
	//It basically just resets the movement component to the state when the move was made so it can simulate from there.

	bWantsPhysCustomMovement = (Flags & GetPhysCustomMovementModeFlag()) != 0;

	/*UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s"),
		*FString(__FUNCTION__),
		GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()));*/
}

FNetworkPredictionData_Client* UPMCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);

	/*UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s"),
		*FString(__FUNCTION__),
		GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()));*/

	if (!ClientPredictionData)
	{
		UPMCharacterMovementComponent* MutableThis = const_cast<UPMCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FPMNetworkPredictionData_Client(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UPMCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	// Phys* functions should only run for characters with ROLE_Authority or ROLE_AutonomousProxy. However, Unreal calls PhysCustom in
	// two separate locations, one of which doesn't check the role, so we must check it here to prevent this code from running on simulated proxies.
	if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy || deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	SCOPED_NAMED_EVENT(UPMCharacterMovementComponent_PhysCustom, FColor::Yellow);
	SCOPE_CYCLE_COUNTER(STAT_UPMCharacterMovementComponent_PhysCustom);

	// TODO: implement physics sub-stepping
	// begin physics sub-stepping
	//float remainingTime = deltaTime;

	//while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner)
	//{
	//	Iterations++;
	//	const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
	//	remainingTime -= timeTick;

	//	// TODO: movement code goes here...
	//}
	// ~end physics sub-stepping

	if (CustomMovementMode == GetPhysCustomMovementModeFlag())
	{
		if (PhysCustomMovement.IsValid())
		{
			//UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s"), *FString(__FUNCTION__), GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()));
			if (!PhysCustomMovement->IsActive())
			{
				// TODO: check if this is even reachable with the new flow
				UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: Movement Mode is valid but it is inactive. Movement mode will be set to %s."), 
					*FString(__FUNCTION__), 
					GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
					*UEnum::GetValueAsString(PhysCustomMovement->FallbackMovementMode));

				SetMovementMode(PhysCustomMovement->FallbackMovementMode);
				StartNewPhysics(deltaTime, Iterations); // TODO: check this
				return;
			}

			if (PhysCustomMovement->CanDoMovement(deltaTime))
			{
				const FVector oldVelocity = Velocity;
				PhysCustomMovement->UpdateMovement(deltaTime, oldVelocity, Velocity);

				// TODO: we probably want to call CalcVelocity since it handles braking, friction, deceleration and also RVO stuff (although RVO only works for walking and navwalking modes)
				/*const float friction = 0.5f * GetPhysicsVolume()->FluidFriction;
				CalcVelocity(deltaTime, friction, true, GetMaxBrakingDeceleration());*/

				const FVector adjustedVelocity = Velocity * deltaTime;
				const FVector oldLocation = UpdatedComponent->GetComponentLocation();
				FHitResult hit(1.f);
				SafeMoveUpdatedComponent(adjustedVelocity, UpdatedComponent->GetComponentQuat(), true, hit);

				if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
				{
					// update velocity with what we really moved
					Velocity = (UpdatedComponent->GetComponentLocation() - oldLocation) / deltaTime;  
					UpdateComponentVelocity();
				}
			}
			else
			{
				UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: Movement %s requirements has failed! Movement mode will be set to %s."), 
					*FString(__FUNCTION__), 
					GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
					*PhysCustomMovement->MovementName.ToString(),
					*UEnum::GetValueAsString(PhysCustomMovement->FallbackMovementMode));

				SetMovementMode(PhysCustomMovement->FallbackMovementMode);
				StartNewPhysics(deltaTime, Iterations); // TODO: check this
			}
		}
		else
		{
			UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: Phys Custom Movement FLAG is Set but movement is not valid. Movement mode will be set to Falling."), 
				*FString(__FUNCTION__), 
				GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()));

			SetMovementMode(MOVE_Falling);
			StartNewPhysics(deltaTime, Iterations); // TODO: check this
		}
	}
	else
	{
		UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: CustomMovementMode doesn't match our Phys Custom Movement Mode Flag. Are you sure you want to run another custom movement?"), 
			*FString(__FUNCTION__), 
			GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()));
	}

	// Not sure if this is needed
	Super::PhysCustom(deltaTime, Iterations);
}

bool UPMCharacterMovementComponent::StartPhysCustomMovement(TSharedPtr<FPhysCustomMovement> inPhysCustomMovement)
{
	if (PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive())
	{
		UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: %s is still active. If you want to start %s, wait till that movement is done or manually stop it."),
			*FString(__FUNCTION__),
			GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
			*PhysCustomMovement->MovementName.ToString(),
			*inPhysCustomMovement->MovementName.ToString());

		return false;
	}

	PhysCustomMovement = inPhysCustomMovement;

	if (inPhysCustomMovement.IsValid())
	{
		UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: Starting Movement: %s"),
			*FString(__FUNCTION__), GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
			*PhysCustomMovement->MovementName.ToString());

		bWantsPhysCustomMovement = PhysCustomMovement->BeginMovement(
			GetCharacterOwner(),
			this,
			GetPhysCustomMovementModeFlag());
	}

	return bWantsPhysCustomMovement;
}

void UPMCharacterMovementComponent::StopPhysCustomMovement()
{
	bWantsPhysCustomMovement = false;

	UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: Ending Movement: %s"),
		*FString(__FUNCTION__),
		GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
		PhysCustomMovement.IsValid() ? *PhysCustomMovement->MovementName.ToString() : TEXT("Invalid"));

	if (PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive())
	{
		PhysCustomMovement->EndMovement();
	}

	PhysCustomMovement.Reset();
}

bool UPMCharacterMovementComponent::IsPhysCustomMovementActive() const
{
	return PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive();
}

uint8 UPMCharacterMovementComponent::GetPhysCustomMovementModeFlag() const
{
	// NOTE: this MUST match the selected custom flag for the 'bWantsPhysCustomMovement' in FPMSavedMove::GetCompressedFlags
	return FPMSavedMove::FLAG_Custom_0;
}


void UPMCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	// NOTE: only Autonomous Proxy and Authority should stop the phys custom movement since only them has an instance of that
	if (GetOwner()->GetLocalRole() >= ROLE_AutonomousProxy)
	{
		// TODO: should check if we are changing from a phys custom movement to another phys custom movement?
		if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == GetPhysCustomMovementModeFlag())
		{
			UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: Movement Mode Changed from %s to %s during Movement %s. Requesting to Stop the Phys Custom Movement..."),
				*FString(__FUNCTION__),
				GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
				*UEnum::GetValueAsString(PreviousMovementMode),
				*UEnum::GetValueAsString(MovementMode),
				PhysCustomMovement.IsValid() ? *PhysCustomMovement->MovementName.ToString() : TEXT("Invalid"));

			StopPhysCustomMovement();
		}
	}
}

void FPMSavedMove::Clear()
{
	Super::Clear();

	bSavedWantsPhysCustomMovement = false;

	// TODO: should clear non predicted data here?
	/*waitTime = 99.f;
	movementDirectionSign = 1.f;*/
}

uint8 FPMSavedMove::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedWantsPhysCustomMovement)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

bool FPMSavedMove::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	//Set which moves can be combined together. This will depend on the bit flags that are used.

	UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: bSavedWantsPhysCustomMovement = %d / SavedMove->bSavedWantsPhysCustomMovement %d"),
		*FString(__FUNCTION__),
		GET_ACTOR_LOCAL_ROLE_FSTRING(Character), 
		bSavedWantsPhysCustomMovement,
		((FPMSavedMove*)NewMove.Get())->bSavedWantsPhysCustomMovement);

	if (bSavedWantsPhysCustomMovement != ((FPMSavedMove*)NewMove.Get())->bSavedWantsPhysCustomMovement)
	{
		return false;
	}

	// TODO: should check for non predicted data?
	/*if (UPMCharacterMovementComponent* characterMovement = Cast<UPMCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		if (auto movement = static_cast<FPhysCustomMovement_NonDeterministicMove*>(characterMovement->PhysCustomMovement.Get()))
		{
			UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: waitTime = %.2f --- movementDirectionSign = %.2f (%.2f, %.2f)"),
				*FString(__FUNCTION__),
				GET_ACTOR_LOCAL_ROLE_FSTRING(Character),
				movement->TimeToWait,
				movement->MovementDirectionSign,
				((FPMSavedMove*)NewMove.Get())->waitTime,
				((FPMSavedMove*)NewMove.Get())->movementDirectionSign);

			return FMath::IsNearlyEqual(movement->TimeToWait, ((FPMSavedMove*)NewMove.Get())->waitTime, KINDA_SMALL_NUMBER)
				&& FMath::IsNearlyEqual(movement->MovementDirectionSign, ((FPMSavedMove*)NewMove.Get())->movementDirectionSign, KINDA_SMALL_NUMBER);
		}
	}*/

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FPMSavedMove::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	if (UPMCharacterMovementComponent* characterMovement = Cast<UPMCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		// Copy values into the saved move
		bSavedWantsPhysCustomMovement = characterMovement->bWantsPhysCustomMovement;

		// TODO: should set non deterministic data here as well?
		//if (auto movement = static_cast<FPhysCustomMovement_NonDeterministicMove*>(characterMovement->PhysCustomMovement.Get()))
		//{
		//	/*UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: waitTime = %.2f --- movementDirectionSign = %.2f"),
		//		*FString(__FUNCTION__),
		//		GET_ACTOR_LOCAL_ROLE_FSTRING(Character),
		//		movement->TimeToWait,
		//		movement->MovementDirectionSign);*/

		//	waitTime = movement->TimeToWait;
		//	movementDirectionSign = movement->MovementDirectionSign;
		//}
	}
}

void FPMSavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

 	if (UPMCharacterMovementComponent* characterMovement = Cast<UPMCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		// Copy values out of the saved move
		characterMovement->bWantsPhysCustomMovement = bSavedWantsPhysCustomMovement;

		// TODO: should apply non predicted values here?
		//if (auto movement = static_cast<FPhysCustomMovement_NonDeterministicMove*>(characterMovement->PhysCustomMovement.Get()))
		//{
		//	/*UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: waitTime = %.2f --- movementDirectionSign = %.2f"),
		//		*FString(__FUNCTION__),
		//		GET_ACTOR_LOCAL_ROLE_FSTRING(Character),
		//		movement->TimeToWait,
		//		movement->MovementDirectionSign);*/

		//	movement->TimeToWait = waitTime;
		//	movement->MovementDirectionSign = movementDirectionSign;
		//}
	}
}

FPMNetworkPredictionData_Client::FPMNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FPMNetworkPredictionData_Client::AllocateNewMove()
{
	return FSavedMovePtr(new FPMSavedMove());
}
