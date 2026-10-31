#include "Interaction/InteractionComponent.h"
#include "Interaction/Interactable.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!bInteractionEnabled || !OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	SetFocus(FindInteractableInView());
}

AActor* UInteractionComponent::FindInteractableInView() const
{
	AActor* Owner = GetOwner();
	FVector ViewLocation;
	FRotator ViewRotation;
	Owner->GetActorEyesViewPoint(ViewLocation, ViewRotation);

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * InteractionDistance;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractionTrace), false, Owner);

	FHitResult Hit;
	if (!GetWorld()->SweepSingleByChannel(Hit, ViewLocation, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(TraceRadius), Params))
	{
		return nullptr;
	}

	AActor* HitActor = Hit.GetActor();
	if (HitActor && HitActor->Implements<UInteractable>() && IInteractable::Execute_CanInteract(HitActor, Owner))
	{
		return HitActor;
	}
	return nullptr;
}

void UInteractionComponent::SetFocus(AActor* NewFocus)
{
	if (FocusedActor.Get() == NewFocus)
	{
		return;
	}

	FocusedActor = NewFocus;
	const FText Prompt = NewFocus ? IInteractable::Execute_GetInteractionPrompt(NewFocus) : FText::GetEmpty();
	OnFocusChanged.Broadcast(NewFocus, Prompt);
}

bool UInteractionComponent::Interact()
{
	AActor* Target = FocusedActor.Get();
	if (!bInteractionEnabled || !Target)
	{
		return false;
	}

	IInteractable::Execute_Interact(Target, GetOwner());
	return true;
}

void UInteractionComponent::SetInteractionEnabled(bool bEnabled)
{
	bInteractionEnabled = bEnabled;
	if (!bEnabled)
	{
		SetFocus(nullptr);
	}
}
