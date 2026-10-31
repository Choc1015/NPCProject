#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueComponent.generated.h"

class UDialogueData;
class USoundBase;
struct FDialogueNode;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDialogueLine, FText, SpeakerName, FText, Text, const TArray<FText>&, Choices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, AActor*, Speaker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

/** Runs a conversation on the player side: walks the nodes of a UDialogueData and reports lines to the UI. */
UCLASS(ClassGroup = (Dialogue), meta = (BlueprintSpawnableComponent))
class NPCPROJECT_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool StartDialogue(UDialogueData* Dialogue, AActor* Speaker, FText SpeakerName);

	/** Advances a line that has no choices. Ignored while choices are shown. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void Continue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectChoice(int32 ChoiceIndex);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsInDialogue() const { return CurrentDialogue != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsWaitingForChoice() const;

	/** Voice-over of the line currently shown, if it has one. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	USoundBase* GetCurrentVoice() const;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueStarted OnDialogueStarted;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueLine OnDialogueLine;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueEnded OnDialogueEnded;

private:
	void GoToNode(FName NodeId);

	UPROPERTY()
	TObjectPtr<UDialogueData> CurrentDialogue;

	TWeakObjectPtr<AActor> CurrentSpeaker;
	FText CurrentSpeakerName;
	const FDialogueNode* CurrentNode = nullptr;
};
