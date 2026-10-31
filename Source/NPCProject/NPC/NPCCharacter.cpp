#include "NPC/NPCCharacter.h"
#include "Dialogue/DialogueComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

ANPCCharacter::ANPCCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::Disabled;

	// The Pawn profile ignores Visibility, but the player's interaction trace runs on it.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ANPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IdleAnimation)
	{
		GetMesh()->PlayAnimation(IdleAnimation, true);
	}
}

void ANPCCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!TargetYaw.IsSet())
	{
		return;
	}

	FRotator Rotation = GetActorRotation();
	const float NewYaw = FMath::FixedTurn(Rotation.Yaw, TargetYaw.GetValue(), TurnToFaceSpeed * DeltaSeconds);
	if (FMath::IsNearlyEqual(FRotator::NormalizeAxis(NewYaw - TargetYaw.GetValue()), 0.f, 0.5f))
	{
		TargetYaw.Reset();
	}
	Rotation.Yaw = NewYaw;
	SetActorRotation(Rotation);
}

void ANPCCharacter::Interact_Implementation(AActor* Interactor)
{
	UDialogueComponent* DialogueComponent = Interactor ? Interactor->FindComponentByClass<UDialogueComponent>() : nullptr;
	if (!DialogueComponent || !DialogueComponent->StartDialogue(Dialogue, this, DisplayName))
	{
		return;
	}

	if (TurnToFaceSpeed > 0.f)
	{
		TargetYaw = (Interactor->GetActorLocation() - GetActorLocation()).Rotation().Yaw;
	}
}

FText ANPCCharacter::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}

bool ANPCCharacter::CanInteract_Implementation(AActor* Interactor) const
{
	return Dialogue != nullptr;
}
