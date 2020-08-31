// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotFactory.h"
#include "XmlFile.h"
#include "HAL/FileManagerGeneric.h"
#include "SDF/SDFDataAsset.h"
#include "Physics/RModel.h"
#include "Factory/RModelBuilder.h"


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

void ARobotFactory::SpawnRobot(FString* InMessage,UWorld World)
{
    const FString& InFilename = *InMessage;
    FXmlFile* XmlFile= new FXmlFile(InFilename,EConstructMethod::ConstructFromBuffer);
    //Root Node is Version then it should be model
    FString Modelname = XmlFile->GetRootNode()->FindChildNode("model")->GetAttribute("name");
    Modelname=Modelname.Append(".uasset");

    FFileManagerGeneric Fm;
    TArray<FString> FileLocations;
    Fm.FindFilesRecursive(FileLocations, *FPaths::ProjectContentDir(), *Modelname, true, false, true);
    if (FileLocations.Num() == 0)
    {
        //Could not find DataAsset
        UE_LOG(LogTemp, Warning, TEXT("[%s]: Could not find the DataAsset. Trying with runtimeParser"), *FString(__FUNCTION__));
        //Need to Parse SDF to get infos to create runtime dataAsset --> Here we can assume Meshes are already there
    }
    else
    {
        for (FString Loc : FileLocations)
        {
            Loc.RemoveFromStart(FPaths::ProjectContentDir());
            int Last;
            Loc.FindLastChar('.', Last);
            Loc.RemoveAt(Last, Loc.Len() - Last);
            FStringAssetReference FoundPath= Loc.Append(Modelname);
            UObject* PossibleSDFDataAsset= FoundPath.TryLoad();
            USDFDataAsset* SDFDataAsset= Cast<USDFDataAsset>(PossibleSDFDataAsset);
            if(SDFDataAsset)
            {
                //We found an SDF Data Asset Spawn
                ARModel* ActortoSpawn= NewObject<ARModel>();
                USDFModel* ModeltoSpawn= SDFDataAsset->Models[0]; // There should only be one Model in the DataAsset, if not which one should I spawn?
                URModelBuilder* BuildingFacotry= NewObject<URModelBuilder>();
                BuildingFacotry->Load(ModeltoSpawn,ActortoSpawn);
                FVector Position =FVector(1,1,1);
                FRotator Rotation = FRotator(0,0,0);
                FActorSpawnParameters SpawnParams;
                AActor*spawnedActor = World.SpawnActor<AActor>(Position,Rotation,SpawnParams);
                spawnedActor=ActortoSpawn; //Does that work?

                //Want to Use RModelBuilder --> Needs ARModell and
                //void URModelBuilder::Load(USDFModel* InModelDescription, ARModel* OutModel)

            }
         }//End For (Possible DataAssets)
     }//End else (Spawn from DataAsset)

 }//End Spawn Robot
