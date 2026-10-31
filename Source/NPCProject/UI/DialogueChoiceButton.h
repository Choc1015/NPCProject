#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "DialogueChoiceButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChoiceButtonClicked, int32, ChoiceIndex);

/** Button that remembers which dialogue choice it represents. Built at runtime by UGameHUDWidget. */
UCLASS()
class NPCPROJECT_API UDialogueChoiceButton : public UButton
{
	GENERATED_BODY()

public:
	UDialogueChoiceButton();

	int32 ChoiceIndex = INDEX_NONE;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnChoiceButtonClicked OnChoiceClicked;

private:
	UFUNCTION()
	void HandleClicked();
};
