#include "Actors/TreeActor.h"
#include "Components/InstancedStaticMeshComponent.h"

FCriticalSection InstancesMutex;

ATreeActor::ATreeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	InstancedTreeComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedTreeComponent"));
	InstancedTreeComponent->SetupAttachment(GetRootComponent());

	InstancedLeafComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedLeafComponent"));
	InstancedLeafComponent->SetupAttachment(GetRootComponent());

}

void ATreeActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!InstancedTreeComponent->GetStaticMesh())
	{
		InstancedTreeComponent->SetStaticMesh(TreeStaticMesh);
		InstancedTreeComponent->SetMaterial(0, UMaterialInstanceDynamic::Create(TreeMaterial, this));
	}


	InstancedTreeComponent->BodyInstance.SetObjectType(ECC_WorldDynamic);
	InstancedTreeComponent->SetMobility(EComponentMobility::Movable);

	if (!InstancedLeafComponent->GetStaticMesh())
	{
		InstancedLeafComponent->SetStaticMesh(LeafStaticMesh);
		InstancedLeafComponent->SetMaterial(0, UMaterialInstanceDynamic::Create(LeafMaterial, this));
	}

	InstancedLeafComponent->BodyInstance.SetObjectType(ECC_WorldDynamic);
	InstancedLeafComponent->SetMobility(EComponentMobility::Movable);
}

void ATreeActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ATreeActor::BeginPlay()
{
	Super::BeginPlay();
	
	GenerateTree();
}

void ATreeActor::InterpretLSystemDerivation()
{
	// F[\\&&F^^F][//&&F^^F]
	// F[\\++F--F][//++F--F]
	// F//++F//++F//++F//++F

	TArray<FTransform> Stack;
	TArray<FQuat> RotatorStack;

	FTransform Base = FTransform(FRotator::ZeroRotator, FVector::ZeroVector,
	                             FVector(Radius / 100, Radius / 100, Length / 100));
	FTransform Relative = FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::OneVector);

	FTransform Forward = FTransform(FRotator::ZeroRotator, FVector(0, 0, 100), FVector::ZeroVector);

	FQuat PitchCw = FQuat(FRotator(Angle, 0, 0));
	FQuat PitchCCW = FQuat(FRotator(-Angle, 0, 0));

	FQuat RollCw = FQuat(FRotator(0, Angle, 0));
	FQuat RollCCW = FQuat(FRotator(0, -Angle, 0));

	FQuat YawCw = FQuat(FRotator(0, 0, Angle));
	FQuat YawCCW = FQuat(FRotator(0, 0, -Angle));

	FQuat Reverse = FQuat(FRotator(180, 0, 0)) * FQuat(FRotator(0, 180, 0));

	FTransform CurrentGlobal = Base;
	FQuat CurrentRelativeRotator = FQuat::Identity;

	FTransform NextRelative = Relative;

	for (TCHAR c : LSystemDerivation)
	{
		switch (c)
		{
		case 'F':
			{
				float BranchXY = FMath::Clamp(CurrentGlobal.GetScale3D().X - (BranchRadiusReduction / 100),
				                              MinBranchRadius / 100, Radius / 100);
				float BranchZ = CurrentGlobal.GetScale3D().Z * BranchLengthRelativeToParent;
				NextRelative.SetScale3D(FVector(BranchXY, BranchXY, BranchZ));
				NextRelative.SetRotation(CurrentRelativeRotator);

				FTransform NextGlobal = TransformToGlobal(CurrentGlobal, NextRelative);
				// InstancedTreeComponent->AddInstance(NextGlobal);

				FInstanceData InstanceData = CreateInstanceData(NextGlobal, EIT_Trunk);
				FScopeLock ScopeLock(&InstancesMutex);
				InstancesToAdd.Add(InstanceData); 

				CurrentGlobal = NextGlobal;
				CurrentRelativeRotator = FQuat::Identity;
				NextRelative = Relative;
				NextRelative.AddToTranslation(Forward.GetLocation());
			}
			break;
		case 'L':
			{
				NextRelative.SetScale3D(FVector(LeafSize / 100, LeafSize / 100, LeafSize / 100));
				NextRelative.SetRotation(CurrentRelativeRotator);

				FTransform NextGlobal = TransformToGlobal(CurrentGlobal, NextRelative);
				
				// InstancedLeafComponent->AddInstance(NextGlobal);
				
				FInstanceData InstanceData = CreateInstanceData(NextGlobal, EIT_Leaf);
				FScopeLock ScopeLock(&InstancesMutex);
				InstancesToAdd.Add(InstanceData);
				
				CurrentGlobal = NextGlobal;
				CurrentRelativeRotator = FQuat::Identity;
				NextRelative = Relative;
			}
			break;
		case '[':
			Stack.Push(CurrentGlobal);
			RotatorStack.Push(CurrentRelativeRotator);
			break;
		case ']':
			CurrentGlobal = Stack.Pop();
			CurrentRelativeRotator = RotatorStack.Pop();
			break;
		case '&':
			CurrentRelativeRotator = CurrentRelativeRotator * PitchCw;
			break;
		case '^':
			CurrentRelativeRotator = CurrentRelativeRotator * PitchCCW;
			break;
		case '+':
			CurrentRelativeRotator = CurrentRelativeRotator * YawCw;
			break;
		case '-':
			CurrentRelativeRotator = CurrentRelativeRotator * YawCCW;
			break;
		case '\\':
			CurrentRelativeRotator = CurrentRelativeRotator * RollCw;
			break;
		case '/':
			CurrentRelativeRotator = CurrentRelativeRotator * RollCCW;
			break;
		case '|':
			CurrentRelativeRotator = CurrentRelativeRotator * Reverse;
			break;
		default:
			break;
		}
	}
}

void ATreeActor::ProcessChunk()
{
	int32 Limit = FMath::Min(CurrentChunk + ChunkSize, InstancesToAdd.Num());
	
	for (; CurrentChunk < Limit; CurrentChunk++)
	{
		if (InstancesToAdd[CurrentChunk].InstanceType == EIT_Trunk)
		{
			InstancedTreeComponent->AddInstance(InstancesToAdd[CurrentChunk].Transform);
		}
		else if (InstancesToAdd[CurrentChunk].InstanceType == EIT_Leaf)
		{
			InstancedLeafComponent->AddInstance(InstancesToAdd[CurrentChunk].Transform);
		}
	}
	
	if (CurrentChunk < InstancesToAdd.Num())
	{
		// Schedule the next chunk
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ATreeActor::ProcessChunk, 0.1f, false);
	} else
	{
		// We're done
		CurrentChunk = 0;
		FScopeLock ScopeLock(&InstancesMutex);
		InstancesToAdd.Empty();
	}
	
}

FTransform ATreeActor::TransformToGlobal(const FTransform& Parent, const FTransform& Local)
{
	FTransform Result = FTransform(Local);

	Result.SetRotation(Parent.GetRotation() * Local.GetRotation());
	Result.SetTranslation(
		Parent.GetTranslation() + Parent.GetRotation().RotateVector(Parent.GetScale3D() * Local.GetTranslation()));
	Result.SetScale3D(/*parent.GetScale3D() **/ Local.GetScale3D());
	return Result;
}

void ATreeActor::SetLSystemDerivation(const FString& NewLSystemDerivation)
{
	LSystemDerivation = NewLSystemDerivation;
	UE_LOG(LogTemp, Warning, TEXT("New  LSystemDerivation: %s"), *LSystemDerivation);
	RefreshTree();
}

void ATreeActor::SetAngle(float NewAngle)
{
	Angle = NewAngle;
	RefreshTree();
}

void ATreeActor::SetRadius(float NewRadius)
{
	Radius = NewRadius;
	RefreshTree();
}

void ATreeActor::SetLength(float NewLength)
{
	Length = NewLength;
	RefreshTree();
}

void ATreeActor::SetBranchRadiusReduction(float NewBranchRadiusReduction)
{
	BranchRadiusReduction = NewBranchRadiusReduction;
	RefreshTree();
}

void ATreeActor::SetMinBranchRadius(float NewMinBranchRadius)
{
	MinBranchRadius = NewMinBranchRadius;
	RefreshTree();
}

void ATreeActor::SetBranchLengthRelativeToParent(float NewBranchLengthRelativeToParent)
{
	BranchLengthRelativeToParent = NewBranchLengthRelativeToParent;
	RefreshTree();
}

void ATreeActor::SetLeafSize(float NewLeafSize)
{
	LeafSize = NewLeafSize;
	RefreshTree();
}

void ATreeActor::GenerateTree()
{
	InterpretLSystemDerivation();

	// Start processing in chunks
	ProcessChunk();
}

void ATreeActor::RefreshTree()
{
	InstancedTreeComponent->ClearInstances();
	InstancedLeafComponent->ClearInstances();
	GenerateTree();
}

FInstanceData ATreeActor::CreateInstanceData(const FTransform& Transform, TEnumAsByte<EInstanceType> InstanceType)
{
	FInstanceData InstanceData;
	InstanceData.Transform = Transform;
	InstanceData.InstanceType = InstanceType;
	return InstanceData;
}
