// Fill out your copyright notice in the Description page of Project Settings.

#include "DataAssetSpawner.h"
#include "SDF/SDFDataAsset.h"
#include "Physics/RModel.h"
#include "Factory/RModelBuilder.h"


FDataAssetSpawner::FDataAssetSpawner()
{
}

FDataAssetSpawner::~FDataAssetSpawner()
{
}
bool FDataAssetSpawner::SpawnRobotFromAsset(FSpawnRobotParams Params)
{
    UWorld* World=Params.World;
    FString AssetToSpawn=Params.DataAssetToSpawn;

    USDFDataAsset* SDFDataAssetL=FindObject<USDFDataAsset>(ANY_PACKAGE,TEXT("SDFDataAsset'/Game/Robots/PR2/pr2.pr2"));
//            UE_LOG(LogTemp, Warning, TEXT("[%s]: help = %s"), *FString(__FUNCTION__),*help);


    StaticLoadObject(UStaticMesh::StaticClass(),nullptr,TEXT("StaticMesh'/Game/Private/Models/IAISupermarket/Catalog/ProductWithAN010055/SM_ProductWithAN010055.SM_ProductWithAN010055'"));
    //StaticLoadObject(USDFDataAsset::StaticClass(),NULL,*Valeera); // cannot load dataAssets?

//            USDFDataAsset* SDFDataAssetS=Cast<USDFDataAsset>(StaticLoadObject(USDFDataAsset::StaticClass(),NULL,TEXT("SDFDataAsset'/Game/Robots/PR2/pr2.pr2"))); //--> Error not in Game Thread...
//            USDFDataAsset* SDFDataAsset= LoadObject<USDFDataAsset>(NULL,*Loc,NULL,LOAD_None,NULL);//Cast<USDFDataAsset>(PossibleSDFDataAsset);
    if(SDFDataAssetL)
    {
        UE_LOG(LogTemp, Log, TEXT("The RobotFactory found some Blueprints to work with... "));
        //We found an SDF Data Asset Spawn
        ARModel* ActortoSpawn= NewObject<ARModel>();
        USDFModel* ModeltoSpawn= SDFDataAssetL->Models[0]; // There should only be one Model in the DataAsset, if not which one should I spawn?
        URModelBuilder* BuildingFacotry= NewObject<URModelBuilder>();
        BuildingFacotry->Load(ModeltoSpawn,ActortoSpawn);
        FVector Position =FVector(1,1,1);
        FRotator Rotation = FRotator(0,0,0);
        FActorSpawnParameters SpawnParams;
        AActor*spawnedActor = World->SpawnActor<AActor>(Position,Rotation,SpawnParams);
        spawnedActor=ActortoSpawn; //Does that work?

        //Want to Use RModelBuilder --> Needs ARModell and
        //void URModelBuilder::Load(USDFModel* InModelDescription, ARModel* OutModel)

    }
    return true;
}
