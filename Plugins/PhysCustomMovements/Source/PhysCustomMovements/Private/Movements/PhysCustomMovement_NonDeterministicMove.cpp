// Copyright 2022 Andrey Bicalho.

#include "Movements/PhysCustomMovement_NonDeterministicMove.h"
#include "Components/PMCharacterMovementComponent.h"
#include "GameFramework/Character.h"

DEFINE_LOG_CATEGORY(LogPhysNonDeterministicMove);

FPhysCustomMovement_NonDeterministicMove::FPhysCustomMovement_NonDeterministicMove() 
    : FPhysCustomMovement() 
{
	TimeToWait = -1.f;
    MovementDirectionSign = 1.f;
    ElapsedTime = 0.f;
}

void FPhysCustomMovement_NonDeterministicMove::UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity)
{
    SCOPED_NAMED_EVENT(FPhysCustomMovement_UpdateMovement_NonDeterministicMove, FColor::Yellow);

    CurrentTime += deltaTime;

    ElapsedTime += deltaTime;

    if (TimeToWait > 0.f && ElapsedTime >= TimeToWait)
    {
        TimeToWait = -1.f;
        MovementDirectionSign *= -1.f;
        ElapsedTime = 0.f;
        OnReachedTime.Broadcast();
    }

    outVelocity = CharacterMovementComponent->GetCurrentAcceleration() * MovementDirectionSign * GetMaxSpeed();
    outVelocity = outVelocity.GetClampedToMaxSize(GetMaxSpeed());
};

UScriptStruct* FPhysCustomMovement_NonDeterministicMove::GetScriptStruct() const
{
    return FPhysCustomMovement_NonDeterministicMove::StaticStruct();
}

FPhysCustomMovement* FPhysCustomMovement_NonDeterministicMove::Clone() const
{
	FPhysCustomMovement_NonDeterministicMove* copyPtr = new FPhysCustomMovement_NonDeterministicMove(*this);
	return copyPtr;
}

void FPhysCustomMovement_NonDeterministicMove::Clear()
{
    FPhysCustomMovement::Clear();

	TimeToWait = -1.f;
	MovementDirectionSign = 1.f;
	ElapsedTime = 0.f;
}

//void FPhysCustomMovement_NonDeterministicMove::SetupBaseFromCustomMovement(const FPhysCustomMovement& physCustomMovement)
//{
//    FPhysCustomMovement::SetupBaseFromCustomMovement(physCustomMovement);
//
//    if (const FPhysCustomMovement_NonDeterministicMove* physCustomMove_NonDeter = static_cast<const FPhysCustomMovement_NonDeterministicMove*>(&physCustomMovement))
//    {
//		TimeToWait = physCustomMove_NonDeter->TimeToWait;
//		MovementDirectionSign = physCustomMove_NonDeter->MovementDirectionSign;
//		ElapsedTime = physCustomMove_NonDeter->ElapsedTime;
//
//        //OnReachedTime = physCustomMove_NonDeter->OnReachedTime; // does it work for delegates?
//    }
//}

//bool FPhysCustomMovement_NonDeterministicMove::NetSerialize(FArchive& ar, UPackageMap* map, bool& bOutSuccess)
//{
//    if (!FPhysCustomMovement::NetSerialize(ar, map, bOutSuccess))
//    {
//        return false;
//    }
//
//	ar << TimeToWait;
//	ar << MovementDirectionSign;
//	ar << ElapsedTime;
//    //ar << OnReachedTime; // TODO: does it support delegates serialization?
//
//    bOutSuccess = true;
//    return true;
//}
