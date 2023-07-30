#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TreeActor.generated.h"

UENUM(BlueprintType)
enum EInstanceType
{
	EIT_Trunk UMETA(DisplayName = "Trunk"),
	EIT_Leaf UMETA(DisplayName = "Leaf")
};

USTRUCT(BlueprintType)
struct FInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance Data")
	FTransform Transform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance Data")
	TEnumAsByte<EInstanceType> InstanceType;
};

UCLASS()
class TREE_API ATreeActor : public AActor
{
	GENERATED_BODY()

public:
	ATreeActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection|LSystem",
		meta = (BlueprintSetter = "SetLSystemDerivation"))
	FString LSystemDerivation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Selection, meta = (BlueprintSetter = "SetAngle"))
	float Angle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Selection, meta = (BlueprintSetter = "SetRadius"))
	float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Selection, meta = (BlueprintSetter = "SetLength"))
	float Length;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Selection,
		meta = (BlueprintSetter = "SetBranchRadiusReduction"))
	float BranchRadiusReduction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Selection, meta = (BlueprintSetter = "SetMinBranchRadius"))
	float MinBranchRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Selection,
		meta = (BlueprintSetter = "SetBranchLengthRelativeToParent"))
	float BranchLengthRelativeToParent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Selection, meta = (BlueprintSetter = "SetLeafSize"))
	float LeafSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Tree)
	UInstancedStaticMeshComponent* InstancedTreeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tree)
	UMaterial* TreeMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tree)
	UStaticMesh* TreeStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Leaves)
	UInstancedStaticMeshComponent* InstancedLeafComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Leaves)
	UMaterial* LeafMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Leaves)
	UStaticMesh* LeafStaticMesh;

	TArray<FInstanceData> InstancesToAdd;

	int ChunkSize = 100;
	int CurrentChunk = 0;

	void InterpretLSystemDerivation();

	void ProcessChunk();

	UFUNCTION(BlueprintSetter)
	void SetLSystemDerivation(const FString& NewLSystemDerivation);

	UFUNCTION(BlueprintSetter)
	void SetAngle(float NewAngle);

	UFUNCTION(BlueprintSetter)
	void SetRadius(float NewRadius);

	UFUNCTION(BlueprintSetter)
	void SetLength(float NewLength);

	UFUNCTION(BlueprintSetter)
	void SetBranchRadiusReduction(float NewBranchRadiusReduction);

	UFUNCTION(BlueprintSetter)
	void SetMinBranchRadius(float NewMinBranchRadius);

	UFUNCTION(BlueprintSetter)
	void SetBranchLengthRelativeToParent(float NewBranchLengthRelativeToParent);

	UFUNCTION(BlueprintSetter)
	void SetLeafSize(float NewLeafSize);

protected:
	virtual void PostInitializeComponents() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;
	
	void GenerateTree();

	void RefreshTree();

	FInstanceData CreateInstanceData(const FTransform& Transform, TEnumAsByte<EInstanceType> InstanceType);

private:
	FTransform TransformToGlobal(const FTransform& Parent, const FTransform& Local);
};
