#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"
#include "LyricImportDialog.generated.h"

UENUM(BlueprintType)
enum class ELrcImportMode : uint8
{
	ESLyric,
	WordByWord,
	LineByLine
};

class DREAMMUSICPLAYERLYRIC_API SLyricImportDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLyricImportDialog)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	void SetParentWindow(TWeakPtr<SWindow> InParentWindow) { ParentWindow = InParentWindow; }

	ELrcImportMode GetSelectedMode() const { return SelectedMode; }
	bool ShouldImport() const { return bShouldImport; }

private:
	TWeakPtr<SWindow> ParentWindow;
	ELrcImportMode SelectedMode = ELrcImportMode::LineByLine;
	bool bShouldImport = false;

	FReply OnImportClicked();
	FReply OnCancelClicked();
};

