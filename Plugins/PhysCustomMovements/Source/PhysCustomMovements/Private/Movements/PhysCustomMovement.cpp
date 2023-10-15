// Copyright 2022 Andrey Bicalho.

#include "Movements/PhysCustomMovement.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/NetSerialization.h"

bool FPhysCustomMovement::BeginMovement(ACharacter* inCharacter, UCharacterMovementComponent* inCharacterMovementComponent, const uint8 inCustomModeFlag)
{
	if (inCharacterMovementComponent)
	{
		CharacterMovementComponent = inCharacterMovementComponent;
		CustomModeFlag = inCustomModeFlag;
		CurrentTime = 0.f;
		bIsActive = true;
		CharacterMovementComponent->MaxCustomMovementSpeed = MaxSpeed;
		CharacterMovementComponent->SetMovementMode(MOVE_Custom, CustomModeFlag);

		return true;
	}

	return false;
}

void FPhysCustomMovement::EndMovement()
{
	bIsActive = false;
	CharacterMovementComponent = nullptr;

	OnCustomMovementEnd.Broadcast();
}

void FPhysCustomMovement::SetupBaseFromCustomMovement(const FPhysCustomMovement& physCustomMovement)
{
	// NOTE: we should setup all properties that are serialized
	CharacterMovementComponent = physCustomMovement.CharacterMovementComponent;
	OnCustomMovementEnd = physCustomMovement.OnCustomMovementEnd;

	MovementName = physCustomMovement.MovementName;
	CurrentVelocity = physCustomMovement.CurrentVelocity;
	CurrentTime = physCustomMovement.CurrentTime;
	bIsActive = physCustomMovement.bIsActive;
	MaxSpeed = physCustomMovement.MaxSpeed;
	MaxAcceleration = physCustomMovement.MaxAcceleration;
	MaxBrakingDeceleration = physCustomMovement.MaxBrakingDeceleration;
	CustomModeFlag = physCustomMovement.CustomModeFlag;
	FallbackMovementMode = physCustomMovement.FallbackMovementMode;
	bSkipMovementUpdates = physCustomMovement.bSkipMovementUpdates;
}

void FPhysCustomMovement::Clear()
{
	bIsActive = false;
	CharacterMovementComponent = nullptr;

	ClearPredictedProperties();
}

bool FPhysCustomMovement::SkipThisUpdate(const float deltaTime)
{
	if (bSkipMovementUpdates)
	{
		UpdateSkipCount++;
		TimeSkippingMovement += deltaTime;

		if (UpdateSkipCount > MaxNumUpdateSkips) // TODO: find a better way of setting this threshold, maybe based on the time we get updates from the server
		{
			ResetUpdateSkipping();
		}
	}

	return bSkipMovementUpdates;
}

void FPhysCustomMovement::HoldMovementUpdates()
{
	bSkipMovementUpdates = true;
}

void FPhysCustomMovement::ResetUpdateSkipping()
{
	TimeSkippingMovement = 0.f;
	UpdateSkipCount = 0;
	bSkipMovementUpdates = false;
}

void FPhysCustomMovement::BindByteProperty(const FName name)
{
	FPhysPredictedProperty<uint8> physProperty;
	physProperty.Name = name;
	PhysBytes.Add(physProperty);
}

void FPhysCustomMovement::BindBoolProperty(const FName name)
{
	FPhysPredictedProperty<bool> physProperty;
	physProperty.Name = name;
	PhysBooleans.Add(physProperty);
}

void FPhysCustomMovement::BindIntegerProperty(const FName name)
{
	FPhysPredictedProperty<int32> physProperty;
	physProperty.Name = name;
	PhysIntegers.Add(physProperty);
}

void FPhysCustomMovement::BindFloatProperty(const FName name)
{
	FPhysPredictedProperty<float> physProperty;
	physProperty.Name = name;
	PhysFloats.Add(physProperty);
}

void FPhysCustomMovement::BindDoubleProperty(const FName name)
{
	FPhysPredictedProperty<double> physProperty;
	physProperty.Name = name;
	PhysDoubles.Add(physProperty);
}

void FPhysCustomMovement::BindVectorProperty(const FName name)
{
	FPhysPredictedProperty<FVector> physProperty;
	physProperty.Name = name;
	PhysVectors.Add(physProperty);
}

void FPhysCustomMovement::BindVector2DProperty(const FName name)
{
	FPhysPredictedProperty<FVector2D> physProperty;
	physProperty.Name = name;
	PhysVectors2D.Add(physProperty);
}

void FPhysCustomMovement::BindVector4Property(const FName name)
{
	FPhysPredictedProperty<FVector4> physProperty;
	physProperty.Name = name;
	PhysVectors4.Add(physProperty);
}

void FPhysCustomMovement::BindRotatorProperty(const FName name)
{
	FPhysPredictedProperty<FRotator> physProperty;
	physProperty.Name = name;
	PhysRotators.Add(physProperty);
}

void FPhysCustomMovement::BindQuatProperty(const FName name)
{
	FPhysPredictedProperty<FQuat> physProperty;
	physProperty.Name = name;
	PhysQuats.Add(physProperty);
}

void FPhysCustomMovement::BindGameplayTagProperty(const FName name)
{
	FPhysPredictedProperty<FGameplayTag> physProperty;
	physProperty.Name = name;
	PhysGameplayTags.Add(physProperty);
}

// used in the SavedMove::SetMoveFor
void FPhysCustomMovement::SetupPredictedProperties(FPhysCustomMovement& outPhysCustomMovement)
{
	// use unreal reflection system to update the member variables with the value in the predicted properties arrays and also add it to the output movement
	for (FPhysPredictedProperty<uint8>& bytePredProp : PhysBytes)
	{
		if (const FByteProperty* bytePropPtr = CastField<FByteProperty>(GetScriptStruct()->FindPropertyByName(bytePredProp.Name)))
		{
			const void* valuePtr = bytePropPtr->ContainerPtrToValuePtr<void>(this);
			const uint8 value = bytePropPtr->GetPropertyValue(valuePtr);
			bytePredProp.Value = value;
			outPhysCustomMovement.PhysBytes.Add(bytePredProp);
		}
	}

	for (FPhysPredictedProperty<bool>& boolPredProp : PhysBooleans)
	{
		if (const FBoolProperty* boolPropPtr = CastField<FBoolProperty>(GetScriptStruct()->FindPropertyByName(boolPredProp.Name)))
		{
			const void* valuePtr = boolPropPtr->ContainerPtrToValuePtr<void>(this);
			const bool value = boolPropPtr->GetPropertyValue(valuePtr);
			boolPredProp.Value = value;
			outPhysCustomMovement.PhysBooleans.Add(boolPredProp);
		}
	}

	for (FPhysPredictedProperty<int32>& integerPredProp : PhysIntegers)
	{
		if (const FIntProperty* integerPredPropPtr = CastField<FIntProperty>(GetScriptStruct()->FindPropertyByName(integerPredProp.Name)))
		{
			const void* valuePtr = integerPredPropPtr->ContainerPtrToValuePtr<void>(this);
			const int32 value = integerPredPropPtr->GetPropertyValue(valuePtr);
			integerPredProp.Value = value;
			outPhysCustomMovement.PhysIntegers.Add(integerPredProp);
		}
	}

	for (FPhysPredictedProperty<float>& floatPredProp : PhysFloats)
	{
		if (const FFloatProperty* floatPropPtr = CastField<FFloatProperty>(GetScriptStruct()->FindPropertyByName(floatPredProp.Name)))
		{
			const void* valuePtr = floatPropPtr->ContainerPtrToValuePtr<void>(this);
			const float value = floatPropPtr->GetPropertyValue(valuePtr);
			floatPredProp.Value = value;
			outPhysCustomMovement.PhysFloats.Add(floatPredProp);
			//outPhysCustomMovement.PhysFloats.Add(FPhysPredictedProperty(floatPredProp.Name, value));
		}
	}

	for (FPhysPredictedProperty<double>& doublePredProp : PhysDoubles)
	{
		if (const FDoubleProperty* doublePropPtr = CastField<FDoubleProperty>(GetScriptStruct()->FindPropertyByName(doublePredProp.Name)))
		{
			const void* valuePtr = doublePropPtr->ContainerPtrToValuePtr<void>(this);
			const double value = doublePropPtr->GetPropertyValue(valuePtr);
			doublePredProp.Value = value;
			outPhysCustomMovement.PhysDoubles.Add(doublePredProp);
		}
	}

	for (FPhysPredictedProperty<FVector>& vectorPredProp : PhysVectors)
	{
		if (const FStructProperty* vectorPredPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(vectorPredProp.Name)))
		{
			vectorPredProp.Value = *vectorPredPropPtr->ContainerPtrToValuePtr<FVector>(this);
			outPhysCustomMovement.PhysVectors.Add(vectorPredProp);
		}
	}

	for (FPhysPredictedProperty<FVector2D>& vec2DPredProp : PhysVectors2D)
	{
		if (const FStructProperty* vec2DPredPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(vec2DPredProp.Name)))
		{
			vec2DPredProp.Value = *vec2DPredPropPtr->ContainerPtrToValuePtr<FVector2D>(this);
			outPhysCustomMovement.PhysVectors2D.Add(vec2DPredProp);
		}
	}

	for (FPhysPredictedProperty<FVector4>& vec4PredProp : PhysVectors4)
	{
		if (const FStructProperty* vec4PredPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(vec4PredProp.Name)))
		{
			vec4PredProp.Value = *vec4PredPropPtr->ContainerPtrToValuePtr<FVector4>(this);
			outPhysCustomMovement.PhysVectors4.Add(vec4PredProp);
		}
	}

	for (FPhysPredictedProperty<FRotator>& rotatorPredProp : PhysRotators)
	{
		if (const FStructProperty* rotatorPredPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(rotatorPredProp.Name)))
		{
			rotatorPredProp.Value = *rotatorPredPropPtr->ContainerPtrToValuePtr<FRotator>(this);
			outPhysCustomMovement.PhysRotators.Add(rotatorPredProp);
		}
	}

	for (FPhysPredictedProperty<FQuat>& quatPredProp : PhysQuats)
	{
		if (const FStructProperty* quatPredPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(quatPredProp.Name)))
		{
			quatPredProp.Value = *quatPredPropPtr->ContainerPtrToValuePtr<FQuat>(this);
			outPhysCustomMovement.PhysQuats.Add(quatPredProp);
		}
	}

	for (FPhysPredictedProperty<FGameplayTag>& gameplayTagPredProp : PhysGameplayTags)
	{
		if (const FStructProperty* gameplayTagPredPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(gameplayTagPredProp.Name)))
		{
			gameplayTagPredProp.Value = *gameplayTagPredPropPtr->ContainerPtrToValuePtr<FGameplayTag>(this);
			outPhysCustomMovement.PhysGameplayTags.Add(gameplayTagPredProp);
		}
	}
}

// used in the UPMCharacterMovementComponent::MoveAutonomous
void FPhysCustomMovement::ReflectFromOtherPredPropsByMyKeys(const FPhysCustomMovement& otherPhysCustomMovement)
{
	for (const FPhysPredictedProperty<uint8>& bytePredProp : PhysBytes)
	{
		if (const FByteProperty* bytePropPtr = CastField<FByteProperty>(GetScriptStruct()->FindPropertyByName(bytePredProp.Name)))
		{
			void* valuePtr = bytePropPtr->ContainerPtrToValuePtr<void>(this);
			bytePropPtr->SetPropertyValue(valuePtr, otherPhysCustomMovement.PhysBytes.FindByKey(bytePredProp)->Value);
		}
	}

	for (const FPhysPredictedProperty<bool>& boolPredProp : PhysBooleans)
	{
		if (const FBoolProperty* boolPropPtr = CastField<FBoolProperty>(GetScriptStruct()->FindPropertyByName(boolPredProp.Name)))
		{
			void* valuePtr = boolPropPtr->ContainerPtrToValuePtr<void>(this);
			boolPropPtr->SetPropertyValue(valuePtr, otherPhysCustomMovement.PhysBooleans.FindByKey(boolPredProp)->Value);
		}
	}

	for (const FPhysPredictedProperty<int32>& integerPredProp : PhysIntegers)
	{
		if (const FIntProperty* integerPropPtr = CastField<FIntProperty>(GetScriptStruct()->FindPropertyByName(integerPredProp.Name)))
		{
			void* valuePtr = integerPropPtr->ContainerPtrToValuePtr<void>(this);
			integerPropPtr->SetPropertyValue(valuePtr, otherPhysCustomMovement.PhysIntegers.FindByKey(integerPredProp)->Value);
		}
	}

	for (FPhysPredictedProperty<float>& floatPredProp : PhysFloats)
	{
		if (FFloatProperty* floatPropPtr = CastField<FFloatProperty>(GetScriptStruct()->FindPropertyByName(floatPredProp.Name)))
		{
			void* valuePtr = floatPropPtr->ContainerPtrToValuePtr<void>(this);
			if (const auto prop = otherPhysCustomMovement.PhysFloats.FindByKey(floatPredProp))
			{
				floatPropPtr->SetPropertyValue(valuePtr, prop->Value);
			}
		}
	}

	for (const FPhysPredictedProperty<double>& doublePredProp : PhysDoubles)
	{
		if (const FDoubleProperty* doublePropPtr = CastField<FDoubleProperty>(GetScriptStruct()->FindPropertyByName(doublePredProp.Name)))
		{
			void* valuePtr = doublePropPtr->ContainerPtrToValuePtr<void>(this);
			doublePropPtr->SetPropertyValue(valuePtr, otherPhysCustomMovement.PhysDoubles.FindByKey(doublePredProp)->Value);
		}
	}

	for (const FPhysPredictedProperty<FVector>& vectorPredProp : PhysVectors)
	{
		if (const FStructProperty* vectorPredPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(vectorPredProp.Name)))
		{
			vectorPredPropPtr->SetValue_InContainer(this, &otherPhysCustomMovement.PhysVectors.FindByKey(vectorPredProp)->Value);
		}
	}

	for (const FPhysPredictedProperty<FVector2D>& vector2DPredProp : PhysVectors2D)
	{
		if (const FStructProperty* vector2DPredPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(vector2DPredProp.Name)))
		{
			vector2DPredPropPtr->SetValue_InContainer(this, &otherPhysCustomMovement.PhysVectors2D.FindByKey(vector2DPredProp)->Value);
		}
	}

	for (const FPhysPredictedProperty<FVector4>& vec4PredProp : PhysVectors4)
	{
		if (const FStructProperty* vec4PredPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(vec4PredProp.Name)))
		{
			vec4PredPropPtr->SetValue_InContainer(this, &otherPhysCustomMovement.PhysVectors4.FindByKey(vec4PredProp)->Value);
		}
	}

	for (const FPhysPredictedProperty<FRotator>& rotatorPredProp : PhysRotators)
	{
		if (const FStructProperty* rotatorPredPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(rotatorPredProp.Name)))
		{
			rotatorPredPropPtr->SetValue_InContainer(this, &otherPhysCustomMovement.PhysRotators.FindByKey(rotatorPredProp)->Value);
		}
	}

	for (const FPhysPredictedProperty<FQuat>& quatPredProp : PhysQuats)
	{
		if (const FStructProperty* quatPredPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(quatPredProp.Name)))
		{
			quatPredPropPtr->SetValue_InContainer(this, &otherPhysCustomMovement.PhysQuats.FindByKey(quatPredProp)->Value);
		}
	}

	for (const FPhysPredictedProperty<FGameplayTag>& gameplayTagPredProp : PhysGameplayTags)
	{
		if (const FStructProperty* gameplayTagPredPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(gameplayTagPredProp.Name)))
		{
			gameplayTagPredPropPtr->SetValue_InContainer(this, &otherPhysCustomMovement.PhysGameplayTags.FindByKey(gameplayTagPredProp)->Value);
		}
	}
}

// used in the SavedMove::PrepMoveFor
void FPhysCustomMovement::ReflectFromOtherPredictedProperties(const FPhysCustomMovement& otherPhysCustomMovement)
{
	// use unreal reflection system to copy all properties names in the array to member variables
	for (const FPhysPredictedProperty<uint8>& byteProp : otherPhysCustomMovement.PhysBytes)
	{
		if (FByteProperty* bytePropertyPtr = CastField<FByteProperty>(GetScriptStruct()->FindPropertyByName(byteProp.Name)))
		{
			void* valuePtr = bytePropertyPtr->ContainerPtrToValuePtr<void>(this);
			bytePropertyPtr->SetPropertyValue(valuePtr, byteProp.Value);
		}
	}

	for (const FPhysPredictedProperty<bool>& boolProp : otherPhysCustomMovement.PhysBooleans)
	{
		if (FBoolProperty* boolPropertyPtr = CastField<FBoolProperty>(GetScriptStruct()->FindPropertyByName(boolProp.Name)))
		{
			void* valuePtr = boolPropertyPtr->ContainerPtrToValuePtr<void>(this);
			boolPropertyPtr->SetPropertyValue(valuePtr, boolProp.Value);
		}
	}

	for (const FPhysPredictedProperty<int32>& integerProp : otherPhysCustomMovement.PhysIntegers)
	{
		if (FIntProperty* integerPropPtr = CastField<FIntProperty>(GetScriptStruct()->FindPropertyByName(integerProp.Name)))
		{
			void* valuePtr = integerPropPtr->ContainerPtrToValuePtr<void>(this);
			integerPropPtr->SetPropertyValue(valuePtr, integerProp.Value);
		}
	}

	for (const FPhysPredictedProperty<float>& floatProp : otherPhysCustomMovement.PhysFloats)
	{
		if (FFloatProperty* floatPropPtr = CastField<FFloatProperty>(GetScriptStruct()->FindPropertyByName(floatProp.Name)))
		{
			void* valuePtr = floatPropPtr->ContainerPtrToValuePtr<void>(this);
			floatPropPtr->SetPropertyValue(valuePtr, floatProp.Value);
		}
	}

	for (const FPhysPredictedProperty<double>& doubleProp : otherPhysCustomMovement.PhysDoubles)
	{
		if (FDoubleProperty* doublePropPtr = CastField<FDoubleProperty>(GetScriptStruct()->FindPropertyByName(doubleProp.Name)))
		{
			void* valuePtr = doublePropPtr->ContainerPtrToValuePtr<void>(this);
			doublePropPtr->SetPropertyValue(valuePtr, doubleProp.Value);
		}
	}

	for (const FPhysPredictedProperty<FVector>& vectorProp : otherPhysCustomMovement.PhysVectors)
	{
		if (FStructProperty* vectorPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(vectorProp.Name)))
		{
			vectorPropPtr->SetValue_InContainer(this, &vectorProp.Value);
		}
	}

	for (const FPhysPredictedProperty<FVector2D>& vector2DProp : otherPhysCustomMovement.PhysVectors2D)
	{
		if (FStructProperty* vector2DPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(vector2DProp.Name)))
		{
			vector2DPropPtr->SetValue_InContainer(this, &vector2DProp.Value);
		}
	}

	for (const FPhysPredictedProperty<FVector4>& vector4Prop : otherPhysCustomMovement.PhysVectors4)
	{
		if (FStructProperty* vector4PropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(vector4Prop.Name)))
		{
			vector4PropPtr->SetValue_InContainer(this, &vector4Prop.Value);
		}
	}

	for (const FPhysPredictedProperty<FRotator>& rotatorProp : otherPhysCustomMovement.PhysRotators)
	{
		if (FStructProperty* rotatorPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(rotatorProp.Name)))
		{
			rotatorPropPtr->SetValue_InContainer(this, &rotatorProp.Value);
		}
	}

	for (const FPhysPredictedProperty<FQuat>& quatProp : otherPhysCustomMovement.PhysQuats)
	{
		if (FStructProperty* quatPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(quatProp.Name)))
		{
			quatPropPtr->SetValue_InContainer(this, &quatProp.Value);
		}
	}

	for (const FPhysPredictedProperty<FGameplayTag>& gameplayTagProp : otherPhysCustomMovement.PhysGameplayTags)
	{
		if (FStructProperty* gameplayTagPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(gameplayTagProp.Name)))
		{
			gameplayTagPropPtr->SetValue_InContainer(this, &gameplayTagProp.Value);
		}
	}
}

bool FPhysCustomMovement::HasBoundPredictedProperties() const
{
	return !PhysBytes.IsEmpty()
		|| !PhysBooleans.IsEmpty()
		|| !PhysIntegers.IsEmpty()
		|| !PhysFloats.IsEmpty()
		|| !PhysDoubles.IsEmpty()
		|| !PhysVectors.IsEmpty()
		|| !PhysVectors2D.IsEmpty()
		|| !PhysVectors4.IsEmpty()
		|| !PhysRotators.IsEmpty()
		|| !PhysQuats.IsEmpty()
		|| !PhysGameplayTags.IsEmpty();
}

void FPhysCustomMovement::ClearPredictedProperties()
{
	if (HasBoundPredictedProperties())
	{
		for (const FPhysPredictedProperty<uint8>& byteProp : PhysBytes)
		{
			if (FByteProperty* bytePropertyPtr = CastField<FByteProperty>(GetScriptStruct()->FindPropertyByName(byteProp.Name)))
			{
				void* valuePtr = bytePropertyPtr->ContainerPtrToValuePtr<void>(this);
				bytePropertyPtr->SetPropertyValue(valuePtr, 0);
			}
		}

		for (const FPhysPredictedProperty<bool>& boolProp : PhysBooleans)
		{
			if (FBoolProperty* boolPropertyPtr = CastField<FBoolProperty>(GetScriptStruct()->FindPropertyByName(boolProp.Name)))
			{
				void* valuePtr = boolPropertyPtr->ContainerPtrToValuePtr<void>(this);
				boolPropertyPtr->SetPropertyValue(valuePtr, false);
			}
		}

		for (const FPhysPredictedProperty<int32>& integerProp : PhysIntegers)
		{
			if (FIntProperty* integerPropPtr = CastField<FIntProperty>(GetScriptStruct()->FindPropertyByName(integerProp.Name)))
			{
				void* valuePtr = integerPropPtr->ContainerPtrToValuePtr<void>(this);
				integerPropPtr->SetPropertyValue(valuePtr, 0);
			}
		}

		for (const FPhysPredictedProperty<float>& floatProp : PhysFloats)
		{
			if (FFloatProperty* floatPropPtr = CastField<FFloatProperty>(GetScriptStruct()->FindPropertyByName(floatProp.Name)))
			{
				void* valuePtr = floatPropPtr->ContainerPtrToValuePtr<void>(this);
				floatPropPtr->SetPropertyValue(valuePtr, 0.f);
			}
		}

		for (const FPhysPredictedProperty<double>& doubleProp : PhysDoubles)
		{
			if (FDoubleProperty* doublePropPtr = CastField<FDoubleProperty>(GetScriptStruct()->FindPropertyByName(doubleProp.Name)))
			{
				void* valuePtr = doublePropPtr->ContainerPtrToValuePtr<void>(this);
				doublePropPtr->SetPropertyValue(valuePtr, 0.f);
			}
		}

		for (const FPhysPredictedProperty<FVector>& vectorProp : PhysVectors)
		{
			if (FStructProperty* vectorPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(vectorProp.Name)))
			{
				vectorPropPtr->SetValue_InContainer(this, &FVector::ZeroVector);
			}
		}

		for (const FPhysPredictedProperty<FVector2D>& vector2DProp : PhysVectors2D)
		{
			if (FStructProperty* vector2DPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(vector2DProp.Name)))
			{
				vector2DPropPtr->SetValue_InContainer(this, &FVector2D::ZeroVector);
			}
		}

		for (const FPhysPredictedProperty<FVector4>& vector4Prop : PhysVectors4)
		{
			if (FStructProperty* vector4PropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(vector4Prop.Name)))
			{
				FVector4 zeroVector4 = FVector4(0);
				vector4PropPtr->SetValue_InContainer(this, &zeroVector4);
			}
		}

		for (const FPhysPredictedProperty<FRotator>& rotatorProp : PhysRotators)
		{
			if (FStructProperty* rotatorPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(rotatorProp.Name)))
			{
				rotatorPropPtr->SetValue_InContainer(this, &FRotator::ZeroRotator);
			}
		}

		for (const FPhysPredictedProperty<FQuat>& quatProp : PhysQuats)
		{
			if (FStructProperty* quatPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(quatProp.Name)))
			{
				quatPropPtr->SetValue_InContainer(this, &FQuat::Identity);
			}
		}

		for (const FPhysPredictedProperty<FGameplayTag>& gameplayTagProp : PhysGameplayTags)
		{
			if (FStructProperty* gameplayTagPropPtr = CastField<FStructProperty>(GetScriptStruct()->FindPropertyByName(gameplayTagProp.Name)))
			{
				gameplayTagPropPtr->SetValue_InContainer(this, &FGameplayTag::EmptyTag);
			}
		}
	}

	PhysBytes.Empty();
	PhysBooleans.Empty();
	PhysIntegers.Empty();
	PhysFloats.Empty();
	PhysDoubles.Empty();
	PhysVectors.Empty();
	PhysVectors2D.Empty();
	PhysVectors4.Empty();
	PhysRotators.Empty();
	PhysQuats.Empty();
	PhysGameplayTags.Empty();
}

FPhysCustomMovement* FPhysCustomMovement::Clone() const
{
	// If child classes don't override this, savedmoves will not work
	checkf(false, TEXT("FPhysCustomMovement::Clone() being called erroneously. This should always be overridden in child classes!"));

	/*
	* boilerplate code example:
	* FPhysCustomMovement_JetPack* copyPtr = new FPhysCustomMovement_JetPack(*this);
	* return copyPtr;
	*/

	return nullptr;
}

bool FPhysCustomMovement::NetSerialize(FArchive& archive, UPackageMap* map, bool& bOutSuccess)
{
	// TODO: or either increase p.NetPackedMovementMaxBits or remove some of the variables because we're replicating way too much stuff

	archive << CharacterMovementComponent;
	//archive << OnCustomMovementEnd; // TODO: should serialize delegates?
	archive << MovementName;
	archive << bIsActive;

	archive << CurrentVelocity;
	archive << CurrentTime;
	archive << MaxSpeed;
	archive << MaxAcceleration;
	archive << MaxBrakingDeceleration;
	archive << CustomModeFlag;

	uint8 FallbackMovementModeSerialize = static_cast<uint8>(FallbackMovementMode);
	archive << FallbackMovementModeSerialize;
	FallbackMovementMode = static_cast<EMovementMode>(FallbackMovementModeSerialize);

	// predicted properties
	archive << PhysBytes;
	archive << PhysBooleans;
	archive << PhysIntegers;
	archive << PhysFloats;
	archive << PhysDoubles;
	archive << PhysVectors;
	archive << PhysVectors2D;
	archive << PhysVectors4;
	archive << PhysRotators;
	archive << PhysQuats;
	// archive << PhysGameplayTags; // TODO: gameplay tags serializes to FName, we have to handle that in the serialization of the predicted property

	bOutSuccess = !archive.IsError();
	return true;
}

UScriptStruct* FPhysCustomMovement::GetScriptStruct() const
{
	return FPhysCustomMovement::StaticStruct();
}

FArchive& operator<<(FArchive& archive, FPhysCustomMovement& physCustomMovement)
{
	const bool bIsSaving = archive.IsSaving();

	//SerializeOptionalValue<UCharacterMovementComponent*>(bIsSaving, archive, physCustomMovement.CharacterMovementComponent.Get(), nullptr);
	//archive << physCustomMovement.CharacterMovementComponent.Get();
	archive << physCustomMovement.MovementName;
	archive << physCustomMovement.CurrentVelocity;
	archive << physCustomMovement.CurrentTime;
	archive << physCustomMovement.bIsActive;
	archive << physCustomMovement.MaxSpeed;
	archive << physCustomMovement.MaxAcceleration;
	archive << physCustomMovement.MaxBrakingDeceleration;
	SerializeOptionalValue<uint8>(bIsSaving, archive, physCustomMovement.CustomModeFlag, 0);

	uint8 FallbackMovementModeSerialize = static_cast<uint8>(physCustomMovement.FallbackMovementMode);
	archive << FallbackMovementModeSerialize;
	physCustomMovement.FallbackMovementMode = static_cast<EMovementMode>(FallbackMovementModeSerialize);

	archive << physCustomMovement.bSkipMovementUpdates;

	// predicted properties
	archive << physCustomMovement.PhysBytes;
	archive << physCustomMovement.PhysBooleans;
	archive << physCustomMovement.PhysIntegers;
	archive << physCustomMovement.PhysFloats;
	archive << physCustomMovement.PhysDoubles;
	archive << physCustomMovement.PhysVectors;
	archive << physCustomMovement.PhysVectors2D;
	archive << physCustomMovement.PhysVectors4;
	archive << physCustomMovement.PhysRotators;
	archive << physCustomMovement.PhysQuats;
	//archive << physCustomMovement.PhysGameplayTags;  // TODO: gameplay tags serializes to FName, we have to handle that in the serialization of the predicted property

	return archive;
}
