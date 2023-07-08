// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Components/PMCharacterMovementComponent.h"
#include "GSCharacterMovementComponent.generated.h"

/**
 *
 */
UCLASS()
class GASSHOOTER_API UGSCharacterMovementComponent : public UPMCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UGSCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);
};
