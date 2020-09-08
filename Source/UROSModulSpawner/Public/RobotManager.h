// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "CoreMinimal.h"
//#include "Engine.h"
#include "GameFramework/Actor.h"
#include "UROSCallbackRegisterBase.h"
#include "RobotManager.generated.h"

/**
 * 
 */
UCLASS()
class UROSMODULSPAWNER_API URobotManager : public UROSCallbackRegisterBase
{
    GENERATED_BODY()
public:
    URobotManager() {}
    ~URobotManager(){}


    virtual void SetupServiceServers();

    virtual void Register(FString DefaultNamespace) override;

private:
    FString Namespace;
};
