// Fill out your copyright notice in the Description page of Project Settings.


#include "World/SCoopGameMode.h"
#include "World/SCoopGameState.h"
#include "AI/SAdvancedAI.h"
#include "Player/SPlayerState.h"
#include "Player/SPlayerController.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"


ASCoopGameMode::ASCoopGameMode()
{
	// Set Default Classes
	PlayerControllerClass = ASPlayerController::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();
	GameStateClass = ASCoopGameState::StaticClass();

	bAllowFriendlyFireDamage = false;

	/* NOTE: Need to set bWantsPlayerState to true in AIController to assign PlayerState to the AI,
	   and to set its default team number to 0 (0 and 1 for the AI and Players resp.) */
	PlayerTeamNum = 1;


	/* Tick every 1 second */
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;

	WaveCount = 0;
	NrOfBotsToSpawn = 0;

	TimeBetweenWaves = 2.0f;

	bSpawnAtTeamPlayer = true;

	ScoreWaveSurvived = 1000;
}


void ASCoopGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}


void ASCoopGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();

	CheckAnyPlayerAlive();
}


void ASCoopGameMode::Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType)
{
	ASPlayerState* KillerPS = Killer ? Cast<ASPlayerState>(Killer->PlayerState) : nullptr;
	ASPlayerState* VictimPS = VictimPlayer ? Cast<ASPlayerState>(VictimPlayer->PlayerState) : nullptr;

	if (KillerPS && (KillerPS != VictimPS) && (!KillerPS->IsABot()) )
	{
		KillerPS->AddKill();
		KillerPS->ScorePoints(10);
	}

	if (VictimPS && !VictimPS->IsABot())
	{
		VictimPS->AddDeath();
	}
}


/// <summary>
/// Spawn the player next to his living coop buddy instead of PlayerStart 
/// </summary>
/// <param name="NewPlayer"></param>

void ASCoopGameMode::RestartPlayer(AController* NewPlayer)
{
	/* Fall back to PlayerStart if team spawning is disabled or we are trying to spawn a bot */
	if (!bSpawnAtTeamPlayer || NewPlayer->PlayerState && NewPlayer->PlayerState->IsABot())
	{
		Super::RestartPlayer(NewPlayer);
		return;
	}

	/* Look for a live player to spawn next to */
	FVector SpawnOrigin = FVector::ZeroVector;
	FRotator StartRotation = FRotator::ZeroRotator;
	for (TActorIterator<APawn> It(GetWorld()); It; ++It) // inside EngineUtils.h
	{
		ASCharacter* MyPawn = Cast<ASCharacter>(*It);
		if (MyPawn && MyPawn->IsAlive())
		{
			SpawnOrigin = MyPawn->GetActorLocation();
			StartRotation = MyPawn->GetActorRotation();
			break;
		}
	}

	/* If no players alive (yet) - spawn using one of the player start */
	if (SpawnOrigin == FVector::ZeroVector)
	{
		Super::RestartPlayer(NewPlayer);
		return;
	}

	/* Get a point on the nav mesh near the other player */
	FNavLocation StartLocation;
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(this);
	if (NavSystem && NavSystem->GetRandomPointInNavigableRadius(SpawnOrigin, 250.0f, StartLocation))
	{
		// Try to create a pawn to use the default class for the player 
		if (NewPlayer->GetPawn() == nullptr && GetDefaultPawnClassForController(NewPlayer) != nullptr)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Instigator = GetInstigator();
			APawn* ResultPawn = GetWorld()->SpawnActor<APawn>(GetDefaultPawnClassForController(NewPlayer), StartLocation.Location, StartRotation, SpawnInfo);
			if (ResultPawn == nullptr)
			{
				UE_LOG(LogGameMode, Warning, TEXT("Couldn't spawn Pawn of type %s at %s"), *GetNameSafe(DefaultPawnClass), &StartLocation.Location);
			}
			NewPlayer->SetPawn(ResultPawn);
		}

		if (NewPlayer->GetPawn() == nullptr)
		{
			NewPlayer->FailedToSpawnPawn();
		}
		else
		{
			NewPlayer->Possess(NewPlayer->GetPawn());

			/* If Pawn is destroyed as a part of possession we have to abort */
			if (NewPlayer->GetPawn() == nullptr)
			{
				NewPlayer->FailedToSpawnPawn();
			}
			else
			{
				// Set initial control rotation to the player start's rotation 
				NewPlayer->ClientSetRotation(NewPlayer->GetPawn()->GetActorRotation(), true);

				FRotator NewControllerRot = StartRotation;
				NewControllerRot.Roll = 0.0f;
				NewPlayer->SetControlRotation(NewControllerRot);

				SetPlayerDefaults(NewPlayer->GetPawn());
			}
		}

	}
}


void ASCoopGameMode::SpawnBotTimerElapsed()
{
	/* Spawn a single bot - Blueprint Implementable Event, runs bot spawn EQS | defined in BP_SCoopGameMode
	   NOTE: We don't want blurprint to do the book keeping, we do that here in c++ */
	SpawnNewBot();

	NrOfBotsToSpawn--;

	if (NrOfBotsToSpawn <= 0)
	{
		EndWave();
	}
}


void ASCoopGameMode::StartWave()
{
	// Increment wave count and set number of bots to spawn double the current wave count 
	WaveCount++;
	NrOfBotsToSpawn = 2 * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawnner, this, &ASCoopGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);
	
	SetWaveState(EWaveState::WaveInProgress, WaveCount);
}


void ASCoopGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawnner);

	SetWaveState(EWaveState::WaitingToComplete, WaveCount);
}


void ASCoopGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASCoopGameMode::StartWave, TimeBetweenWaves, false, 0.0f);

	SetWaveState(EWaveState::WaitingToStart, WaveCount);
}


void ASCoopGameMode::CheckWaveState()
{
	bool bIsPreparingForNextWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	if (NrOfBotsToSpawn > 0 || bIsPreparingForNextWave)
	{
		return;
	}
	
	bool bIsAnyBotAlive = false;
	
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		ASAdvancedAI* BotPawn = Cast<ASAdvancedAI>(TestPawn);
		//ASPlayerState* BotPlayerState = Cast<ASPlayerState>(BotPawn->GetPlayerState());
		if (BotPawn && BotPawn->GetHealth() > 0.0f)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive)
	{
		SetWaveState(EWaveState::WaveComplete, WaveCount);
		PrepareForNextWave();

		// Restart Player when no bots are alive in current wave 
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
		{
			/* Look for all players that are spectating */
			ASPlayerController* MyController = Cast<ASPlayerController>(*It);
			if (MyController)
			{
				if (MyController->PlayerState->IsSpectator())
				{
					RestartPlayer(MyController);
					MyController->ClientHUDStateChanged(EHUDState::Playing);
				}
				else
				{
					/* Add Wave Survived Points on wave survival */
					ASCharacter* MyPawn = Cast<ASCharacter>(MyController->GetPawn());
					if (MyPawn && MyPawn->IsAlive())
					{
						ASPlayerState* PS = Cast<ASPlayerState>(MyController->PlayerState);
						if (PS)
						{
							PS->ScorePoints(ScoreWaveSurvived);
						}
					}
				}
			}
		}
	}
}


void ASCoopGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASPlayerController* PC = Cast<ASPlayerController>(It->Get());
		if (PC && PC->GetPawn())
		{
			ASCharacter* PlayerPawn = Cast<ASCharacter>(PC->GetPawn());
			if (PlayerPawn && PlayerPawn->GetHealth() > 0.0f)
			{
				// A player is still alive 
				return;
			}
		}
	}

	// No player alive. End Game 
	GameOver();
}


void ASCoopGameMode::GameOver()
{
	//UE_LOG(LogTemp, Error, TEXT("GAME OVER!!! \n All Players are dead!"));

	SetWaveState(EWaveState::GameOver, WaveCount);
	EndWave();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		/* Look for all players that are spectating */
		ASPlayerController* MyController = Cast<ASPlayerController>(*It);
		if (MyController)
		{
			MyController->ClientHUDStateChanged(EHUDState::MatchEnd);
		}
	}
}


void ASCoopGameMode::SetWaveState(EWaveState NewState, int32 NewWaveCount)
{
	ASCoopGameState* GS = GetGameState<ASCoopGameState>();
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState, NewWaveCount);
	}
}
