#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/Views/SListView.h" // 引入 ListView
#include "Widgets/SWindow.h"
#include "DreamLyricParserRuntimeBlueprint.h"
#include "DreamLyricImportDialog.generated.h"

UENUM(BlueprintType)
enum class ELrcImportMode : uint8
{
	ESLyric,
	WordByWord,
	LineByLine
};

class DREAMMUSICPLAYERIMPORTER_API SLyricImportDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLyricImportDialog)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetParentWindow(TWeakPtr<SWindow> InParentWindow) { ParentWindow = InParentWindow; }
	void SetFileFormat(const FString& InFileFormat) { FileFormat = InFileFormat; }

	void SetFilePath(const FString& InFilePath)
	{
		FilePath = InFilePath;
		LoadPreviewContent();
	}

	void SetFileContent(const FString& InFileContent)
	{
		PreviewContent = InFileContent;
		ProcessPreviewLines();
	}

	ELrcImportMode GetSelectedMode() const { return SelectedMode; }
	FDreamLyricParserOptions GetParserOptions() const { return ParserOptions; }
	bool ShouldImport() const { return bShouldImport; }
	
	void GetAssFileRoleKeys(FString& OutKey_Lyric, FString& OutKey_Translation, FString& OutKey_Romanization) const;

private:
	TWeakPtr<SWindow> ParentWindow;
	FString FileFormat = TEXT("lrc"); // 文件格式：lrc, ass, srt
	FString FilePath; // 文件路径
	FString PreviewContent; // 预览内容全文

	// 预览行数据（用于 ListView）
	TArray<TSharedPtr<FString>> PreviewLines;

	ELrcImportMode SelectedMode = ELrcImportMode::LineByLine;
	FDreamLyricParserOptions ParserOptions;
	bool bShouldImport = false;
	bool bShowAdvancedOptions = false;

	// 预览控件 (改为 ListView)
	TSharedPtr<SListView<TSharedPtr<FString>>> PreviewListView;

	// 滚动框（用于同步显示，如果需要的话，或者仅仅作为容器）
	TSharedPtr<class SScrollBox> PreviewScrollBox;

	// UI 控件引用
	TSharedPtr<class SComboBox<TSharedPtr<FString>>> FallbackRoleComboBox;
	TArray<TSharedPtr<FString>> FallbackRoleOptions;
	
	TSharedPtr<class SEditableTextBox> AssKeyRoleTextBox_Lyric;
	TSharedPtr<class SEditableTextBox> AssKeyRoleTextBox_Translation;
	TSharedPtr<class SEditableTextBox> AssKeyRoleTextBox_Romanization;

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

	// 添加一个新组
	FReply OnAddGroupClicked();
	// 移除一个组
	FReply OnRemoveGroupClicked(int32 Index);
	// 上移组
	FReply OnMoveGroupUpClicked(int32 Index);
	// 下移组
	FReply OnMoveGroupDownClicked(int32 Index);

	// 创建添加 Role 的下拉菜单
	TSharedRef<SWidget> MakeRolePickerMenu(int32 GroupIndex);
	// 实际添加 Role 的逻辑
	void OnAddRoleToGroup(int32 GroupIndex, EDreamMusicLyricTextRole Role);
	// 移除 Role 的逻辑
	FReply OnRemoveRoleClicked(int32 GroupIndex, int32 RoleIndex);
	// 上移 Role
	FReply OnMoveRoleUpClicked(int32 GroupIndex, int32 RoleIndex);
	// 下移 Role
	FReply OnMoveRoleDownClicked(int32 GroupIndex, int32 RoleIndex);

	// 辅助：获取单个 Enum 的显示文本
	FText GetRoleEnumText(EDreamMusicLyricTextRole Role) const;

	// 加载预览内容
	void LoadPreviewContent();
	// 处理预览行
	void ProcessPreviewLines();

	// 生成预览行 Widget
	TSharedRef<ITableRow> OnGenerateRowForPreview(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);
};
