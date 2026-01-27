#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DreamMusicAssetFactory.generated.h"

class UDreamMusicDataAsset;
struct FDreamMusicTag;

UCLASS()
class DREAMMUSICPLAYERIMPORTER_API UDreamMusicAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDreamMusicAssetFactory();

	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual FText GetDisplayName() const override;

private:
	// Helper to import the actual audio file (creates a USoundWave)
	USoundBase* ImportAudioFile(const FString& InFilename, const FString& InTargetPackagePath, const FString& InAssetName);

	// Helper to create the texture asset from TagLib data
	UTexture2D* CreateCoverArtTexture(const FString& InTargetPackagePath, const FString& InAssetName, const TArray<uint8>& InData, const FString& InMimeType);

	// Helper to apply data to the asset
	// Changed to UObject* to avoid header dependency/parsing issues in header
	void ApplyImportData(UObject* Asset, const FDreamMusicTag& ImportTag, USoundBase* ImportedSound, UTexture2D* ImportedCover);
};
