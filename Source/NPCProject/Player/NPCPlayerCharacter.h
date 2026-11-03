#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPCPlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UInteractionComponent;
class UDialogueComponent;
class USoundBase;
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
	virtual void Landed(const FHitResult& Hit) override;

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

	/** One of these plays (never the same twice in a row) every FootstepStride of ground travel. */
	UPROPERTY(EditAnywhere, Category = "Audio|Footsteps")
	TArray<TObjectPtr<USoundBase>> FootstepSounds;

	/** Ground distance (cm) between two footsteps. */
	UPROPERTY(EditAnywhere, Category = "Audio|Footsteps", meta = (ClampMin = "10"))
	float FootstepStride = 170.f;

	UPROPERTY(EditAnywhere, Category = "Audio|Footsteps", meta = (ClampMin = "0"))
	float FootstepVolume = 0.6f;

	/** Random pitch range per step so repeats don't sound mechanical. */
	UPROPERTY(EditAnywhere, Category = "Audio|Footsteps")
	FVector2D FootstepPitchRange = FVector2D(0.9f, 1.1f);

private:
	void PlayFootstep();

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void OnInteractPressed();
	void OnJumpPressed();

	UFUNCTION()
	void HandleDialogueStarted(AActor* Speaker);

	UFUNCTION()
	void HandleDialogueEnded();

	float FootstepDistance = 0.f;
	int32 LastFootstepIndex = INDEX_NONE;
};
