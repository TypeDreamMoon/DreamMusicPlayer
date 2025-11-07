#pragma once

#include "CoreMinimal.h"
#include "DreamLyricParser/Parser_Interface.hpp"
#include "DreamLyricParser/Types.hpp"
#include "UObject/NoExportTypes.h"
#include "Factories/Factory.h"
#include "LyricAssetFactory.generated.h"

UCLASS()
class DREAMMUSICPLAYERLYRIC_API ULyricAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	ULyricAssetFactory(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual bool FactoryCanImport(const FString& Filename) override;

private:
	bool ImportLyricFile(const FString& Filename, ULyricAsset* Asset, dream_lyric_parser::FParserFormat Format);
	void ConvertParsedLyricToAsset(const dream_lyric_parser::FParsedLyric& ParsedLyric, ULyricAsset* Asset);
};

