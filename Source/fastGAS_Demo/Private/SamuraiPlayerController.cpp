// Fill out your copyright notice in the Description page of Project Settings.


#include "SamuraiPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SamuraiCharacter.h"
#include "CombatGameplayTags.h"

void ASamuraiPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if(bAutoInputDisableOnBeginPlay)
		SetPlayerInputEnabled(false);
	else
		SetPlayerInputEnabled(true);
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
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASamuraiPlayerController::Input_MoveEnd);
	}

	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASamuraiPlayerController::Input_Look);
	}

	if (AttackAction)
	{
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ASamuraiPlayerController::Input_Attack);
	}

	if (LockAction)
	{
		EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Triggered, this, &ASamuraiPlayerController::Input_Lock);
	}

	if (DodgeAction)
	{
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASamuraiPlayerController::Input_Dodge);
	}
}

void ASamuraiPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledCharacter = Cast<ASamuraiCharacter>(InPawn);

	auto ASC = ControlledCharacter->GetAbilitySystemComponent();
	HandleForOnDodgeTagChanged = ASC->RegisterGameplayTagEvent(CombatGameplayTags::CGTAG_STATE_Action_Dodge)
		.AddUObject(this, &ASamuraiPlayerController::OnDodgeTagChanged);
}

void ASamuraiPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	auto ASC = ControlledCharacter->GetAbilitySystemComponent();
	ASC->UnregisterGameplayTagEvent(HandleForOnDodgeTagChanged, CombatGameplayTags::CGTAG_STATE_Action_Dodge);
	HandleForOnDodgeTagChanged.Reset();

	ControlledCharacter = nullptr;
}

void ASamuraiPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// カメラの向きをターゲットの方向に向ける
	ASamuraiCharacter* Samurai = Cast<ASamuraiCharacter>(GetPawn());
	if (!Samurai) return;

	if (AActor* Target = Samurai->GetLockedOnTarget().Get())
	{
		const FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
		const FVector TargetLocation = Target->GetActorLocation();

		const FRotator CurrentRot = GetControlRotation();
		const FRotator TargetRot = (TargetLocation - CameraLocation).Rotation();
		FRotator NewRot = FMath::RInterpTo(CurrentRot,TargetRot,DeltaTime, TurnAroundSpeedForLockon);

		SetControlRotation(NewRot);
	}
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

	if (!ControlledCharacter)return;

	const FRotator CameraRotation = GetControlRotation();
	const FRotator YawRotation(0.0f, CameraRotation.Yaw, 0.0f);

	const FVector ForwardVec = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightVec = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector DesiredDirection = (ForwardVec * MovementVector.Y + RightVec * MovementVector.X).GetSafeNormal();
	ControlledCharacter->RequestMove(DesiredDirection, 1.0f);
}

void ASamuraiPlayerController::Input_MoveEnd(const FInputActionValue& Value)
{
	if (!ControlledCharacter)return;

	ControlledCharacter->RequestMove(FVector::ZeroVector, 0.0f);
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

void ASamuraiPlayerController::Input_Lock(const FInputActionValue& Value)
{
	const bool bIsPush = Value.Get<bool>();
	if (!bIsPush)
		return;

	if (!ControlledCharacter)
		return;

	ControlledCharacter->ToggleLockOnTarget();
}

void ASamuraiPlayerController::Input_Dodge(const FInputActionValue& Value)
{
	const bool bIsPush = Value.Get<bool>();

	if (bIsPush)
	{
		if (!ControlledCharacter)
			return;
		ControlledCharacter->ExecDodge();
		return;
	}
}

void ASamuraiPlayerController::OnDodgeTagChanged(FGameplayTag Tag, int32 NewCount)
{
	bool bIsAdded = (NewCount > 0);

	if (bIsAdded)
	{
		return;
	}
}
