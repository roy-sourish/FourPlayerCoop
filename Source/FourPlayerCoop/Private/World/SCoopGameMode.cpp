// Fill out your copyright notice in the Description page of Project Settings.


#include "World/SCoopGameMode.h"
#include "World/SCoopGameState.h"
#include "AI/SAdvancedAI.h"
#include "Player/SPlayerState.h"
#include "Player/SPlayerController.h"


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
	
	SetWaveState(EWaveState::WaveInProgress);
}


void ASCoopGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawnner);

	SetWaveState(EWaveState::WaitingToComplete);
}


void ASCoopGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASCoopGameMode::StartWave, TimeBetweenWaves, false, 0.0f);

	SetWaveState(EWaveState::WaitingToStart);
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
		SetWaveState(EWaveState::WaveComplete);
		PrepareForNextWave();
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

	SetWaveState(EWaveState::GameOver);
	EndWave();
}


void ASCoopGameMode::SetWaveState(EWaveState NewState)
{
	ASCoopGameState* GS = GetGameState<ASCoopGameState>();
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}
