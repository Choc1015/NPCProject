#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueData.generated.h"

/** A reply the player can pick. NextNodeId = None ends the conversation. */
USTRUCT(BlueprintType)
struct FDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName NextNodeId;
};

/** One NPC line. With no choices, Continue moves to NextNodeId (None ends the conversation). */
USTRUCT(BlueprintType)
struct FDialogueNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName NodeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (MultiLine = true))
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueChoice> Choices;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName NextNodeId;
};

/** A whole conversation, authored in the editor as a list of nodes. */
UCLASS(BlueprintType)
class NPCPROJECT_API UDialogueData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName StartNodeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (TitleProperty = "NodeId"))
	TArray<FDialogueNode> Nodes;

	const FDialogueNode* FindNode(FName NodeId) const;
};
