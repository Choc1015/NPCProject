#include "NPC/NPCCharacter.h"
#include "Dialogue/DialogueComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

ANPCCharacter::ANPCCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::Disabled;

	// The Pawn profile ignores Visibility, but the player's interaction trace runs on it.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ANPCCharacter::PlayRandomTalkAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	const int32 Count = TalkAnimations.Num();
	if (!AnimInstance || Count == 0)
	{
		return;
	}

	// Pick any index except the previous one (when there is more than one to choose from).
	int32 Index = FMath::RandRange(0, Count - 1);
	if (Count > 1 && Index == LastTalkIndex)
	{
		Index = (Index + FMath::RandRange(1, Count - 1)) % Count;
	}
	LastTalkIndex = Index;

	// A new montage in the same slot blends out the one still playing, and a finished
	// montage blends back to the idle pose feeding the slot.
	if (UAnimSequenceBase* Animation = TalkAnimations[Index])
	{
		AnimInstance->PlaySlotAnimationAsDynamicMontage(Animation, TalkSlotName, TalkBlendInTime, TalkBlendOutTime);
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
	if (!DialogueComponent)
	{
		return;
	}

	// Bind before starting: StartDialogue broadcasts the first line immediately.
	BindToDialogue(DialogueComponent);
	if (!DialogueComponent->StartDialogue(Dialogue, this, DisplayName))
	{
		UnbindFromDialogue();
		return;
	}

	if (TurnToFaceSpeed > 0.f)
	{
		TargetYaw = (Interactor->GetActorLocation() - GetActorLocation()).Rotation().Yaw;
	}
}

void ANPCCharacter::BindToDialogue(UDialogueComponent* DialogueComponent)
{
	ActiveDialogue = DialogueComponent;
	DialogueComponent->OnDialogueLine.AddDynamic(this, &ThisClass::HandleDialogueLine);
	DialogueComponent->OnDialogueEnded.AddDynamic(this, &ThisClass::HandleDialogueEnded);
}

void ANPCCharacter::UnbindFromDialogue()
{
	if (UDialogueComponent* DialogueComponent = ActiveDialogue.Get())
	{
		DialogueComponent->OnDialogueLine.RemoveDynamic(this, &ThisClass::HandleDialogueLine);
		DialogueComponent->OnDialogueEnded.RemoveDynamic(this, &ThisClass::HandleDialogueEnded);
	}
	ActiveDialogue.Reset();
}

void ANPCCharacter::HandleDialogueLine(FText SpeakerName, FText Text, const TArray<FText>& Choices)
{
	PlayRandomTalkAnimation();
}

void ANPCCharacter::HandleDialogueEnded()
{
	// The current talk animation finishes on its own and blends back to idle.
	UnbindFromDialogue();
}

FText ANPCCharacter::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}

bool ANPCCharacter::CanInteract_Implementation(AActor* Interactor) const
{
	return Dialogue != nullptr;
}
