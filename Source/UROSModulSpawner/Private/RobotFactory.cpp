// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotFactory.h"
#include "XmlFile.h"
#include "HAL/FileManagerGeneric.h"
#include "SDF/SDFDataAsset.h"
#include "Physics/RModel.h"
#include "Factory/RModelBuilder.h"
#include "ROSBridgeGameInstance.h"
#include "Engine/PrimaryAssetLabel.h"
#include "SpawnRobotServer.h"
#include "DataAssetSpawner.h"
#include "Async/TaskGraphInterfaces.h"


// Sets default values
ARobotFactory::ARobotFactory()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARobotFactory::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void ARobotFactory::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void ARobotFactory::SpawnRobot(FString InMessage,UWorld* World)
{
    UE_LOG(LogTemp, Log, TEXT("RobotFactory starts to work... "));

 }//End Spawn Robot



bool ARobotFactory::SpawnRobotFromAsset(const FSpawnRobotParams Params)
{
    UE_LOG(LogTemp, Log, TEXT("RobotFactory starts to work... "));
    UWorld* World=Params.World;
    FString AssetToSpawn=Params.DataAssetToSpawn;

    UE_LOG(LogTemp, Log, TEXT("Copied path to file: SDFDataAsset'/Game/Robots/PR2/pr2.pr2"));
    UE_LOG(LogTemp, Log, TEXT("Params.path: %s"),*AssetToSpawn);


    USDFDataAsset* SDFDataAssetS=Cast<USDFDataAsset>(StaticLoadObject(USDFDataAsset::StaticClass(),NULL,*AssetToSpawn)); //--> Error not in Game Thread...

    if(SDFDataAssetS)
    {
        UE_LOG(LogTemp, Log, TEXT("The RobotFactory found some Blueprints to work with... "));
        //We found an SDF Data Asset Spawn
        ARModel* ActortoSpawn= NewObject<ARModel>();
        USDFModel* ModeltoSpawn= SDFDataAssetS->Models[0]; // There should only be one Model in the DataAsset, if not which one should I spawn?
        FVector Position =FVector(100,-100,20);
        FRotator Rotation = FRotator(0,0,0);
        FActorSpawnParameters SpawnParams;
        ActortoSpawn = World->SpawnActor<ARModel>(Position,Rotation,SpawnParams);
        if(ActortoSpawn)
        {
            UE_LOG(LogTemp, Log, TEXT("The spawned Actor is ARModel. "));
        }
        URModelBuilder* BuildingFacotry= NewObject<URModelBuilder>();
        BuildingFacotry->Load(ModeltoSpawn,ActortoSpawn);

        return true;
    }
    return false;
}
