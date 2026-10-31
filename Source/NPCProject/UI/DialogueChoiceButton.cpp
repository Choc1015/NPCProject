#include "UI/DialogueChoiceButton.h"

UDialogueChoiceButton::UDialogueChoiceButton()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
	}
}

void UDialogueChoiceButton::HandleClicked()
{
	OnChoiceClicked.Broadcast(ChoiceIndex);
}
