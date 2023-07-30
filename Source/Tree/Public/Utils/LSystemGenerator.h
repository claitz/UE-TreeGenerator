// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ETreePreset : uint8
{
	BasicTree UMETA(DisplayName = "BasicTree"),
	Tree UMETA(DisplayName = "Tree"),
	Hilbert UMETA(DisplayName = "Hilbert")
};

class TREE_API LSystemGenerator
{
public:
	LSystemGenerator();
	
	void SetPreset(const ETreePreset& NewPreset);

	void GenerateLSystem(FString& OutLSystemDerivation);
	
	FString GetLSystemDerivation() const { return LSystemDerivation; }

protected:
	ETreePreset TreeGeneratorPreset = ETreePreset::BasicTree;
	
	FString Axiom;

	TMap<FString, FString> Rules;

	float Iterations = 14.f;
	
	FString LSystemDerivation;
	
	void RefreshLSystem();

	void ApplyPreset(const FString& PresetName);
};
