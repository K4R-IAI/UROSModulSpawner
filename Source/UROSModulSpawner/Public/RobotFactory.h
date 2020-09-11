// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SDF/SDFDataAsset.h"
#include "RobotFactory.generated.h"

UCLASS()
class UROSMODULSPAWNER_API ARobotFactory : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARobotFactory();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    bool SpawnRobotFromAsset(UWorld* InWorld,USDFDataAsset* InDataAsset);
    bool SpawnRobotFromAsset(UWorld* InWorld, FString InPath);

struct FSpawnRobotParams
{
//    FString Id;
//    FString Name;
    FString DataAssetToSpawn;
//    FString ParentId;
    UWorld* World;
//    FVector Location;
//    FRotator Rotator;

};
    void SpawnRobot(FString InMessage,UWorld* World);
    bool SpawnRobotFromAsset(const FSpawnRobotParams Params);

};
