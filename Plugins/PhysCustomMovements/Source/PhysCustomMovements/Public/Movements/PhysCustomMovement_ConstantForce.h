// Copyright 2022 Andrey Bicalho.

#pragma once

#include "Movements/PhysCustomMovement.h"
#include "PhysCustomMovement_ConstantForce.generated.h"

/**
* An example of a Constant Force.
* 
*/
USTRUCT()
struct PHYSCUSTOMMOVEMENTS_API FPhysCustomMovement_ConstantForce : public FPhysCustomMovement
{
	GENERATED_USTRUCT_BODY()

	FVector Force = FVector::ZeroVector;

	FPhysCustomMovement_ConstantForce();

	virtual ~FPhysCustomMovement_ConstantForce() {}

	virtual void UpdateMovement(const float deltaTime, const FVector& oldVelocity, FVector& outVelocity) override;

	/*virtual bool NetSerialize(FArchive& ar, UPackageMap* map, bool& bOutSuccess) override;
	virtual UScriptStruct* GetScriptStruct() const override;*/
};

//template<>
//struct TStructOpsTypeTraits<FPhysCustomMovement_ConstantForce> : public TStructOpsTypeTraitsBase2<FPhysCustomMovement_ConstantForce>
//{
//	enum
//	{
//		WithNetSerializer = true,
//		WithCopy = true
//	};
//};
