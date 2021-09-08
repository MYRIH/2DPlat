// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "PlayerCharacter.generated.h"

/**
 *
 */
UCLASS()
class QUICK_API APlayerCharacter : public APaperCharacter
{
	GENERATED_BODY()

		APlayerCharacter();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class USphereComponent* WallCollision;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// The animation to play while running around
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UPaperFlipbook* RunningAnimation;

	// The animation to play while idle (standing still)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UPaperFlipbook* IdleAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UPaperFlipbook* JumpAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UPaperFlipbook* FallAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UPaperFlipbook* DashAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UPaperFlipbook* RollAnimation;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
		float RunSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
		float JumpForce;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
		float SlideSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
		float WallJumpLerp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
		float DashSpeed;
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "JumpSpecific")
		float JumpPressed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "JumpSpecific")
		float JumpPressedRememberTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "JumpSpecific")
		float OnGround;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "JumpSpecific")
		float OnGroundRememberTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "JumpSpecific")
		float FallMultiplier;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "JumpSpecific")
		float LowJumpMultiplier;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Booleans")
		bool bCanMove;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Booleans")
		bool bWallGrab;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Booleans")
		bool bWallJumped;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Booleans")
		bool bWallSlide;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Booleans")
		bool bFacingWall;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Booleans")
		bool bIsDashing;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Booleans")
		bool bGroundTouch;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Booleans")
		bool bHasDashed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Booleans")
		bool bJumped;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Booleans")
		bool bJumpModifiers;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Direction")
		int PlayerDirection;

	float x;
	float z;

public:

	UFUNCTION(BlueprintCallable)
		void Horizontal(float Value);

	UFUNCTION(BlueprintCallable)
		void Vertical(float Value);

	UFUNCTION(BlueprintCallable)
		void UpdateCharacter();

	UFUNCTION(BlueprintCallable)
		void UpdateAnim();

	UFUNCTION(BlueprintCallable)
		void JumpImpl();

	UFUNCTION(BlueprintCallable)
		void Dash();

	void JumpModifier();

public:
	FTimerHandle DashGravityTimerHandle;

	FTimerHandle DashAnimTimerHandle;

	UFUNCTION()
		void ResetGravity();

	UFUNCTION()
		void DashWait();

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
