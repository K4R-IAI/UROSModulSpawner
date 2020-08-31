// Fill out your copyright notice in the Description page of Project Settings.


#include "RuntimeSDFParser.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/Paths.h"
#include "HAL/FileManagerGeneric.h"
//#include "UObject/ConstructorHelpers.h"
//#include "RStaticMeshEditUtils.h"

FRuntimeSDFParser::FRuntimeSDFParser()
{
    this->XmlFile=nullptr;
    this->bSDFLoaded=false;
}

FRuntimeSDFParser::FRuntimeSDFParser(FString InMessage)
{
    this->XmlFile=nullptr;
    this->bSDFLoaded=false;
    LoadSDF(InMessage);

}

FRuntimeSDFParser::~FRuntimeSDFParser()
{
}



// Load sdf from file
bool FRuntimeSDFParser::LoadSDF(const FString& InFilename)
{
  // Make sure parser is clean
  Clear();

  // Load xml file
  XmlFile = new FXmlFile(InFilename,EConstructMethod::ConstructFromBuffer);

  // Check for valid sdf
  bSDFLoaded = IsValidSDF();

  return bSDFLoaded;
}

void FRuntimeSDFParser::Clear()
{
    if (XmlFile)
      {
        XmlFile->Clear();
        delete XmlFile;
        XmlFile = nullptr;
      }
    if (bSDFLoaded)
      {
        bSDFLoaded = false;
        DataAsset = nullptr;
      }
}

bool FRuntimeSDFParser::IsValidSDF()
{
    if (XmlFile == nullptr)
      {
        return false;
      }

    // Check if root node is <sdf> or <gazebo> (sdf version 1.2)
    if (!XmlFile->GetRootNode()->GetTag().Equals(TEXT("sdf"))
        && !XmlFile->GetRootNode()->GetTag().Equals(TEXT("gazebo")))
      {
        // UE_LOG(LogTemp, Error, TEXT("[%s][%d] Root node is not <sdf> or <gazebo>(sdf version 1.2)"), TEXT(__FUNCTION__), __LINE__);
        return false;
      }
    return true;
}

// Parse <sdf> node
void FRuntimeSDFParser::ParseSDF()
{
  // Get "version" from node attribute
  const FString SDFVersion = XmlFile->GetRootNode()->GetAttribute(TEXT("version"));
  if (!SDFVersion.IsEmpty())
    {
      DataAsset->Version = SDFVersion;
    }
  else
    {
      UE_LOG(LogTemp, Warning, TEXT("[%s][%d] <sdf> has no \"version\" attribute, added a default value.."),
             *FString(__FUNCTION__), __LINE__);
      DataAsset->Version = TEXT("__default__");
    }

  // Iterate <sdf> child nodes
  for (const auto& ChildNode : XmlFile->GetRootNode()->GetChildrenNodes())
    {
      // Check if <model>
      if (ChildNode->GetTag().Equals(TEXT("model")))
        {
          ParseModel(ChildNode);
        }
      else
        {
          UE_LOG(LogTemp, Warning, TEXT("[%s][%d] <sdf> child <%s> not supported, ignored.."),
                 *FString(__FUNCTION__), __LINE__, *ChildNode->GetTag());
          continue;
        }
    }
}




// Parse <model> node
void FRuntimeSDFParser::ParseModel(const FXmlNode* InNode)
{
  // Ptr to the new model
  USDFModel* NewModel = nullptr;

  // Get "name" from node attribute
  const FString Name = InNode->GetAttribute(TEXT("name"));

  if(FPaths::DirectoryExists(Name))
  {
      UObject* OutObject = NewObject<UObject>();
      FindObject<USDFDataAsset>(OutObject,*Name);
      this->DataAsset= Cast<USDFDataAsset>(OutObject);
      if(DataAsset)
      {
          //Cast was successfull we have an SDFDataAsset to work with
     }
  }



  if(!Name.IsEmpty())
    {
      NewModel = NewObject<USDFModel>(DataAsset, FName(*Name));
      NewModel->Name = Name;
    }
  else
    {
      UE_LOG(LogTemp, Warning, TEXT("[%s][%d] <model> has no \"name\" attribute, added a default value.."),
             *FString(__FUNCTION__), __LINE__);
      NewModel = NewObject<USDFModel>(DataAsset/*, FName(TEXT("__default__"))*/);
      NewModel->Name = TEXT("__default__");
    }

  // Iterate <model> child nodes
  for (const auto& ChildNode : InNode->GetChildrenNodes())
    {
      if (ChildNode->GetTag().Equals(TEXT("link")))
        {
          ParseLink(ChildNode, NewModel);
        }
      else if (ChildNode->GetTag().Equals(TEXT("joint")))
        {
          ParseJoint(ChildNode, NewModel);
        }
      else if (ChildNode->GetTag().Equals(TEXT("static")))
        {
          NewModel->bStatic = ChildNode->GetContent().ToBool();
        }
      else if (ChildNode->GetTag().Equals(TEXT("pose")))
        {
          NewModel->Pose = PoseContentToFTransform(ChildNode->GetContent());
        }
      else
        {
          UE_LOG(LogTemp, Warning, TEXT("[%s][%d] <model> child <%s> not supported, ignored.."),
                 *FString(__FUNCTION__), __LINE__, *ChildNode->GetTag());
          continue;
        }
    }

  // Add model to the data asset
  DataAsset->Models.Add(NewModel);
}


// Parse <link> node
void FRuntimeSDFParser::ParseLink(const FXmlNode* InNode, USDFModel*& OutModel)
{
  // Ptr to the new link
  USDFLink* NewLink = nullptr;

  // Get "name" from node attribute
  const FString Name = InNode->GetAttribute(TEXT("name"));
  if (!Name.IsEmpty())
    {
      NewLink = NewObject<USDFLink>(OutModel, FName(*Name));
      NewLink->Name = Name;
      CurrentLinkName = Name;
    }
  else
    {
      UE_LOG(LogTemp, Warning, TEXT("[%s][%d] <link> has no \"name\" attribute, added a default value.."),
             *FString(__FUNCTION__), __LINE__);
      NewLink = NewObject<USDFLink>(OutModel/*, FName(TEXT("__default__"))*/);
      NewLink->Name = TEXT("__default__");
    }

  // Iterate <link> child nodes
  for (const auto& ChildNode : InNode->GetChildrenNodes())
    {
      if (ChildNode->GetTag().Equals(TEXT("pose")))
        {
          NewLink->Pose = PoseContentToFTransform(ChildNode->GetContent());
        }
      else if (ChildNode->GetTag().Equals(TEXT("inertial")))
        {
          ParseLinkInertial(ChildNode, NewLink);
        }
      else if (ChildNode->GetTag().Equals(TEXT("visual")))
        {
          ParseVisual(ChildNode, NewLink);
        }
      else if (ChildNode->GetTag().Equals(TEXT("collision")))
        {
          ParseCollision(ChildNode, NewLink);
        }
      else if (ChildNode->GetTag().Equals(TEXT("self_collide")))
        {
          NewLink->bSelfCollide = ChildNode->GetContent().ToBool();
        }
      else if (ChildNode->GetTag().Equals(TEXT("gravity")))
        {
          NewLink->bGravity = ChildNode->GetContent().ToBool();
        }
      else
        {
          UE_LOG(LogTemp, Warning, TEXT("[%s][%d] <link> child <%s> not supported, ignored.."),
                 *FString(__FUNCTION__), __LINE__, *ChildNode->GetTag());
          continue;
        }
    }
  if(NewLink->Collisions.Num() == 0)
    {
//      USDFCollision* Collision = CreateVirtualCollision(NewLink);
//      if(Collision)
//        {
//          NewLink->Collisions.Add(Collision);
//        }
//      else
//        {
//          UE_LOG(LogTemp, Error, TEXT("Creation of Virtual Link % failed"), *CurrentLinkName);
//        }
    }

  // Add link to the data asset
  OutModel->Links.Add(NewLink);
}








//USDFCollision* FRuntimeSDFParser::CreateVirtualCollision(USDFLink* OutLink)
//{
//  USDFCollision* NewCollision = NewObject<USDFCollision>(OutLink, FName(*CurrentLinkName));
//  NewCollision->Name = CurrentLinkName;
//  NewCollision->Pose = FTransform();
//  NewCollision->Geometry = NewObject<USDFGeometry>(NewCollision);
//  NewCollision->Geometry->Type = ESDFGeometryType::Box;
//  NewCollision->Geometry->Size = FVector(0.5f, 0.5f, 0.5f);
//  //NewCollision->Geometry->Mesh = CreateMesh(ESDFType::Collision, ESDFGeometryType::Box, CurrentLinkName, RStaticMeshUtils::GetGeometryParameter(NewCollision->Geometry));
//  return NewCollision;
//}

//During Runtime we wont create a new Mesh but load it. Maybe we need to rename this
UStaticMesh* FRuntimeSDFParser::LoadMesh(ESDFType InType, FString InName)
{
  // FString Path = "";
  FName MeshName = GenerateMeshName(InType, InName);
  UObject* OutObject = NewObject<UObject>();
  FindObject<UStaticMesh>(OutObject,*MeshName.ToString());
  UStaticMesh* Mesh= Cast<UStaticMesh>(OutObject); //=FAssetModifier::LoadMesh(Params.Name, Params.StartDir)/*load correct Mesh*/;
  return Mesh;
}

FName FRuntimeSDFParser::GenerateMeshName(ESDFType InType, FString InName)
{
  FName MeshName;
  if (InType == ESDFType::Collision)
    {
      MeshName = FName(*(TEXT("SM_") + InName + TEXT("_C")));
    }
  else if (InType == ESDFType::Visual)
    {
      MeshName = FName(*(TEXT("SM_") + InName + TEXT("_V")));
    }
  return MeshName;
}


// Parse <visual> node
void FRuntimeSDFParser::ParseVisual(const FXmlNode* InNode, USDFLink*& OutLink)
{
  // Ptr to the new visual
  USDFVisual* NewVisual = nullptr;

  // Get "name" from node attribute
  const FString Name = InNode->GetAttribute(TEXT("name"));
  if (!Name.IsEmpty())
    {
      NewVisual = NewObject<USDFVisual>(OutLink, FName(*Name));
      NewVisual->Name = Name;
    }
  else
    {
      UE_LOG(LogTemp, Warning, TEXT("[%s][%d] <link> has no \"name\" attribute, added a default value.."),
             *FString(__FUNCTION__), __LINE__);
      NewVisual = NewObject<USDFVisual>(OutLink/*, FName(TEXT("__default__"))*/);
      NewVisual->Name = TEXT("__default__");
    }

  // Iterate <visual> child nodes
  for (const auto& ChildNode : InNode->GetChildrenNodes())
    {
      if (ChildNode->GetTag().Equals(TEXT("pose")))
        {
          NewVisual->Pose = PoseContentToFTransform(ChildNode->GetContent());
        }
      else if (ChildNode->GetTag().Equals(TEXT("geometry")))
        {
          ParseGeometry(ChildNode, NewVisual->Geometry, ESDFType::Visual);
          if(NewVisual->Geometry->Type == ESDFGeometryType::Box ||
             NewVisual->Geometry->Type == ESDFGeometryType::Cylinder ||
             NewVisual->Geometry->Type == ESDFGeometryType::Sphere)
            {
              NewVisual->Geometry->Mesh = LoadMesh(ESDFType::Visual, Name);
            }
        }
      else
        {
          UE_LOG(LogTemp, Warning, TEXT("[%s][%d] <link> <visual> child <%s> not supported, ignored.."),
                 *FString(__FUNCTION__), __LINE__, *ChildNode->GetTag());
          continue;
        }
    }

  // Add visual to array
  OutLink->Visuals.Add(NewVisual);
}

// Parse <collision> node
void FRuntimeSDFParser::ParseCollision(const FXmlNode* InNode, USDFLink*& OutLink)
{
  // Ptr to the new collision
  USDFCollision* NewCollision = nullptr;

  // Get "name" from node attribute
  const FString Name = InNode->GetAttribute(TEXT("name"));
  if (!Name.IsEmpty())
    {
      NewCollision = NewObject<USDFCollision>(OutLink, FName(*Name));
      NewCollision->Name = Name;
    }
  else
    {
      UE_LOG(LogTemp, Warning, TEXT("[%s][%d] <collision> has no \"name\" attribute, added a default value.."),
             *FString(__FUNCTION__), __LINE__);
      NewCollision = NewObject<USDFCollision>(OutLink/*, FName(TEXT("__default__"))*/);
      NewCollision->Name = TEXT("__default__");
    }

  // Iterate <collision> child nodes
  for (const auto& ChildNode : InNode->GetChildrenNodes())
    {
      if (ChildNode->GetTag().Equals(TEXT("pose")))
        {
          NewCollision->Pose = PoseContentToFTransform(ChildNode->GetContent());
        }
      else if (ChildNode->GetTag().Equals(TEXT("geometry")))
        {
          ParseGeometry(ChildNode, NewCollision->Geometry, ESDFType::Collision);
          if(NewCollision->Geometry->Type == ESDFGeometryType::Box ||
             NewCollision->Geometry->Type == ESDFGeometryType::Cylinder ||
             NewCollision->Geometry->Type == ESDFGeometryType::Sphere)
            {
              NewCollision->Geometry->Mesh = LoadMesh(ESDFType::Collision,Name);//Is this even needed now? Mesh should have Collision Care
              // RStaticMeshUtils::CreateStaticMeshThroughBrush(OutLink,NewCollision);
            }
        }
      else
        {
          UE_LOG(LogTemp, Warning, TEXT("[%s][%d] <inertial> child <%s> not supported, ignored.."),
                 *FString(__FUNCTION__), __LINE__, *ChildNode->GetTag());
          continue;
        }
    }

  // Add collision to array
  OutLink->Collisions.Add(NewCollision);
}


USDFDataAsset* FRuntimeSDFParser::ParseToNewDataAsset(UObject* InParent, FName InName, EObjectFlags InFlags)
{
    if (!bSDFLoaded)
      {
        return nullptr;
      }

    // Create a new SDFDataAsset
    DataAsset = NewObject<USDFDataAsset>(InParent, InName, InFlags);

    // Parse sdf data and fill the data asset
    ParseSDF();

    return DataAsset;
}

void FRuntimeSDFParser::ParseGeometryMesh(const FXmlNode *InNode, USDFGeometry *&OutGeometry, ESDFType Type)
{
    // Set geometry type
    OutGeometry->Type = ESDFGeometryType::Mesh;

    // Iterate <geometry> <mesh> child nodes
    for (const auto& ChildNode : InNode->GetChildrenNodes())
      {
        if (ChildNode->GetTag().Equals(TEXT("uri")))
          {
            // Import mesh, set Uri as the relative path from the asset to the mesh uasset
            OutGeometry->Uri = ChildNode->GetContent();
            OutGeometry->Mesh = LoadMesh(Type, OutGeometry->Uri);
          }
        else
          {
            UE_LOG(LogTemp, Warning, TEXT("[%s][%d] <geometry> <mesh> child <%s> not supported, ignored.."),
                   *FString(__FUNCTION__), __LINE__, *ChildNode->GetTag());
            continue;
          }
      }
}


