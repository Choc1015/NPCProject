#include "Dialogue/DialogueComponent.h"
#include "Dialogue/DialogueData.h"

bool UDialogueComponent::StartDialogue(UDialogueData* Dialogue, AActor* Speaker, FText SpeakerName)
{
	if (IsInDialogue() || !Dialogue || !Dialogue->FindNode(Dialogue->StartNodeId))
	{
		return false;
	}

	CurrentDialogue = Dialogue;
	CurrentSpeaker = Speaker;
	CurrentSpeakerName = SpeakerName;

	OnDialogueStarted.Broadcast(Speaker);
	GoToNode(Dialogue->StartNodeId);
	return true;
}

bool UDialogueComponent::IsWaitingForChoice() const
{
	return CurrentNode && CurrentNode->Choices.Num() > 0;
}

void UDialogueComponent::Continue()
{
	if (CurrentNode && !IsWaitingForChoice())
	{
		GoToNode(CurrentNode->NextNodeId);
	}
}

void UDialogueComponent::SelectChoice(int32 ChoiceIndex)
{
	if (CurrentNode && CurrentNode->Choices.IsValidIndex(ChoiceIndex))
	{
		GoToNode(CurrentNode->Choices[ChoiceIndex].NextNodeId);
	}
}

void UDialogueComponent::GoToNode(FName NodeId)
{
	CurrentNode = CurrentDialogue ? CurrentDialogue->FindNode(NodeId) : nullptr;
	if (!CurrentNode)
	{
		EndDialogue();
		return;
	}

	TArray<FText> ChoiceTexts;
	for (const FDialogueChoice& Choice : CurrentNode->Choices)
	{
		ChoiceTexts.Add(Choice.Text);
	}
	OnDialogueLine.Broadcast(CurrentSpeakerName, CurrentNode->Text, ChoiceTexts);
}

void UDialogueComponent::EndDialogue()
{
	if (!IsInDialogue())
	{
		return;
	}

	CurrentDialogue = nullptr;
	CurrentNode = nullptr;
	CurrentSpeaker.Reset();
	OnDialogueEnded.Broadcast();
}
