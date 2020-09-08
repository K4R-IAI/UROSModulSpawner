// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotManager.h"
#include "SpawnRobotServer.h"



void URobotManager::Register(FString DefaultNamespace)
{
    Namespace = DefaultNamespace;

    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s]: GetWorld returned, NULL."), *FString(__FUNCTION__));
        return;
    }

    SetupServiceServers();
}


void URobotManager::SetupServiceServers()
{
    // Add Service Servers
    UWorld* World = GetWorld();

    ServicesToPublish.Add(MakeShareable<FROSSpawnRobotServer>(new FROSSpawnRobotServer(Namespace,TEXT("spawn_robot"),World,this)));
}
