// Copyright 2022 Andrey Bicalho.

#include "Components/PMCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Movements/PhysCustomMovement_NonDeterministicMove.h" // TODO: remove this when figure out how to bind non predicted data dynamically

DEFINE_LOG_CATEGORY(LogPhysCustomMovement);

DECLARE_CYCLE_STAT(TEXT("Char PhysCustom"), STAT_UPMCharacterMovementComponent_PhysCustom, STATGROUP_Character);

#define PMC_DEBUG_VERBOSE !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

UPMCharacterMovementComponent::UPMCharacterMovementComponent(const FObjectInitializer& ObjectInitializer) 
	: UCharacterMovementComponent(ObjectInitializer)
{
	SetNetworkMoveDataContainer(CustomCharacterNetworkMoveDataContainer);
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

float UPMCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (bWantsPhysCustomMovement && PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive())
	{
		return PhysCustomMovement->GetMaxBrakingDeceleration();
	}

	return Super::GetMaxBrakingDeceleration();
}

float UPMCharacterMovementComponent::GetMaxAcceleration() const
{
	if (bWantsPhysCustomMovement && PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive())
	{
		return PhysCustomMovement->GetMaxAcceleration();
	}

	return Super::GetMaxAcceleration();
}

void UPMCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	//This is called on the server when processing a move with a given timestamp.
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

	if (MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == GetPhysCustomMovementModeFlag())
	{
		if (PhysCustomMovement.IsValid())
		{
			if (!PhysCustomMovement->IsActive())
			{
#if PMC_DEBUG_VERBOSE
				UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: Movement Mode is valid but it is inactive. Movement mode will be set to %s."), 
					ANSI_TO_TCHAR(__FUNCTION__),
					GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
					*UEnum::GetValueAsString(PhysCustomMovement->FallbackMovementMode));
#endif // PMC_DEBUG_VERBOSE
				SetMovementMode(PhysCustomMovement->FallbackMovementMode);
				StartNewPhysics(deltaTime, Iterations);
				return;
			}

			if (PhysCustomMovement->CanDoMovement(deltaTime))
			{
				if (PhysCustomMovement->SkipThisUpdate(deltaTime))
				{
#if PMC_DEBUG_VERBOSE
					UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: Update for movement %s will be skipped to prevent more degradation. Time: %.2fs"),
						ANSI_TO_TCHAR(__FUNCTION__),
						GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
						*PhysCustomMovement->MovementName.ToString(),
						PhysCustomMovement->TimeSkippingMovement);
#endif // PMC_DEBUG_VERBOSE
					return;
				}

				// TODO: should also prevent to update velocity if has just teleported (bJustTeleported)?
				if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
				{
#if PMC_DEBUG_VERBOSE
					UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: %s will not update velocity due to following: HasAnimRootMotion: %d or CurrentRootMotion Has Override Velocity: %d"),
						ANSI_TO_TCHAR(__FUNCTION__),
						GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
						*PhysCustomMovement->MovementName.ToString(),
						HasAnimRootMotion(),
						CurrentRootMotion.HasOverrideVelocity());
#endif // PMC_DEBUG_VERBOSE
					return;
				}

				// Apply acceleration
				const float friction = 0.f; // TODO: create a custom friction: PhysCustomMovement->GetFriction() (something like: GetPhysicsVolume()->FluidFriction * CustomMovementFrictionMultiplier)
				CalcVelocity(deltaTime, friction, false, GetMaxBrakingDeceleration());

				// override velocity with the custom movement logic
				const FVector oldVelocity = Velocity;
				PhysCustomMovement->UpdateMovement(deltaTime, oldVelocity, Velocity);

				// move
				const FVector adjustedVelocity = Velocity * deltaTime;
				const FVector oldLocation = UpdatedComponent->GetComponentLocation();
				FHitResult hit(1.f);
				SafeMoveUpdatedComponent(adjustedVelocity, UpdatedComponent->GetComponentQuat(), true, hit);

				// update velocity with what we really moved
				Velocity = (UpdatedComponent->GetComponentLocation() - oldLocation) / deltaTime; // v = dx / dt
				UpdateComponentVelocity();

				// Update acceleration (NOTE: if this is done before we update custom movement we can't have inputs during the movement)
				Acceleration = GetMaxAcceleration() * Velocity.GetSafeNormal();
				Acceleration = Acceleration.GetClampedToMaxSize(GetMaxAcceleration());
				AnalogInputModifier = ComputeAnalogInputModifier(); // recompute since acceleration have changed.
			}
			else
			{
#if PMC_DEBUG_VERBOSE
				UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: Movement %s requirements has failed! Movement mode will be set to %s. Iterations: %d"),
					ANSI_TO_TCHAR(__FUNCTION__),
					GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
					*PhysCustomMovement->MovementName.ToString(),
					*UEnum::GetValueAsString(PhysCustomMovement->FallbackMovementMode),
					Iterations);
#endif // PMC_DEBUG_VERBOSE
				SetMovementMode(PhysCustomMovement->FallbackMovementMode);
				StartNewPhysics(deltaTime, Iterations);
			}
		}
		else
		{
#if PMC_DEBUG_VERBOSE
			UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: Phys Custom Movement FLAG is Set but movement is invalid. Iterations: %d"),
				ANSI_TO_TCHAR(__FUNCTION__),
				GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
				Iterations);
#endif // PMC_DEBUG_VERBOSE
			SetMovementMode(EMovementMode::MOVE_Falling);
			StartNewPhysics(deltaTime, Iterations);
			return;
		}
	}

	// Not sure if this is needed
	Super::PhysCustom(deltaTime, Iterations);
}

bool UPMCharacterMovementComponent::StartPhysCustomMovement(const TSharedPtr<FPhysCustomMovement>& inPhysCustomMovement)
{
	if (PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive())
	{
#if PMC_DEBUG_VERBOSE
		UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: %s is still active. If you want to start %s, wait till that movement is done or manually stop it."),
			ANSI_TO_TCHAR(__FUNCTION__),
			GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
			*PhysCustomMovement->MovementName.ToString(),
			*inPhysCustomMovement->MovementName.ToString());
#endif // PMC_DEBUG_VERBOSE
		return false;
	}

	PhysCustomMovement = inPhysCustomMovement;

	if (inPhysCustomMovement.IsValid())
	{
#if PMC_DEBUG_VERBOSE
		UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: Starting Movement: %s"),
			ANSI_TO_TCHAR(__FUNCTION__), GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
			*PhysCustomMovement->MovementName.ToString());
#endif // PMC_DEBUG_VERBOSE
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
#if PMC_DEBUG_VERBOSE
	UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: Ending Movement: %s"),
		ANSI_TO_TCHAR(__FUNCTION__),
		GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
		PhysCustomMovement.IsValid() ? *PhysCustomMovement->MovementName.ToString() : TEXT("Invalid"));
#endif // PMC_DEBUG_VERBOSE
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
#if PMC_DEBUG_VERBOSE
			UE_LOG(LogPhysCustomMovement, Display, TEXT("%s: %s: Movement Mode Changed from %s to %s during Movement %s. Requesting to Stop the Phys Custom Movement..."),
				ANSI_TO_TCHAR(__FUNCTION__),
				GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
				*UEnum::GetValueAsString(PreviousMovementMode),
				*UEnum::GetValueAsString(MovementMode),
				PhysCustomMovement.IsValid() ? *PhysCustomMovement->MovementName.ToString() : TEXT("Invalid"));
#endif // PMC_DEBUG_VERBOSE

			StopPhysCustomMovement();
		}
	}
}

void UPMCharacterMovementComponent::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags,
	const FVector& NewAccel)
{
	// Apply unpredicted data to the current custom movement to keep server and client simulating with the same values.
	// TODO: how to dynamically set those??
	if (CustomMovementMode == GetPhysCustomMovementModeFlag() && PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive())
	{
		if (PhysCustomMovement->GetTypeStruct() == FPhysCustomMovement_NonDeterministicMove::StaticStruct())
		{
			if (FPhysCustomMovement_NonDeterministicMove* movement = static_cast<FPhysCustomMovement_NonDeterministicMove*>(PhysCustomMovement.Get()))
			{
				//Unpacks the Network Move Data for the CMC to use on the server or during replay. Copies Network Move Data into CMC.
				if (const FPMCharacterNetworkMoveData* moveData = static_cast<FPMCharacterNetworkMoveData*>(GetCurrentNetworkMoveData()))
				{
					/*UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: (WaitTime = %.2f, ElapsedTime = %.2f, MovementDirectionSign = %.2f)"),
						ANSI_TO_TCHAR(__FUNCTION__),
						GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
						moveData->WaitTime,
						moveData->ElapsedTime,
						moveData->MovementDirectionSign);*/

					movement->TimeToWait = moveData->WaitTime;
					movement->MovementDirectionSign = moveData->MovementDirectionSign;
					movement->ElapsedTime = moveData->ElapsedTime;
				}
			}
		}
	}

	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAccel);
}

void UPMCharacterMovementComponent::OnClientCorrectionReceived(class FNetworkPredictionData_Client_Character& ClientData, float TimeStamp, FVector NewLocation, FVector NewVelocity, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode)
{
	if (PhysCustomMovement.IsValid() && PhysCustomMovement->IsActive())
	{
		const FVector clientLocAtCorrectedMove = ClientData.LastAckedMove.IsValid() ? ClientData.LastAckedMove->SavedLocation : UpdatedComponent->GetComponentLocation();
		const FVector locDiff = clientLocAtCorrectedMove - NewLocation;
		const float amountDiff = locDiff.Size();

#if PMC_DEBUG_VERBOSE
		UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: Received %.2f of location error during execution of movement %s."),
			ANSI_TO_TCHAR(__FUNCTION__),
			GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
			amountDiff,
			PhysCustomMovement.IsValid() ? *PhysCustomMovement->MovementName.ToString() : TEXT(""));
#endif // PMC_DEBUG_VERBOSE

		if (!locDiff.IsNearlyZero(PhysCustomMovement->LocationErrorToleranceThreshold))
		{
			// NewLocation: where the server corrected us to
			// clientLocAtCorrectedMove: location where client thought they were
			// locDiff is zero: we already corrected, it basically means "no-op"
			PhysCustomMovement->HoldMovementUpdates();

#if PMC_DEBUG_VERBOSE
			UE_LOG(LogPhysCustomMovement, Warning, TEXT("%s: %s: Location error (%.2f) exceeded limit (%.2f) and subsequential moves for movement %s will be skipped to prevent more degradation and hopefully get in sync with the server."),
				ANSI_TO_TCHAR(__FUNCTION__),
				GET_ACTOR_LOCAL_ROLE_FSTRING(GetCharacterOwner()),
				amountDiff,
				PhysCustomMovement->LocationErrorToleranceThreshold,
				PhysCustomMovement.IsValid() ? *PhysCustomMovement->MovementName.ToString() : TEXT(""));
#endif // PMC_DEBUG_VERBOSE
		}
		else
		{
			PhysCustomMovement->ResetUpdateSkipping();
		}
	}

	Super::OnClientCorrectionReceived(ClientData, TimeStamp, NewLocation, NewVelocity, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
}

void FPMSavedMove::Clear()
{
	Super::Clear();

	bSavedWantsPhysCustomMovement = false;

	// TODO: should clear non predicted data here?
	waitTime = 99.f;
	movementDirectionSign = 1.f;
	elapsedTime = 0.f;
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

	// TODO: should check for non predicted data? how to do it if they were dynamically added?
	if (UPMCharacterMovementComponent* characterMovement = Cast<UPMCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		if (auto movement = static_cast<FPhysCustomMovement_NonDeterministicMove*>(characterMovement->PhysCustomMovement.Get()))
		{
			// TODO: how to dynamically check these? 
			return FMath::IsNearlyEqual(movement->TimeToWait, ((FPMSavedMove*)NewMove.Get())->waitTime, KINDA_SMALL_NUMBER)
				&& FMath::IsNearlyEqual(movement->MovementDirectionSign, ((FPMSavedMove*)NewMove.Get())->movementDirectionSign, KINDA_SMALL_NUMBER)
				&& FMath::IsNearlyEqual(movement->ElapsedTime, ((FPMSavedMove*)NewMove.Get())->elapsedTime, KINDA_SMALL_NUMBER);
		}
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FPMSavedMove::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	if (UPMCharacterMovementComponent* characterMovement = Cast<UPMCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		// Copy values into the saved move
		bSavedWantsPhysCustomMovement = characterMovement->bWantsPhysCustomMovement;

		// TODO: should set non deterministic data here as well? how to do it if they were dynamically added
		if (auto movement = static_cast<FPhysCustomMovement_NonDeterministicMove*>(characterMovement->PhysCustomMovement.Get()))
		{
			// TODO: how to dynamically set these??
			waitTime = movement->TimeToWait;
			movementDirectionSign = movement->MovementDirectionSign;
			elapsedTime = movement->ElapsedTime;
		}
	}
}

void FPMSavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	// This is used to copy state from the saved move to the character movement component. 
	// This is ONLY used for predictive corrections, the actual data must be sent through RPC.

 	if (UPMCharacterMovementComponent* characterMovement = Cast<UPMCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		// Copy values out of the saved move
		characterMovement->bWantsPhysCustomMovement = bSavedWantsPhysCustomMovement;

		// TODO: should apply non predicted values here? how to do it if they were dynamically added
		if (auto movement = static_cast<FPhysCustomMovement_NonDeterministicMove*>(characterMovement->PhysCustomMovement.Get()))
		{
			// TODO: how to dynamically set these??
			movement->TimeToWait = waitTime;
			movement->MovementDirectionSign = movementDirectionSign;
			movement->ElapsedTime = elapsedTime;
		}
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
// 
FPMCharacterNetworkMoveDataContainer::FPMCharacterNetworkMoveDataContainer() : Super()
{
	NewMoveData = &CustomDefaultMoveData[0];
	PendingMoveData = &CustomDefaultMoveData[1];
	OldMoveData = &CustomDefaultMoveData[2];
}

//Sends the Movement Data 
bool FPMCharacterNetworkMoveData::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	Super::Serialize(CharacterMovement, Ar, PackageMap, MoveType);

	// TODO: how to dynamically serialize these??
	SerializeOptionalValue<float>(Ar.IsSaving(), Ar, WaitTime, 0.f);
	SerializeOptionalValue<float>(Ar.IsSaving(), Ar, MovementDirectionSign, 1.f);
	SerializeOptionalValue<float>(Ar.IsSaving(), Ar, ElapsedTime, 0.f);

	return !Ar.IsError();
}

void FPMCharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(ClientMove, MoveType);

	const FPMSavedMove& savedMove = static_cast<const FPMSavedMove&>(ClientMove);

	// TODO: how to dynamically set these??
	WaitTime = savedMove.waitTime;
	MovementDirectionSign = savedMove.movementDirectionSign;
	ElapsedTime = savedMove.elapsedTime;
}
