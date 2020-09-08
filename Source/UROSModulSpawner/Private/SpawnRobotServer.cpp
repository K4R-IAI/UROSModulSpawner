// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnRobotServer.h"
//#include "SDFParser.h"
#include "WorldControlGameInstance.h"
#include "RobotFactory.h"
#include "XmlFile.h"
#include "HAL/FileManagerGeneric.h"
//#include "URModelBuilder.h"
//#include "URoboSim/Classes/SDF/SDFDataAsset.h"




TSharedPtr<FROSBridgeSrv::SrvRequest> FROSSpawnRobotServer::FromJson(TSharedPtr<FJsonObject> JsonObject) const
{
    TSharedPtr<FROSRobotModelSrv::Request> Request =
        MakeShareable(new FROSRobotModelSrv::Request());
    Request->FromJson(JsonObject);
    return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);
}



TSharedPtr<FROSBridgeSrv::SrvResponse> FROSSpawnRobotServer::Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> Request)
{
    UE_LOG(LogTemp, Log, TEXT("SpawnRobotServer recieved a Message. Time to see what it is"));
    TSharedPtr<FROSRobotModelSrv::Request> SpawnRobotRequest =
        StaticCastSharedPtr<FROSRobotModelSrv::Request>(Request);
    if(!SpawnRobotRequest)
    {
        UE_LOG(LogTemp, Log, TEXT("SpawnRobotServer recieved a Message. But in the wrong format. Aborting... "));
        return nullptr;
    }
    /*ARobotFactory* RobotFactory = NewObject<ARobotFactory>();
    RobotFactory->SpawnRobot(SpawnRobotRequest->GetName(),World);*/

    const FString& InFilename = SpawnRobotRequest->GetName();
    FXmlFile* XmlFile= new FXmlFile(InFilename,EConstructMethod::ConstructFromBuffer);
    //Root Node is Version then it should be model
    check(XmlFile->IsValid())

    FString Modelname = XmlFile->GetRootNode()->FindChildNode("model")->GetAttribute("name");
    FString Filename=Modelname+ ".uasset";


    FFileManagerGeneric Fm;
    TArray<FString> FileLocations;
    UE_LOG(LogTemp, Warning, TEXT("FindFileRecursive Params ([],%s,%s,true,false,true)"), *FPaths::ProjectContentDir().Append("Content/Robots"), *Filename);

    Fm.FindFilesRecursive(FileLocations, *FPaths::ProjectContentDir().Append("Robots"), *Filename, true, false, true);

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
            UE_LOG(LogTemp, Warning, TEXT("[%s]: We found an interesting location %s"), *FString(__FUNCTION__), *Loc);
            Loc.RemoveFromStart(FPaths::ProjectContentDir());
            int Last;
            Loc.FindLastChar('.', Last);
            Loc.RemoveAt(Last, Loc.Len() - Last);
//            UE_LOG(LogTemp, Warning, TEXT("[%s]: Loc before we add Modulname after cutting %s"), *FString(__FUNCTION__),*Loc);
//            FString FoundPath= Loc.Append(Modelname);
//            UE_LOG(LogTemp, Warning, TEXT("[%s]: Loc after we add Modulname after cutting %s"), *FString(__FUNCTION__),*Loc);


            UE_LOG(LogTemp, Warning, TEXT("[%s]: We interesting location short %s"), *FString(__FUNCTION__),*Loc);
            FString Valeera=TEXT("SDFDataAsset'/Game/" + Loc +"."+Modelname+ "'");
            UE_LOG(LogTemp, Warning, TEXT("[%s]: Closes to AssetModifier %s"), *FString(__FUNCTION__),*Valeera);
            UE_LOG(LogTemp, Warning, TEXT("[%s]: Should be SDFDataAsset'/Game/Robots/PR2/pr2.pr2'"), *FString(__FUNCTION__));
//            UObject* PossibleSDFDataAsset= FoundPath.TryLoad();
            FString help="/Game/" + Loc ;
            //Star Spawn from DataAsset from the
            ARobotFactory::FSpawnRobotParams Params;
            Params.World=World;
            Params.DataAssetToSpawn=Valeera;
            ARobotFactory* RobotFactory = NewObject<ARobotFactory>();
            //Execute on game Thread
            double start=FPlatformTime::Seconds();
            FGraphEventRef Task=FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
            {
                ServiceSuccess = RobotFactory->SpawnRobotFromAsset(Params);
            },TStatId(),nullptr,ENamedThreads::GameThread);

            //wait for code above to complete (Spawning in GameThread)
            FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
            double end= FPlatformTime::Seconds();
            UE_LOG(LogTemp, Display, TEXT("SpawnRobot executed in %f seconds."), end-start);
            MakeShareable<FROSBridgeSrv::SrvResponse>(new FROSRobotModelSrv::Response(Id,FinalActorName,ServiceSuccess);


         }//End For (Possible DataAssets)
     }//End else (Spawn from DataAsset)

    return MakeShareable<FROSBridgeSrv::SrvResponse>(new FROSRobotModelSrv::Response(/*Id,FinalActorName,ServiceSuccess*/));
}





//

//    TSharedPtr<FROSRobotModelSrv::Request> SpawnRobotRequest=StaticCastSharedPtr<FROSRobotModelSrv::Request>(Request);
//    //NO check if we check and it fails the whole system gets shoot down


//    //start spawning the robot
//    //get USDFModel
//    //USDFParser* parser = NewObject<USDFParser>();
//    //parser->LoadSDF(TEXT("/home/nleusmann/Documents/Sandbox/UE4_Environment/URoboSimExampleRobots/PR2/model.sdf"));
//    //FSDFParser* parser =new FSDFParser(TEXT("/home/nleusmann/Documents/Sandbox/UE4_Environment/URoboSimExampleRobots/PR2/model.sdf")); // NewObject<FSDFParser>(TEXT("/home/nleusmann/Documents/Sandbox/UE4_Environment/URoboSimExampleRobots/PR2/model.sdf"));
//    UE_LOG(LogTemp, Log, TEXT("Loaded SDF-File"));
//    //EObjectFlags flags = RF_Transactional; // In USDFDataAssetFactory --> Flags |= RF_Transactional;

//    //USDFDataAsset* dataAsset = parser->ParseToNewDataAsset(GetTransientPackage(),TEXT("PR2DataAsset"), flags);

//    UE_LOG(LogTemp, Log, TEXT("We have an dataAsset File"));
//    //Create ARModelActor
//    //plug everthing into the URMODELBuilder
//    //URModelBuilder ModelBuilder= NewObject<URModelBuilder>();



