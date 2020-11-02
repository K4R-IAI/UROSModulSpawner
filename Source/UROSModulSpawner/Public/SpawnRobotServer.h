// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSBridgeSrvServer.h"
#include "modul_spawner_msgs/srv/SpawnRobot.h"
//#include "RWCManager.h"
#include "RobotManager.h"
//delete RobotFactory?
#include "SDF/SDFDataAsset.h"

/**
 * This Service should be able to spawn Robot
 */
class FROSSpawnRobotServer: public FROSBridgeSrvServer
{
protected:

    FROSSpawnRobotServer()
    {
    };

    UWorld* World;

    FThreadSafeBool ServiceSuccess;


private:
AActor* SpawnRobotFromAsset(USDFDataAsset* InDataAsset, FString InId);


public:
    //URWCManager* Controller;
    URobotManager* Controller;
    FROSSpawnRobotServer(FString Namespace, FString Name, UWorld* InWorld, URobotManager* InController) :
        FROSBridgeSrvServer(Namespace + TEXT("/") + Name, TEXT("modul_spawner_msgs/SpawnRobot"))
    {
        Controller = InController;
        World = InWorld;
    }

    TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override;

    TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> Request) override;
};
