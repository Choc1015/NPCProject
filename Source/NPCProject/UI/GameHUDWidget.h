#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameHUDWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UDialogueComponent;
class UInteractionComponent;

/**
 * In-game HUD: interaction prompt + dialogue box.
 * Layout lives in a Widget Blueprint (WBP_GameHUD); the named widgets below are bound from it.
 */
UCLASS(Abstract)
class NPCPROJECT_API UGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Hooks the HUD up to the player's interaction and dialogue components. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void InitForPawn(APawn* Pawn);

protected:
	virtual void NativeConstruct() override;

	/** Shown while looking at something interactable. Whole widget is hidden otherwise. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> PromptPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PromptText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> DialoguePanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeakerText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LineText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ChoiceBox;

	/** "Press E to continue" hint, hidden while choices are shown. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> ContinueHint;

	/** Key label put in front of the interaction prompt, e.g. "[E]". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	FText InteractKeyLabel = FText::FromString(TEXT("[E]"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	FSlateFontInfo ChoiceFont;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	FSlateColor ChoiceTextColor = FSlateColor(FLinearColor::White);

	/** Blueprint hook for polish (sounds, animations) whenever a new line appears. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnLineShown(const FText& Speaker, const FText& Text, bool bHasChoices);

private:
	UFUNCTION()
	void HandleFocusChanged(AActor* FocusedActor, FText Prompt);

	UFUNCTION()
	void HandleDialogueStarted(AActor* Speaker);

	UFUNCTION()
	void HandleDialogueLine(FText Speaker, FText Text, const TArray<FText>& Choices);

	UFUNCTION()
	void HandleDialogueEnded();

	UFUNCTION()
	void HandleChoiceClicked(int32 ChoiceIndex);

	void SetUIInputMode(bool bShowCursor);

	TWeakObjectPtr<UDialogueComponent> Dialogue;
	TWeakObjectPtr<UInteractionComponent> Interaction;
};
