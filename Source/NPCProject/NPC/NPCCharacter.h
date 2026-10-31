#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interaction/Interactable.h"
#include "NPCCharacter.generated.h"

class UDialogueData;
class UAnimSequenceBase;
class UDialogueComponent;
class UAudioComponent;

/**
 * A talkable NPC. Looks, name, dialogue and animations are set up in a Blueprint subclass.
 * Idle lives in the mesh's Anim Blueprint; talk animations play on top of it as montages
 * through the Anim Blueprint's slot, so idle <-> talk and talk <-> talk blend smoothly.
 */
UCLASS()
class NPCPROJECT_API ANPCCharacter : public ACharacter, public IInteractable
{
	GENERATED_BODY()

public:
	ANPCCharacter();

	virtual void Tick(float DeltaSeconds) override;

	// IInteractable
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	FText DisplayName;

	/** Shown on the HUD while the player looks at this NPC. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	FText InteractionPrompt = NSLOCTEXT("NPC", "TalkPrompt", "Talk");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	TObjectPtr<UDialogueData> Dialogue;

	/** One of these plays once on every dialogue line, never the same one twice in a row. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Animation")
	TArray<TObjectPtr<UAnimSequenceBase>> TalkAnimations;

	/** Anim Blueprint slot the talk montages play in. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Animation")
	FName TalkSlotName = TEXT("DefaultSlot");

	/** Blend time into a talk animation (from idle or from the previous talk animation). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Animation")
	float TalkBlendInTime = 0.25f;

	/** Blend time from a finished talk animation back to idle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Animation")
	float TalkBlendOutTime = 0.4f;

	/** Plays each dialogue line's voice-over from the NPC's head. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Voice")
	TObjectPtr<UAudioComponent> VoiceAudio;

	/** Fade-out when the conversation ends (or moves to a line without voice) mid voice line. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Voice")
	float VoiceFadeOutTime = 0.15f;

	/** Degrees per second the NPC turns to face whoever talks to it. 0 disables turning. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	float TurnToFaceSpeed = 360.f;

private:
	void PlayRandomTalkAnimation();
	void PlayVoice(USoundBase* Voice);
	void StopVoice();
	void BindToDialogue(UDialogueComponent* DialogueComponent);
	void UnbindFromDialogue();

	UFUNCTION()
	void HandleDialogueLine(FText SpeakerName, FText Text, const TArray<FText>& Choices);

	UFUNCTION()
	void HandleDialogueEnded();

	TOptional<float> TargetYaw;
	TWeakObjectPtr<UDialogueComponent> ActiveDialogue;
	int32 LastTalkIndex = INDEX_NONE;
};
