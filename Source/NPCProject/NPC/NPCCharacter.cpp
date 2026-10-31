#include "NPC/NPCCharacter.h"
#include "Dialogue/DialogueComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

ANPCCharacter::ANPCCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::Disabled;

	// The Pawn profile ignores Visibility, but the player's interaction trace runs on it.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	VoiceAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudio"));
	VoiceAudio->SetupAttachment(GetCapsuleComponent());
	VoiceAudio->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	VoiceAudio->bAutoActivate = false;
	// Heard from the NPC's position: full volume up close, fading out across the camp.
	VoiceAudio->bOverrideAttenuation = true;
	VoiceAudio->AttenuationOverrides.bAttenuate = true;
	VoiceAudio->AttenuationOverrides.bSpatialize = true;
	VoiceAudio->AttenuationOverrides.AttenuationShapeExtents = FVector(400.f);
	VoiceAudio->AttenuationOverrides.FalloffDistance = 1600.f;
}

void ANPCCharacter::PlayVoice(USoundBase* Voice)
{
	if (!Voice)
	{
		StopVoice();
		return;
	}
	// SetSound cuts off whatever line is still playing.
	VoiceAudio->SetSound(Voice);
	VoiceAudio->Play();
}

void ANPCCharacter::StopVoice()
{
	if (VoiceAudio->IsPlaying())
	{
		VoiceAudio->FadeOut(VoiceFadeOutTime, 0.f);
	}
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

	const UDialogueComponent* DialogueComponent = ActiveDialogue.Get();
	PlayVoice(DialogueComponent ? DialogueComponent->GetCurrentVoice() : nullptr);
}

void ANPCCharacter::HandleDialogueEnded()
{
	// The current talk animation finishes on its own and blends back to idle.
	StopVoice();
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
