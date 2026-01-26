#pragma once

#include "CoreMinimal.h"
#include "DreamLyricParserRuntimeBlueprint.h"
#include "dlp/File.hpp"
#include "Factories/Factory.h"
#include "DreamLyricAssetFactory.generated.h"

class UDreamLyricAsset;

namespace dlp::Parser
{
	class FParserOptions;
}

UCLASS()
class DREAMMUSICPLAYERLYRIC_API ULyricAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	ULyricAssetFactory(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual FText GetDisplayName() const override;

private:
	bool ImportLyricFile(
		const FString& Filename,
		UDreamLyricAsset* Asset,
		dlp::EFileFormat Format,
		const FDreamLyricParserOptions& ParserOptions = FDreamLyricParserOptions());
	bool ImportLyricFile(
		const FString& Filename,
		UDreamLyricAsset* Asset,
		dlp::EFileFormat Format,
		dlp::Parser::FParserOptions* ParserOptions);
	void ConvertParsedLyricToAsset(
		const dlp::File::FLyricFile& ParsedFile,
		UDreamLyricAsset* Asset);
};
