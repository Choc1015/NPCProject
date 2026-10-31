#include "Player/NPCPlayerController.h"
#include "UI/GameHUDWidget.h"

void ANPCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UGameHUDWidget>(this, HUDWidgetClass);
		HUDWidget->AddToViewport();
		HUDWidget->InitForPawn(GetPawn());
	}

	SetInputMode(FInputModeGameOnly());
}

void ANPCPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (HUDWidget)
	{
		HUDWidget->InitForPawn(InPawn);
	}
}
