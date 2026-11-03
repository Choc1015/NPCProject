#include "Player/NPCPlayerCharacter.h"
#include "Interaction/InteractionComponent.h"
#include "Dialogue/DialogueComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"

ANPCPlayerCharacter::ANPCPlayerCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(40.f, 90.f);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	DialogueComponent = CreateDefaultSubobject<UDialogueComponent>(TEXT("DialogueComponent"));

	BaseEyeHeight = 70.f;
}

void ANPCPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	DialogueComponent->OnDialogueStarted.AddDynamic(this, &ThisClass::HandleDialogueStarted);
	DialogueComponent->OnDialogueEnded.AddDynamic(this, &ThisClass::HandleDialogueEnded);
}

void ANPCPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateIdleSway(DeltaSeconds);
}

void ANPCPlayerCharacter::UpdateIdleSway(float DeltaSeconds)
{
	const bool bStandingStill = bEnableIdleSway
		&& !GetCharacterMovement()->IsFalling()
		&& GetVelocity().Size2D() < IdleSwaySpeedThreshold;
	IdleSwayWeight = FMath::FInterpTo(IdleSwayWeight, bStandingStill ? 1.f : 0.f, DeltaSeconds, IdleSwayBlendSpeed);

	FirstPersonCamera->ClearAdditiveOffset();
	if (IdleSwayWeight <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	IdleSwayTime += DeltaSeconds;
	const float Phase = IdleSwayTime * UE_TWO_PI / IdleSwayPeriod;

	const FVector Offset(
		0.f,
		FMath::Sin(Phase * 0.5f) * IdleSwayWidth,
		FMath::Sin(Phase) * IdleSwayHeight);
	const FRotator Tilt(
		FMath::Sin(Phase + 0.6f) * IdleSwayPitchDegrees,
		0.f,
		FMath::Sin(Phase * 0.5f + 1.f) * IdleSwayRollDegrees);

	FirstPersonCamera->AddAdditiveOffset(FTransform(Tilt * IdleSwayWeight, Offset * IdleSwayWeight), 0.f);
}

void ANPCPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ANPCPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (MoveAction)
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
	}
	if (LookAction)
	{
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
	}
	if (JumpAction)
	{
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::OnJumpPressed);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	if (InteractAction)
	{
		Input->BindAction(InteractAction, ETriggerEvent::Started, this, &ThisClass::OnInteractPressed);
	}
}

void ANPCPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (DialogueComponent->IsInDialogue())
	{
		return;
	}

	const FVector2D Axis = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void ANPCPlayerCharacter::Look(const FInputActionValue& Value)
{
	if (DialogueComponent->IsInDialogue())
	{
		return;
	}

	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ANPCPlayerCharacter::OnJumpPressed()
{
	if (!DialogueComponent->IsInDialogue())
	{
		Jump();
	}
}

void ANPCPlayerCharacter::OnInteractPressed()
{
	if (DialogueComponent->IsInDialogue())
	{
		DialogueComponent->Continue();
	}
	else
	{
		InteractionComponent->Interact();
	}
}

void ANPCPlayerCharacter::HandleDialogueStarted(AActor* Speaker)
{
	InteractionComponent->SetInteractionEnabled(false);
	GetCharacterMovement()->StopMovementImmediately();
}

void ANPCPlayerCharacter::HandleDialogueEnded()
{
	InteractionComponent->SetInteractionEnabled(true);
}
