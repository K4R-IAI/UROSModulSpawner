// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


/**
 * 
 */



class UROSMODULSPAWNER_API FDataAssetSpawner
{
public:

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

    FDataAssetSpawner();
    ~FDataAssetSpawner();

    bool SpawnRobotFromAsset(FSpawnRobotParams Params);

};
