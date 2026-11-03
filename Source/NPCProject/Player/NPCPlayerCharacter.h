#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPCPlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UInteractionComponent;
class UDialogueComponent;
struct FInputActionValue;

/** First-person player: camera only, walks around and talks to NPCs. */
UCLASS()
class NPCPROJECT_API ANPCPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANPCPlayerCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }
	UInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }
	UDialogueComponent* GetDialogueComponent() const { return DialogueComponent; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDialogueComponent> DialogueComponent;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void OnInteractPressed();
	void OnJumpPressed();

	UFUNCTION()
	void HandleDialogueStarted(AActor* Speaker);

	UFUNCTION()
	void HandleDialogueEnded();
};
