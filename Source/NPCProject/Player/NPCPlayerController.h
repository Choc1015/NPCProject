#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NPCPlayerController.generated.h"

class UGameHUDWidget;

/** Creates the HUD and keeps it pointed at the possessed pawn. */
UCLASS()
class NPCPROJECT_API ANPCPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UGameHUDWidget> HUDWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UGameHUDWidget> HUDWidget;
};
