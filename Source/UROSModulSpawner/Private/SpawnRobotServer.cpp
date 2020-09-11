// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnRobotServer.h"
#include "RuntimeSDFParser.h"
//#include "SDFParser.h"
#include "WorldControlGameInstance.h"
#include "RobotFactory.h"
#include "XmlFile.h"
#include "HAL/FileManagerGeneric.h"
//#include "URModelBuilder.h"
//#include "URoboSim/Classes/SDF/SDFDataAsset.h"


//for testing
//#include "AssetRegistryModule.h"


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

    Fm.FindFilesRecursive(FileLocations, *FPaths::ProjectContentDir().Append("Robots"), *Filename, true, false, true);
    ARobotFactory* RobotFactory = NewObject<ARobotFactory>();

    if (FileLocations.Num() == 0)
    {
        //Could not find DataAsset
        UE_LOG(LogTemp, Warning, TEXT("[%s]: Could not find the DataAsset. Trying with runtimeParser"), *FString(__FUNCTION__));
        //Need to Parse SDF to get infos to create runtime dataAsset --> Here we can assume Meshes are already there
        //Try the runtime Parser

        FRuntimeSDFParser* RuntimeParser = new FRuntimeSDFParser(InFilename);
        RuntimeParser->LoadSDF(SpawnRobotRequest->GetName());
        USDFDataAsset* ToSpawnDataAsset= RuntimeParser->ParseToNewDataAsset();
        UE_LOG(LogTemp, Error, TEXT("[%s]: NOW WE SHOULD SPAWN THE DATA ASSET WE CREATED"), *FString(__FUNCTION__));

        if(ToSpawnDataAsset)
        {
            //About the Data Asset prints etc
            UE_LOG(LogTemp, Log, TEXT("[%s]: Number of Models %d"), *FString(__FUNCTION__),ToSpawnDataAsset->Models.Num());
            for(int ModelNum=0;ModelNum<ToSpawnDataAsset->Models.Num();ModelNum++)
            {
                UE_LOG(LogTemp, Log, TEXT("[%s]: Model Number %d, Links %d"), *FString(__FUNCTION__),ModelNum,ToSpawnDataAsset->Models[ModelNum]->Links.Num());
                for(int LinkNum=0;LinkNum<ToSpawnDataAsset->Models[ModelNum]->Links.Num();LinkNum++)
                {
                    UE_LOG(LogTemp, Log, TEXT("[%s]: Model Number %d, Links %s has %f Mass (from Interial)"),*FString(__FUNCTION__),ModelNum,*(ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Name),ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Inertial->Mass);
                    UE_LOG(LogTemp, Log, TEXT("[%s]: Model Number %d, Links %s has %d Visuals and %d Collisions "),*FString(__FUNCTION__),ModelNum,*(ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Name),ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Visuals.Num(),ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Collisions.Num());
                    for(int VisualNum=0;VisualNum<ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Visuals.Num();VisualNum++)
                    {
                        if(ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Visuals.IsValidIndex(VisualNum))
                        {
                           auto VisualMeshPath= ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Visuals[VisualNum]->Geometry->Mesh->GetPathName();
                           UE_LOG(LogTemp, Log, TEXT("[%s]: Model Number %d, Link %s has the following MeshPath %s"),*FString(__FUNCTION__),ModelNum,*(ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Name),*VisualMeshPath);

                        }
                    }
//                    for(int CollisionNum=0;ToSpawnDataAsset->Models[ModelNum]->Links[LinkNum]->Collisions.Num();CollisionNum++)
//                    {

//                    }
                }//End For Links
            }//End for Models






            //Spawn with the DataAsset
//            //DEBUG
//            FGraphEventRef Task2=FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
//            {
//                ServiceSuccess = RobotFactory->SpawnRobotFromAsset(World,ToSpawnDataAsset);

//            FString AssetPath = FString("../../../Playground/Content/Dev/");
//            FString PackagePath = FString("/Game/Dev/TestingAsset");
//            UPackage *Package = CreatePackage(nullptr, *PackagePath);
//            USDFDataAsset* TestAsset = NewObject<USDFDataAsset>(Package, USDFDataAsset::StaticClass(), *FString("TestingAsset"), EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
//            TestAsset=ToSpawnDataAsset;



//            },TStatId(),nullptr,ENamedThreads::GameThread);

//            //wait for code above to complete (Spawning in GameThread)
//            FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task2);
//            //END DEBUG


            //Execute on game Thread
            double start=FPlatformTime::Seconds();
            FGraphEventRef Task=FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
            {
                ServiceSuccess = RobotFactory->SpawnRobotFromAsset(World,ToSpawnDataAsset);
            },TStatId(),nullptr,ENamedThreads::GameThread);

            //wait for code above to complete (Spawning in GameThread)
            FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
            double end= FPlatformTime::Seconds();
            UE_LOG(LogTemp, Display, TEXT("SpawnRobot executed in %f seconds."), end-start);
            return MakeShareable<FROSBridgeSrv::SrvResponse>(new FROSRobotModelSrv::Response(/*Id,FinalActorName,ServiceSuccess*/));
        }

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
            FString PathtoDataAsset=TEXT("SDFDataAsset'/Game/" + Loc +"."+Modelname+ "'");
            UE_LOG(LogTemp, Warning, TEXT("[%s]: Closes to AssetModifier %s"), *FString(__FUNCTION__),*PathtoDataAsset);
            UE_LOG(LogTemp, Warning, TEXT("[%s]: Should be SDFDataAsset'/Game/Robots/PR2/pr2.pr2'"), *FString(__FUNCTION__));
//            UObject* PossibleSDFDataAsset= FoundPath.TryLoad();
            FString help="/Game/" + Loc ;
            //Star Spawn from DataAsset from the
            //Execute on game Thread
            double start=FPlatformTime::Seconds();
            FGraphEventRef Task=FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
            {
                ServiceSuccess = RobotFactory->SpawnRobotFromAsset(World,PathtoDataAsset);
            },TStatId(),nullptr,ENamedThreads::GameThread);

            //wait for code above to complete (Spawning in GameThread)
            FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
            double end= FPlatformTime::Seconds();
            UE_LOG(LogTemp, Display, TEXT("SpawnRobot executed in %f seconds."), end-start);
            //MakeShareable<FROSBridgeSrv::SrvResponse>(new FROSRobotModelSrv::Response(TEXT("Silvermoon")),TEXT("FinalActorName"),ServiceSuccess);//TodO get final Actor Name
            return MakeShareable<FROSBridgeSrv::SrvResponse>(new FROSRobotModelSrv::Response(/*Id,FinalActorName,ServiceSuccess*/));


         }//End For (Possible DataAssets)
        UE_LOG(LogTemp, Display, TEXT("It is done"));
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



