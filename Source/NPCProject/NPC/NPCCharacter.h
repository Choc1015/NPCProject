#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interaction/Interactable.h"
#include "NPCCharacter.generated.h"

class UDialogueData;
class UAnimationAsset;

/** A talkable NPC. Looks, name, dialogue and idle animation are set up in a Blueprint subclass. */
UCLASS()
class NPCPROJECT_API ANPCCharacter : public ACharacter, public IInteractable
{
	GENERATED_BODY()

public:
	ANPCCharacter();

	virtual void BeginPlay() override;
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

	/** Looping animation played on the mesh (no Anim Blueprint needed). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	TObjectPtr<UAnimationAsset> IdleAnimation;

	/** Degrees per second the NPC turns to face whoever talks to it. 0 disables turning. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	float TurnToFaceSpeed = 360.f;

private:
	TOptional<float> TargetYaw;
};
