// Copyright 2022 Andrey Bicalho.

#include "Components/PMCharacterMovementComponent.h"
#include "GameFramework/Character.h"

DEFINE_LOG_CATEGORY(LogPhysCustomMovement);

DECLARE_CYCLE_STAT(TEXT("Char PhysCustom"), STAT_UPMCharacterMovementComponent_PhysCustom, STATGROUP_Character);

UPMCharacterMovementComponent::UPMCharacterMovementComponent()
{
}

float UPMCharacterMovementComponent::GetMaxSpeed() const
{
	// NOTE: this only returns for Autonomous Proxy and Authority.
	// since SimulatedProxies don't have a copy of the movement if we ever need the max speed for them we should replicate or change design so they also run movement.
	if (bWantsPhysCustomMovement && PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive())
	{
		return MaxCustomMovementSpeed;
		//return PhysCustomMovement->GetMaxSpeed(); // NOTE: equivalent to the above since we also set MaxCustomMovementSpeed during custom movement execution.
	}

	return Super::GetMaxSpeed();
}

void UPMCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	//The Flags parameter contains the compressed input flags that are stored in the saved move.
	//UpdateFromCompressed flags simply copies the flags from the saved move into the movement component.
	//It basically just resets the movement component to the state when the move was made so it can simulate from there.

	bWantsPhysCustomMovement = (Flags & GetPhysCustomMovementModeFlag()) != 0;
}

FNetworkPredictionData_Client* UPMCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);

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
			if (!PhysCustomMovement->IsActive())
			{
				// TODO: check if this is even reachable with the new flow
				UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: Movement Mode is valid but it is inactive. Movement mode will be set to %s."), 
					ANSI_TO_TCHAR(__FUNCTION__),
					GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
					*UEnum::GetValueAsString(PhysCustomMovement->FallbackMovementMode));

				SetMovementMode(PhysCustomMovement->FallbackMovementMode);
				StartNewPhysics(deltaTime, Iterations);
				return;
			}

			if (PhysCustomMovement->CanDoMovement(deltaTime))
			{
				// TODO: should only update velocity if we didn't teleport and if we don't have any root motion source?				
				//if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())

				const FVector oldVelocity = Velocity;
				PhysCustomMovement->UpdateMovement(deltaTime, oldVelocity, Velocity);

				// TODO: we probably want to call take into account things like braking, friction, deceleration and also RVO stuff (although RVO only works for walking and navwalking modes) check CalcVelocity usage:
				/*const float friction = 0.5f * GetPhysicsVolume()->FluidFriction;
				CalcVelocity(deltaTime, friction, true, GetMaxBrakingDeceleration());*/

				const FVector adjustedVelocity = Velocity * deltaTime;
				const FVector oldLocation = UpdatedComponent->GetComponentLocation();
				FHitResult hit(1.f);
				SafeMoveUpdatedComponent(adjustedVelocity, UpdatedComponent->GetComponentQuat(), true, hit);

				// update velocity with what we really moved
				Velocity = (UpdatedComponent->GetComponentLocation() - oldLocation) / deltaTime;
				UpdateComponentVelocity();

				// update acceleration
				if (Acceleration.SizeSquared() > SMALL_NUMBER)
				{
					Acceleration = Acceleration.GetSafeNormal() * GetMaxAcceleration();
				}
				else
				{
					Acceleration = GetMaxAcceleration() * (Velocity.SizeSquared() < SMALL_NUMBER ? UpdatedComponent->GetForwardVector() : Velocity.GetSafeNormal());
				}

				AnalogInputModifier = ComputeAnalogInputModifier(); // recompute since acceleration may have changed.
			}
			else
			{
				UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: Movement %s requirements has failed! Movement mode will be set to %s."),
					ANSI_TO_TCHAR(__FUNCTION__),
					GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
					*PhysCustomMovement->MovementName.ToString(),
					*UEnum::GetValueAsString(PhysCustomMovement->FallbackMovementMode));

				SetMovementMode(PhysCustomMovement->FallbackMovementMode);
				StartNewPhysics(deltaTime, Iterations);
			}
		}
		else
		{
			// TODO: figure it out what to do with this case... give it time to sync or stop it right away?
			UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: Phys Custom Movement FLAG is Set but movement is invalid."),
				ANSI_TO_TCHAR(__FUNCTION__),
				GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()));

			/*SetMovementMode(MOVE_Falling);
			StartNewPhysics(deltaTime, Iterations);*/
		}
	}
	else
	{
		UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: CustomMovementMode doesn't match our Phys Custom Movement Mode Flag. Are you sure you want to run another custom movement?"),
			ANSI_TO_TCHAR(__FUNCTION__),
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
			ANSI_TO_TCHAR(__FUNCTION__),
			GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
			*PhysCustomMovement->MovementName.ToString(),
			*inPhysCustomMovement->MovementName.ToString());

		return false;
	}

	PhysCustomMovement = inPhysCustomMovement;

	if (inPhysCustomMovement.IsValid())
	{
		UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: Starting Movement: %s"),
			ANSI_TO_TCHAR(__FUNCTION__), GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
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
		ANSI_TO_TCHAR(__FUNCTION__),
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
				ANSI_TO_TCHAR(__FUNCTION__),
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

	if (bSavedWantsPhysCustomMovement != ((FPMSavedMove*)NewMove.Get())->bSavedWantsPhysCustomMovement)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FPMSavedMove::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UPMCharacterMovementComponent* CharacterMovement = Cast<UPMCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		// Copy values into the saved move
		bSavedWantsPhysCustomMovement = CharacterMovement->bWantsPhysCustomMovement;
	}
}

void FPMSavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UPMCharacterMovementComponent* CharacterMovement = Cast<UPMCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		// Copy values out of the saved move
		CharacterMovement->bWantsPhysCustomMovement = bSavedWantsPhysCustomMovement;
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
