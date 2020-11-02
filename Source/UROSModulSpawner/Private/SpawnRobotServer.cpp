// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnRobotServer.h"
#include "RuntimeSDFParser.h"
//#include "SDFParser.h"
#include "WorldControlGameInstance.h"
#include "XmlFile.h"
#include "HAL/FileManagerGeneric.h"
//#include "URModelBuilder.h"
//#include "URoboSim/Classes/SDF/SDFDataAsset.h"
#include "Ids.h"
//delete RobotFactory?
#include "Physics/RModel.h"
#include "Factory/RModelBuilder.h"
#include "Tags.h"

TSharedPtr<FROSBridgeSrv::SrvRequest> FROSSpawnRobotServer::FromJson(TSharedPtr<FJsonObject> JsonObject) const
{
    TSharedPtr<FROSRobotModelSrv::Request> Request =
        MakeShareable(new FROSRobotModelSrv::Request());
    Request->FromJson(JsonObject);
    return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);
}


//We are first looking for the [RobotName] in Content/Robot if we find it we will look if there is a [RobotName] DataAsset and if not we will try to parse the SDF File given during the
//runtime to create an DataAsset, which is always needed to spawn a Robot. Careful even with the RuntimeParser it is still neccesarry to have all mehes already present in the
// Content/Robots/[RobotName] folder
TSharedPtr<FROSBridgeSrv::SrvResponse> FROSSpawnRobotServer::Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> Request)
{
    UE_LOG(LogTemp, Log, TEXT("SpawnRobotServer recieved a Message. Time to see what it is."));
    TSharedPtr<FROSRobotModelSrv::Request> SpawnRobotRequest =
        StaticCastSharedPtr<FROSRobotModelSrv::Request>(Request);
    if(!SpawnRobotRequest)
    {
        UE_LOG(LogTemp, Log, TEXT("SpawnRobotServer recieved a Message. But in the wrong format. Aborting... "));
        return nullptr;
    }

    const FString& InFilename = SpawnRobotRequest->GetName();

    FString Id=SpawnRobotRequest->GetId().IsEmpty() ? FIds::NewGuidInBase64() : SpawnRobotRequest->GetId();

    FXmlFile* XmlFile= new FXmlFile(InFilename,EConstructMethod::ConstructFromBuffer);
    //Root Node is Version then it should be model
//    check(XmlFile->IsValid())
    if(!XmlFile->IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] Send XML File was not valid."),*FString(__FUNCTION__));
        return MakeShareable<FROSBridgeSrv::SrvResponse>(new FROSRobotModelSrv::Response(TEXT("NONE"),TEXT("NONE"),false));
    }

    AActor* spawnedActor;
    FString Modelname = XmlFile->GetRootNode()->FindChildNode("model")->GetAttribute("name");
    FString Filename=Modelname+ ".uasset";


    FFileManagerGeneric Fm;
    TArray<FString> FileLocations;

    Fm.FindFilesRecursive(FileLocations, *FPaths::ProjectContentDir().Append("Robots"), *Filename, true, false, true);

    if (FileLocations.Num() == 0)
    {
        //Could not find DataAsset
        UE_LOG(LogTemp, Warning, TEXT("[%s]: Could not find the DataAsset at %s. Trying with runtimeParser"), *FString(__FUNCTION__),*FPaths::ProjectContentDir().Append("Robots"));
        //Need to Parse SDF to get infos to create runtime dataAsset --> Here we can assume Meshes are already there
        //Try the runtime Parser
        if(Fm.DirectoryExists(*FPaths::ProjectContentDir().Append("Robots/"+Modelname)))
        {
            FRuntimeSDFParser* RuntimeParser = new FRuntimeSDFParser(InFilename);
            RuntimeParser->LoadSDF(SpawnRobotRequest->GetName());
            USDFDataAsset* ToSpawnDataAsset= RuntimeParser->ParseToNewDataAsset();

            if(ToSpawnDataAsset)
            {
                int noncount=0;
                //About the Data Asset prints etc
                UE_LOG(LogTemp, Log, TEXT("[%s]: Number of Models %d"), *FString(__FUNCTION__),ToSpawnDataAsset->Models.Num());
                for(int ModelNum=0;ModelNum<ToSpawnDataAsset->Models.Num();ModelNum++)
                {
                    UE_LOG(LogTemp, Log, TEXT("[%s]: Model Number %d, Links %d"), *FString(__FUNCTION__),ModelNum,ToSpawnDataAsset->Models[ModelNum]->Links.Num());
                    for(int LinkNum=0;LinkNum<ToSpawnDataAsset->Models[ModelNum]->Links.Num();LinkNum++)
                    {
    //                    UE_LOG(LogTemp, Log, TEXT("[%s]: Model Number %d, Links %s has %f Mass (from Interial)"),*FString(__FUNCTION__),ModelNum,*(ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Name),ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Inertial->Mass);
                        UE_LOG(LogTemp, Log, TEXT("[%s]: Model Number %d, Links %s has %d Visuals and %d Collisions "),*FString(__FUNCTION__),ModelNum,*(ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Name),ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Visuals.Num(),ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Collisions.Num());
                        for(int VisualNum=0;VisualNum<ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Visuals.Num();VisualNum++)
                        {
                            if(ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Visuals.IsValidIndex(VisualNum))
                            {
                               auto VisualMeshPath= ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Visuals[VisualNum]->Geometry->Mesh->GetPathName();
                               if(VisualMeshPath=="None")
                                   noncount++;

    //                           UE_LOG(LogTemp, Log, TEXT("[%s]: Model Number %d, Link %s has the following MeshPath %s"),*FString(__FUNCTION__),ModelNum,*(ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Name),*VisualMeshPath);

                            }
                        }
                        UE_LOG(LogTemp, Log, TEXT("[%s]: numbers of none %d"),*FString(__FUNCTION__),noncount); // It should be noted that there are some SDF Files where it is possible to have a None and still be able to spawn them

                    }//End For Links
                }//End for Models


                //Execute on game Thread
                double start=FPlatformTime::Seconds();
                FGraphEventRef Task=FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
                {
                    spawnedActor=this->SpawnRobotFromAsset(ToSpawnDataAsset,Id);
                    if(spawnedActor)
                    {ServiceSuccess=true;}
                    else
                    {ServiceSuccess=false;}
                },TStatId(),nullptr,ENamedThreads::GameThread);

                //wait for code above to complete (Spawning in GameThread)
                FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
                double end= FPlatformTime::Seconds();
                UE_LOG(LogTemp, Display, TEXT("SpawnRobot executed in %f seconds."), end-start);
                UE_LOG(LogTemp, Display, TEXT("SpawnRobot has the Name %s"), *spawnedActor->GetName());
                UE_LOG(LogTemp,Display,TEXT("Spawned Robot has the ID %s"),*FString::FromInt(spawnedActor->GetUniqueID()));
                return MakeShareable<FROSBridgeSrv::SrvResponse>(new FROSRobotModelSrv::Response(Id,spawnedActor->GetName(),ServiceSuccess));
            }
        }//end if Content/Robot/[ModelName] exsits
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[%s]: Could not find the folder %s Aborting... "),*FString(__FUNCTION__), *FPaths::ProjectContentDir().Append("Robots/"+Modelname));
            return MakeShareable<FROSBridgeSrv::SrvResponse>(new FROSRobotModelSrv::Response(TEXT("NONE"),TEXT("NONE"),false));
        }

    }//end if FileLocation==0
    else
    {
        //Star Spawn from DataAsset
        for (FString Loc : FileLocations)
        {
            if(!Loc.StartsWith(FPaths::ProjectContentDir().Append("Robots/"+Modelname)))
            {
                UE_LOG(LogTemp, Warning, TEXT("[%s]: The found DataAsset is not in the %s folder"), *FString(__FUNCTION__), *Modelname);
            }
            UE_LOG(LogTemp, Log, TEXT("[%s]: We found a DataAsset at %s. Trying to spawn it..."), *FString(__FUNCTION__), *Loc);
            Loc.RemoveFromStart(FPaths::ProjectContentDir());
            int Last;
            Loc.FindLastChar('.', Last);
            Loc.RemoveAt(Last, Loc.Len() - Last);

            FString PathtoDataAsset=TEXT("SDFDataAsset'/Game/" + Loc +"."+Modelname+ "'");
            FString help="/Game/" + Loc ;
            //Execute on game Thread
            double start=FPlatformTime::Seconds();
            FGraphEventRef Task=FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
            {
                USDFDataAsset* SDFDataAssetS=Cast<USDFDataAsset>(StaticLoadObject(USDFDataAsset::StaticClass(),NULL,*PathtoDataAsset));
                spawnedActor=this->SpawnRobotFromAsset(SDFDataAssetS,Id);
                if(spawnedActor)
                {ServiceSuccess=true;}
                else
                {ServiceSuccess=false;}
            },TStatId(),nullptr,ENamedThreads::GameThread);

            //wait for code above to complete (Spawning in GameThread)
            FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
            double end= FPlatformTime::Seconds();
            UE_LOG(LogTemp, Display, TEXT("SpawnRobot executed in %f seconds."), end-start);
            UE_LOG(LogTemp, Display, TEXT("SpawnRobot has the Name %s"), *spawnedActor->GetName());
            UE_LOG(LogTemp,Display,TEXT("Spawned Robot has the ID %s"),*FString::FromInt(spawnedActor->GetUniqueID()));


            return MakeShareable<FROSBridgeSrv::SrvResponse>(new FROSRobotModelSrv::Response(Id,spawnedActor->GetName(),ServiceSuccess));
         }//End For (Possible DataAssets)
     }//End else (Spawn from DataAsset)
    return MakeShareable<FROSBridgeSrv::SrvResponse>(new FROSRobotModelSrv::Response(TEXT("NONE"),TEXT("NONE"),false));
}

AActor* FROSSpawnRobotServer::SpawnRobotFromAsset(USDFDataAsset* InDataAsset, FString InId)
{
    UE_LOG(LogTemp, Log, TEXT("[%s] RobotFactory starts to work... "),*FString(__FUNCTION__));
    USDFDataAsset* AssetToSpawn=InDataAsset;
    FString Id=InId;

    ARModel* ActortoSpawn= NewObject<ARModel>();
    USDFModel* ModeltoSpawn= AssetToSpawn->Models[0]; // There should only be one Model in the DataAsset, if not which one should I spawn?
    FVector Position =FVector(100,-100,20); //This does not work the robot always spawns at (0|0|0)
    FRotator Rotation = FRotator(0,0,0);
    FActorSpawnParameters SpawnParams;
    FString RobotName= TEXT("Robot")+FGuid::NewGuid().ToString();
    SpawnParams.Name=*RobotName;
    ActortoSpawn = World->SpawnActor<ARModel>(Position,Rotation,SpawnParams);
//    UE_LOG(LogTemp, Log, TEXT("[%s] ActortoSpawnLocation after SpawnActor: [%f,%f,%f] "),*FString(__FUNCTION__),ActortoSpawn->GetActorLocation().X,ActortoSpawn->GetActorLocation().Y,ActortoSpawn->GetActorLocation().Z);
    ActortoSpawn->SetRootComponent(nullptr);
//    UE_LOG(LogTemp, Log, TEXT("[%s] ActortoSpawnLocation after rootComponent=NULL: [%f,%f,%f] "),*FString(__FUNCTION__),ActortoSpawn->GetActorLocation().X,ActortoSpawn->GetActorLocation().Y,ActortoSpawn->GetActorLocation().Z);
    URModelBuilder* BuildingFacotry= NewObject<URModelBuilder>();
    BuildingFacotry->Load(ModeltoSpawn,ActortoSpawn);
//    UE_LOG(LogTemp, Log, TEXT("[%s] ActortoSpawnLocation after URModelBuilder->Load: [%f,%f,%f] "),*FString(__FUNCTION__),ActortoSpawn->GetActorLocation().X,ActortoSpawn->GetActorLocation().Y,ActortoSpawn->GetActorLocation().Z);

    FTags::AddKeyValuePair(
        ActortoSpawn,
        TEXT("SemLog"),
        TEXT("id"),
        Id);


    return ActortoSpawn;
}
