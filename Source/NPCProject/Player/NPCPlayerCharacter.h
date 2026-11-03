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
	virtual void Tick(float DeltaSeconds) override;

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

	/** Idle "breathing" sway, faded in while the player stands still. */
	UPROPERTY(EditAnywhere, Category = "Camera|Idle Sway")
	bool bEnableIdleSway = true;

	/** Seconds per breath. */
	UPROPERTY(EditAnywhere, Category = "Camera|Idle Sway", meta = (ClampMin = "0.5"))
	float IdleSwayPeriod = 4.5f;

	/** Up/down camera motion per breath, in cm. */
	UPROPERTY(EditAnywhere, Category = "Camera|Idle Sway", meta = (ClampMin = "0"))
	float IdleSwayHeight = 0.6f;

	/** Slow side-to-side drift (half the breath rate), in cm. */
	UPROPERTY(EditAnywhere, Category = "Camera|Idle Sway", meta = (ClampMin = "0"))
	float IdleSwayWidth = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Camera|Idle Sway", meta = (ClampMin = "0"))
	float IdleSwayPitchDegrees = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Camera|Idle Sway", meta = (ClampMin = "0"))
	float IdleSwayRollDegrees = 0.25f;

	/** Below this ground speed (cm/s) the player counts as standing still. */
	UPROPERTY(EditAnywhere, Category = "Camera|Idle Sway", meta = (ClampMin = "0"))
	float IdleSwaySpeedThreshold = 10.f;

	/** How fast the sway fades in/out. */
	UPROPERTY(EditAnywhere, Category = "Camera|Idle Sway", meta = (ClampMin = "0.1"))
	float IdleSwayBlendSpeed = 1.5f;

private:
	void UpdateIdleSway(float DeltaSeconds);

	float IdleSwayTime = 0.f;
	float IdleSwayWeight = 0.f;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void OnInteractPressed();
	void OnJumpPressed();

	UFUNCTION()
	void HandleDialogueStarted(AActor* Speaker);

	UFUNCTION()
	void HandleDialogueEnded();
};
