// Fill out your copyright notice in the Description page of Project Settings.


#include "SamuraiPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SamuraiCharacter.h"

void ASamuraiPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if(bAutoInputDisableOnBeginPlay)
		SetPlayerInputEnabled(false);
}

void ASamuraiPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
		return;

	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASamuraiPlayerController::Input_Move);
	}

	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASamuraiPlayerController::Input_Look);
	}

	if (AttackAction)
	{
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ASamuraiPlayerController::Input_Attack);
	}
}

void ASamuraiPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledCharacter = Cast<ASamuraiCharacter>(InPawn);
}

void ASamuraiPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	ControlledCharacter = nullptr;
}

void ASamuraiPlayerController::SetPlayerInputEnabled(bool bEnabled)
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem || !DefaultIMC) return;

	if (bEnabled)
	{
		Subsystem->AddMappingContext(DefaultIMC, 0);
		UE_LOG(LogTemp, Log, TEXT("[Input] DefaultIMC Added (Input Enabled)"));
	}
	else
	{
		Subsystem->RemoveMappingContext(DefaultIMC);
		UE_LOG(LogTemp, Log, TEXT("[Input] DefaultIMC Removed (Input Disabled)"));
	}
}

void ASamuraiPlayerController::Input_Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)return;

	const FRotator CameraRotation = GetControlRotation();
	const FRotator YawRotation(0.0f, CameraRotation.Yaw, 0.0f);

	const FVector ForwardVec = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightVec = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	ControlledPawn->AddMovementInput(ForwardVec, MovementVector.Y);
	ControlledPawn->AddMovementInput(RightVec, MovementVector.X);
}

void ASamuraiPlayerController::Input_Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddYawInput(LookAxisVector.X);
	AddPitchInput(LookAxisVector.Y);
}

void ASamuraiPlayerController::Input_Attack(const FInputActionValue& Value)
{
	const bool bIsAttack = Value.Get<bool>();

	if (!ControlledCharacter)
		return;

	ControlledCharacter->ExecAttack();
}