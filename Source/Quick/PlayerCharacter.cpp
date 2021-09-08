// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/InputSettings.h"
#include "PaperFlipbookComponent.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/GameplayStatics.h"

#define JUMP_BUTTON EKeys::Gamepad_FaceButton_Bottom
#define DASH_BUTTON EKeys::Gamepad_FaceButton_Right
#define WALLGRAB_BUTTON EKeys::Gamepad_LeftTrigger

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	WallCollision = CreateDefaultSubobject<USphereComponent>(TEXT("WallCollision"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;
	GetCharacterMovement()->bNotifyApex = true;

	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	RootComponent = GetCapsuleComponent();
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &APlayerCharacter::OnHit);
	GetCapsuleComponent()->GetBodyInstance()->bUseCCD = true;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateCharacter();
	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, UKismetStringLibrary::Conv_VectorToString(WallCollision->GetComponentVelocity()));
}

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("Horizontal", this, &APlayerCharacter::Horizontal);
	PlayerInputComponent->BindAxis("Vertical", this, &APlayerCharacter::Vertical);
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &APlayerCharacter::Dash);
}

void APlayerCharacter::Horizontal(float Value)
{
	const FVector Movement(1.f, 0.f, 0.f);
	AddMovementInput(Movement, Value);
}

void APlayerCharacter::Vertical(float Value)
{
	const FVector Movement(0.f, 0.f, 1.f);
	AddMovementInput(Movement, Value);
}

void APlayerCharacter::UpdateCharacter()
{
	UpdateAnim();
	if (bJumpModifiers)
	{
		JumpModifier();
	}

	const FVector PlayerVelocity = GetVelocity();
	float TravelDirection = PlayerVelocity.X;

	x = GetInputAxisValue("Horizontal");
	z = GetInputAxisValue("Vertical");

	if (Controller != nullptr)
	{
		if (TravelDirection < 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0, 180.0f, 0.0f));
		}
		else if (TravelDirection > 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
		}
	}

	if (bFacingWall && UGameplayStatics::GetPlayerController(GetWorld(), 0)->WasInputKeyJustPressed(WALLGRAB_BUTTON))
	{
		bWallGrab = true;
		bWallSlide = false;
	}
	if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->WasInputKeyJustReleased(WALLGRAB_BUTTON) || !bFacingWall)
	{
		bWallGrab = false;
		bWallSlide = false;
	}

	if (bWallGrab && !bIsDashing)
	{
		GetCharacterMovement()->GravityScale = 0;
	}

	if (GetCharacterMovement()->IsMovingOnGround() && !bIsDashing)
	{
		bWallJumped = false;
	}
	OnGround -= GetWorld()->GetDeltaSeconds();
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		OnGround = OnGroundRememberTime;
	}
	JumpPressed -= GetWorld()->GetDeltaSeconds();

	if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->WasInputKeyJustPressed(JUMP_BUTTON))
	{
		JumpPressed = JumpPressedRememberTime;
		bJumped = true;
	}
	if ((JumpPressed > 0) && bGroundTouch | (OnGround > 0))
	{
		JumpPressed = 0;
		bJumped = true;
		OnGround = 0;
		JumpImpl();
	}
	if (GetCharacterMovement()->IsMovingOnGround() && !bGroundTouch)
	{
		bGroundTouch = true;
		bJumped = false;
		bHasDashed = false;
		bIsDashing = false;
	}
	if (!GetCharacterMovement()->IsMovingOnGround() && bGroundTouch)
	{
		bGroundTouch = false;
	}

	if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->WasInputKeyJustPressed(DASH_BUTTON) && !bHasDashed && !bGroundTouch && !GetCharacterMovement()->IsMovingOnGround())
	{
		Dash();
	}
}

void APlayerCharacter::UpdateAnim()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	UPaperFlipbook* DesiredAnimation;
	if (bJumped)
	{
		DesiredAnimation = JumpAnimation;
	}
	else
	{
		DesiredAnimation = (PlayerSpeedSqr > 0.0f) ? RunningAnimation : IdleAnimation;
	}
	if (PlayerVelocity.Z < 0)
	{
		DesiredAnimation = FallAnimation;
	}
	if (bIsDashing)
	{
		DesiredAnimation = DashAnimation;
	}
	if (GetSprite()->GetFlipbook() != DesiredAnimation)
	{
		GetSprite()->SetFlipbook(DesiredAnimation);
	}
}

void APlayerCharacter::JumpImpl()
{
	GetCharacterMovement()->Velocity = FVector(GetCharacterMovement()->Velocity.X, 0.f, 0.f);
	FVector Dir(0.f, 0.f, 1.f);
	GetCharacterMovement()->AddImpulse(Dir * JumpForce, true);
}

void APlayerCharacter::Dash()
{
	bHasDashed = true;

	if (GetCharacterMovement()->Velocity.Z < 0)
	{
		DashSpeed = 550;
	}
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	FVector Movement(x, 0.f, z);
	GetCharacterMovement()->Velocity += Movement.GetUnsafeNormal() * DashSpeed;
	DashWait();
}

void APlayerCharacter::JumpModifier()
{
	if (GetCharacterMovement()->Velocity.Z < 0)
	{
		GetCharacterMovement()->Velocity += FVector(0.f, 0.f, 1.f) * GetCharacterMovement()->GetGravityZ() * (FallMultiplier - 1) * GetWorld()->GetDeltaSeconds();
	}
	else if (GetCharacterMovement()->Velocity.Z > 0 && !UGameplayStatics::GetPlayerController(GetWorld(), 0)->IsInputKeyDown(JUMP_BUTTON))
	{
		GetCharacterMovement()->Velocity += FVector(0.f, 0.f, 1.f) * GetCharacterMovement()->GetGravityZ() * (LowJumpMultiplier - 1) * GetWorld()->GetDeltaSeconds();
		bJumped = false;
	}
}

void APlayerCharacter::ResetGravity()
{
	GetCharacterMovement()->GravityScale = 1;
	bJumpModifiers = true;
	bWallJumped = false;
	bIsDashing = false;
}

void APlayerCharacter::DashWait()
{
	GetCharacterMovement()->GravityScale = 0;
	bIsDashing = true;
	bWallJumped = true;
	bJumpModifiers = false;
	GetWorld()->GetTimerManager().SetTimer(DashGravityTimerHandle, this, &APlayerCharacter::ResetGravity, 0.1, false, -1);
}

void APlayerCharacter::OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, UKismetStringLibrary::Conv_FloatToString(Hit.ImpactNormal.X));
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, FString::Printf(TEXT("Forward Vector: %f"), GetActorForwardVector().X));
	if (GetActorForwardVector().X != Hit.ImpactNormal.X && Hit.ImpactNormal.X != 0)
	{
		bFacingWall = true;
	}
	//else if (GetActorForwardVector().X == -1 && Hit.ImpactNormal.X == 1)
	//{
	//	bFacingWall = true;
	//}
	else bFacingWall = false;
}

void APlayerCharacter::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, UKismetStringLibrary::Conv_VectorToString(OtherComp->GetForwardVector()));
	if (GetActorForwardVector().X != SweepResult.ImpactNormal.X && SweepResult.ImpactNormal.X != 0)
	{
		bFacingWall = true;
	}
	//else if (GetActorForwardVector().X == -1 && SweepResult.ImpactNormal.X == 1)
	//{
	//	bFacingWall = true;
	//}
	else bFacingWall = false;
}