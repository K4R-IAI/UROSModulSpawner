// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "SDF/SDFDataAsset.h"
//#include "SDFParser.h"
#include "SDFParserInterface.h"
#include "XmlParser.h"

/**
 *
 */
class UROSMODULSPAWNER_API FRuntimeSDFParser : public ISDFParserInterface
{
public:
    FRuntimeSDFParser();
    FRuntimeSDFParser(FString InMessage);
    ~FRuntimeSDFParser();
    virtual bool LoadSDF(const FString& InFilename) override;
    virtual void Clear() override;
    virtual bool IsValidSDF() override;
    virtual USDFDataAsset* ParseToNewDataAsset(UObject* InParent, FName InName, EObjectFlags InFlags)override;

protected:
    virtual void ParseSDF() override ;
    virtual void ParseModel(const FXmlNode* InNode) override;
    virtual void ParseLink(const FXmlNode* InNode, USDFModel*& OutModel)override;
    virtual void ParseVisual(const FXmlNode* InNode, USDFLink*& OutLink) override;
    virtual void ParseCollision(const FXmlNode* InNode, USDFLink*& OutLink) override;
    virtual void ParseGeometryMesh(const FXmlNode *InNode, USDFGeometry *&OutGeometry, ESDFType Type) override;

   void SpawnRobot(FString InMessage);

//    USDFCollision* CreateVirtualCollision(USDFLink* OutLink);
    UStaticMesh* LoadMesh(ESDFType InType, FString InName);
    FString GeneratePackageName(FName MeshName);
    FName GenerateMeshName(ESDFType InType, FString InName);

    //Variables
    USDFDataAsset* DataAsset; // This could be put into the interface, but there seems to be some error which prevents me from doing it
    FString CurrentLinkName; // This could be put into the interface, but there seems to be some error which prevents me from doing it
};
