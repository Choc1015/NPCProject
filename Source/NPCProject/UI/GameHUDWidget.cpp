#include "UI/GameHUDWidget.h"
#include "UI/DialogueChoiceButton.h"
#include "Interaction/InteractionComponent.h"
#include "Dialogue/DialogueComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"

void UGameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PromptPanel->SetVisibility(ESlateVisibility::Collapsed);
	DialoguePanel->SetVisibility(ESlateVisibility::Collapsed);
}

void UGameHUDWidget::InitForPawn(APawn* Pawn)
{
	if (UInteractionComponent* OldInteraction = Interaction.Get())
	{
		OldInteraction->OnFocusChanged.RemoveAll(this);
	}
	if (UDialogueComponent* OldDialogue = Dialogue.Get())
	{
		OldDialogue->OnDialogueStarted.RemoveAll(this);
		OldDialogue->OnDialogueLine.RemoveAll(this);
		OldDialogue->OnDialogueEnded.RemoveAll(this);
	}

	Interaction = Pawn ? Pawn->FindComponentByClass<UInteractionComponent>() : nullptr;
	Dialogue = Pawn ? Pawn->FindComponentByClass<UDialogueComponent>() : nullptr;

	if (UInteractionComponent* NewInteraction = Interaction.Get())
	{
		NewInteraction->OnFocusChanged.AddDynamic(this, &ThisClass::HandleFocusChanged);
	}
	if (UDialogueComponent* NewDialogue = Dialogue.Get())
	{
		NewDialogue->OnDialogueStarted.AddDynamic(this, &ThisClass::HandleDialogueStarted);
		NewDialogue->OnDialogueLine.AddDynamic(this, &ThisClass::HandleDialogueLine);
		NewDialogue->OnDialogueEnded.AddDynamic(this, &ThisClass::HandleDialogueEnded);
	}
}

void UGameHUDWidget::HandleFocusChanged(AActor* FocusedActor, FText Prompt)
{
	if (!FocusedActor)
	{
		PromptPanel->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	PromptText->SetText(FText::Format(INVTEXT("{0} {1}"), InteractKeyLabel, Prompt));
	PromptPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UGameHUDWidget::HandleDialogueStarted(AActor* Speaker)
{
	PromptPanel->SetVisibility(ESlateVisibility::Collapsed);
	DialoguePanel->SetVisibility(ESlateVisibility::Visible);
}

void UGameHUDWidget::HandleDialogueLine(FText Speaker, FText Text, const TArray<FText>& Choices)
{
	SpeakerText->SetText(Speaker);
	LineText->SetText(Text);

	ChoiceBox->ClearChildren();
	FSlateFontInfo Font = ChoiceFont.HasValidFont() ? ChoiceFont : LineText->GetFont();
	for (int32 Index = 0; Index < Choices.Num(); ++Index)
	{
		UDialogueChoiceButton* Button = WidgetTree->ConstructWidget<UDialogueChoiceButton>();
		Button->ChoiceIndex = Index;
		Button->SetBackgroundColor(FLinearColor(0.12f, 0.09f, 0.06f, 0.9f));
		Button->OnChoiceClicked.AddDynamic(this, &ThisClass::HandleChoiceClicked);

		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
		Label->SetText(FText::Format(INVTEXT("{0}. {1}"), FText::AsNumber(Index + 1), Choices[Index]));
		Label->SetFont(Font);
		Label->SetColorAndOpacity(ChoiceTextColor);
		Button->AddChild(Label);

		if (UVerticalBoxSlot* BoxSlot = ChoiceBox->AddChildToVerticalBox(Button))
		{
			BoxSlot->SetPadding(FMargin(0.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		}
	}

	const bool bHasChoices = Choices.Num() > 0;
	if (ContinueHint)
	{
		ContinueHint->SetVisibility(bHasChoices ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	SetUIInputMode(bHasChoices);
	OnLineShown(Speaker, Text, bHasChoices);
}

void UGameHUDWidget::HandleDialogueEnded()
{
	ChoiceBox->ClearChildren();
	DialoguePanel->SetVisibility(ESlateVisibility::Collapsed);
	SetUIInputMode(false);
}

void UGameHUDWidget::HandleChoiceClicked(int32 ChoiceIndex)
{
	if (UDialogueComponent* DialogueComponent = Dialogue.Get())
	{
		DialogueComponent->SelectChoice(ChoiceIndex);
	}
}

void UGameHUDWidget::SetUIInputMode(bool bShowCursor)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	PC->SetShowMouseCursor(bShowCursor);
	if (bShowCursor)
	{
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		PC->SetInputMode(Mode);
	}
	else
	{
		PC->SetInputMode(FInputModeGameOnly());
	}
}
