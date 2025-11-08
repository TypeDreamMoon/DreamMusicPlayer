#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"
#include "DreamLyricParserRuntime.h"
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
	void SetFileFormat(const FString& InFileFormat) { FileFormat = InFileFormat; }

	ELrcImportMode GetSelectedMode() const { return SelectedMode; }
	FDreamLyricParserOptions GetParserOptions() const { return ParserOptions; }
	bool ShouldImport() const { return bShouldImport; }

private:
	TWeakPtr<SWindow> ParentWindow;
	FString FileFormat = TEXT("lrc"); // 文件格式：lrc, ass, srt
	ELrcImportMode SelectedMode = ELrcImportMode::LineByLine;
	FDreamLyricParserOptions ParserOptions;
	bool bShouldImport = false;
	bool bShowAdvancedOptions = false;

	// UI 控件引用
	TSharedPtr<class SCheckBox> LyricRoleCheckBox;
	TSharedPtr<class SCheckBox> RomanizationRoleCheckBox;
	TSharedPtr<class SCheckBox> TranslationRoleCheckBox;
	TSharedPtr<class SComboBox<TSharedPtr<FString>>> FallbackRoleComboBox;
	TArray<TSharedPtr<FString>> FallbackRoleOptions;
	
	// 分组序列列表控件
	TSharedPtr<class SVerticalBox> GroupingSequenceList;

	FReply OnImportClicked();
	FReply OnCancelClicked();
	FReply OnToggleAdvancedOptions();
	
	void OnFallbackRoleSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	TSharedRef<SWidget> MakeFallbackRoleWidget(TSharedPtr<FString> InOption);
	FText GetFallbackRoleText() const;
	
	void UpdateParserOptions();
	void RefreshGroupingSequenceList();
	void OnRoleCheckBoxChanged(ECheckBoxState NewState, ELyricTextRole Role);
	FReply OnMoveUpClicked(int32 Index);
	FReply OnMoveDownClicked(int32 Index);
	FReply OnRemoveClicked(int32 Index);
	FText GetRoleDisplayText(ELyricTextRole Role) const;
	
	// 获取角色在序列中的索引
	int32 GetRoleIndexInSequence(ELyricTextRole Role) const;
};

