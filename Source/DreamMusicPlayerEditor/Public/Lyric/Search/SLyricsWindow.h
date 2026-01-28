#pragma once

#include "CoreMinimal.h"
#include "DreamLyricsSearcher.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "DreamLyricsSearcher.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

// 定义列表项的数据指针类型 (Slate 列表通常使用 TSharedPtr)
using FSongItemPtr = TSharedPtr<FSongInfo>;

/**
 * 歌词搜索结果行的 UI 控件
 */
class SLyricsResultRow : public SMultiColumnTableRow<FSongItemPtr>
{
public:
	SLATE_BEGIN_ARGS(SLyricsResultRow)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, FSongItemPtr InItem)
	{
		Item = InItem;
		SMultiColumnTableRow<FSongItemPtr>::Construct(FSuperRowType::FArguments(), InOwnerTableView);
	}

	// 生成每一列的内容
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	FSongItemPtr Item;
};

/**
 * 主歌词搜索窗口
 */
class SLyricsWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLyricsWindow)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// --- UI 回调 ---
	FReply OnSearchClicked();
	void OnSearchTextCommitted(const FText& InText, ETextCommit::Type CommitInfo);
	TSharedRef<ITableRow> OnGenerateRow(FSongItemPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnRowDoubleClicked(FSongItemPtr Item);

	// --- 逻辑处理 ---
	void PerformSearch();
	void UpdateStatus(const FString& Message);

	// --- 数据源 ---
	TArray<FSongItemPtr> SongListItems; // 列表数据源
	TArray<TSharedPtr<FString>> PlatformOptions; // 下拉框选项

	// --- UI 控件引用 ---
	TSharedPtr<SEditableTextBox> SearchBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> PlatformComboBox;
	TSharedPtr<STextBlock> CurrentPlatformText;
	TSharedPtr<SListView<FSongItemPtr>> ResultsListView;
	TSharedPtr<SMultiLineEditableTextBox> LyricsPreviewBox;
	TSharedPtr<STextBlock> StatusTextBlock;

	// 当前选中的平台
	ELyricsPlatform CurrentPlatform = ELyricsPlatform::Netease;
};
