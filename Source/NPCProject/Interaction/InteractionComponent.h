#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionFocusChanged, AActor*, FocusedActor, FText, Prompt);

/** Traces from the owner's view each tick and tracks the interactable actor in front of it. */
UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class NPCPROJECT_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Interacts with the currently focused actor. Returns false if nothing is focused. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool Interact();

	/** Disabling clears the current focus (used while a dialogue is open). */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractionFocusChanged OnFocusChanged;

	/** How far in front of the camera the player can interact (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDistance = 250.f;

	/** Sphere trace radius, so the player doesn't need pixel-perfect aim (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float TraceRadius = 15.f;

private:
	AActor* FindInteractableInView() const;
	void SetFocus(AActor* NewFocus);

	TWeakObjectPtr<AActor> FocusedActor;
	bool bInteractionEnabled = true;
};
