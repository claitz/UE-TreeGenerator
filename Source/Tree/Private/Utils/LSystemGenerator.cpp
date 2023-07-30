// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/LSystemGenerator.h"

LSystemGenerator::LSystemGenerator()
{
	SetPreset(TreeGeneratorPreset);
	
}

void LSystemGenerator::SetPreset(const ETreePreset& NewPreset)
{
	TreeGeneratorPreset = NewPreset;

	switch (TreeGeneratorPreset)
	{
	case ETreePreset::Tree:
		ApplyPreset("Tree");
		break;
	case ETreePreset::BasicTree:
		ApplyPreset("BasicTree");
		break;
	case ETreePreset::Hilbert:
		ApplyPreset("Hilbert");
		break;
	default:
		break;
	}
}

void LSystemGenerator::GenerateLSystem(FString& OutLSystemDerivation)
{
	UE_LOG(LogTemp, Warning, TEXT("Generating LSystem"));
	UE_LOG(LogTemp, Warning, TEXT("Axiom: %s"), *Axiom);
	UE_LOG(LogTemp, Warning, TEXT("Iterations: %f"), Iterations);

	FString Current = Axiom;
	for (int i = 0; i < Iterations; ++i)
	{
		UE_LOG(LogTemp, Warning, TEXT("Iteration: %d"), i);
		FString Next = "";
		for (int j = 0; j < Current.Len(); ++j)
		{
			UE_LOG(LogTemp, Warning, TEXT("Iteration: %d"), j);
			FString CurrentChar = Current.Mid(j, 1);
			FString* Rule = Rules.Find(CurrentChar);
			if (Rule)
			{
				Next += *Rule;
			}
			else
			{
				Next += CurrentChar;
			}
			Current = Next;
		}
	}
	
	OutLSystemDerivation = Current;
}

void LSystemGenerator::RefreshLSystem()
{
	GenerateLSystem(LSystemDerivation);
}

void LSystemGenerator::ApplyPreset(const FString& PresetName)
{
	if (PresetName == "Tree")
	{
		Axiom = "FA";
		
		Rules.Empty();
		Rules.Add("A", "^FB///B/////B");
		Rules.Add("B", "[^^F//////AL]");
	}
	else if (PresetName == "BasicTree")
	{
		Axiom = "FFFFFA";
		
		Rules.Empty();
		Rules.Add("A", "F[++A][--A]///A");
	}
	else if (PresetName == "Hilbert") {
	    Axiom = "A";
		
		Rules.Empty();
	    Rules.Add("A", "B-F+CFC+F-D&F^D-F+&&CFC+F+B//");
	    Rules.Add("B", "A&F^CFB^F^D^^-F-D^|F^B|FC^F^A//");
	    Rules.Add("C", "|D^|F^B-F+C^F^A&&FA&F^C+F+B^F^D//");
	    Rules.Add("D", "|CFB-F+B|FA&F^A&&FB-F+B|FC//");
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unknown preset name: %s"), *PresetName);
		return;
	}

	RefreshLSystem();
}
