#include "Lyric/Search/SLyricsWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"

// ============================================================================
// 行控件实现 (SLyricsResultRow)
// ============================================================================

TSharedRef<SWidget> SLyricsResultRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	FString TextContent;

	if (ColumnName == TEXT("Title")) TextContent = Item->Title;
	else if (ColumnName == TEXT("Artist")) TextContent = Item->Artist;
	else if (ColumnName == TEXT("Album")) TextContent = Item->Album;
	else if (ColumnName == TEXT("Duration"))
	{
		// 毫秒转 mm:ss
		FTimespan Timespan = FTimespan::FromMilliseconds(Item->Duration);
		TextContent = FString::Printf(TEXT("%02d:%02d"), (int32)Timespan.GetTotalMinutes(), Timespan.GetSeconds());
	}
	else if (ColumnName == TEXT("Source")) TextContent = Item->Source;

	return SNew(SBox)
		.Padding(FMargin(4, 0))
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TextContent))
			.ColorAndOpacity(FSlateColor::UseForeground())
		];
}

// ============================================================================
// 主窗口实现 (SLyricsWindow)
// ============================================================================

void SLyricsWindow::Construct(const FArguments& InArgs)
{
	// 初始化下拉菜单选项
	PlatformOptions.Add(MakeShared<FString>(TEXT("Netease (网易云)")));
	PlatformOptions.Add(MakeShared<FString>(TEXT("Lrclib (开源库)")));

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(8)
		[
			SNew(SVerticalBox)

			// --- 1. 顶部搜索栏 ---
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(SHorizontalBox)

				// 平台选择
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 8, 0)
				[
					SAssignNew(PlatformComboBox, SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&PlatformOptions)
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
					{
						return SNew(STextBlock).Text(FText::FromString(*Item));
					})
					.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelection, ESelectInfo::Type)
					{
						if (NewSelection.IsValid())
						{
							CurrentPlatformText->SetText(FText::FromString(*NewSelection));
							if (NewSelection->Contains(TEXT("Netease"))) CurrentPlatform = ELyricsPlatform::Netease;
							else CurrentPlatform = ELyricsPlatform::Lrclib;
						}
					})
					[
						SAssignNew(CurrentPlatformText, STextBlock)
						.Text(FText::FromString(*PlatformOptions[0]))
					]
				]

				// 搜索输入框
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0, 0, 8, 0)
				[
					SAssignNew(SearchBox, SEditableTextBox)
					.HintText(FText::FromString(TEXT("请输入歌曲名 / 歌手...")))
					.OnTextCommitted(this, &SLyricsWindow::OnSearchTextCommitted)
				]

				// 搜索按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("搜索")))
					.OnClicked(this, &SLyricsWindow::OnSearchClicked)
					.ContentPadding(FMargin(12, 2))
				]
			]

			// --- 2. 结果列表 ---
			+ SVerticalBox::Slot()
			.FillHeight(0.6f)
			.Padding(0, 0, 0, 8)
			[
				SAssignNew(ResultsListView, SListView<FSongItemPtr>)
				.ItemHeight(24)
				.ListItemsSource(&SongListItems)
				.OnGenerateRow(this, &SLyricsWindow::OnGenerateRow)
				.OnMouseButtonDoubleClick(this, &SLyricsWindow::OnRowDoubleClicked)
				.SelectionMode(ESelectionMode::Single)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Title").DefaultLabel(FText::FromString(TEXT("标题"))).FillWidth(0.4f)
					+ SHeaderRow::Column("Artist").DefaultLabel(FText::FromString(TEXT("歌手"))).FillWidth(0.2f)
					+ SHeaderRow::Column("Album").DefaultLabel(FText::FromString(TEXT("专辑"))).FillWidth(0.25f)
					+ SHeaderRow::Column("Duration").DefaultLabel(FText::FromString(TEXT("时长"))).FixedWidth(60)
					+ SHeaderRow::Column("Source").DefaultLabel(FText::FromString(TEXT("来源"))).FixedWidth(60)
				)
			]

			// --- 3. 歌词预览区 ---
			+ SVerticalBox::Slot()
			.FillHeight(0.4f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("歌词预览 (双击上方歌曲下载):")))
				]
				+ SVerticalBox::Slot().FillHeight(1.0f)
				[
					SAssignNew(LyricsPreviewBox, SMultiLineEditableTextBox)
					.IsReadOnly(true)
					.BackgroundColor(FSlateColor(FLinearColor(0.05f, 0.05f, 0.05f)))
				]
			]

			// --- 4. 底部状态栏 ---
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 4, 0, 0)
			[
				SAssignNew(StatusTextBlock, STextBlock)
				.Text(FText::FromString(TEXT("就绪")))
				.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
			]
		]
	];
}

FReply SLyricsWindow::OnSearchClicked()
{
	PerformSearch();
	return FReply::Handled();
}

void SLyricsWindow::OnSearchTextCommitted(const FText& InText, ETextCommit::Type CommitInfo)
{
	if (CommitInfo == ETextCommit::OnEnter)
	{
		PerformSearch();
	}
}

void SLyricsWindow::PerformSearch()
{
	FString Keyword = SearchBox->GetText().ToString();
	if (Keyword.IsEmpty()) return;

	UpdateStatus(TEXT("正在搜索..."));
	SongListItems.Empty();
	ResultsListView->RequestListRefresh();

	// 调用之前写的工具类
	FLyricsTool::Search(Keyword, CurrentPlatform, [this](bool bSuccess, const TArray<FSongInfo>& Results)
	{
		// 确保 UI 更新在主线程
		AsyncTask(ENamedThreads::GameThread, [this, bSuccess, Results]()
		{
			if (!bSuccess)
			{
				UpdateStatus(TEXT("搜索失败，请检查网络"));
				return;
			}

			SongListItems.Empty();
			for (const FSongInfo& Info : Results)
			{
				// 将结果复制到 SharedPtr 供 Slate 使用
				SongListItems.Add(MakeShared<FSongInfo>(Info));
			}
			ResultsListView->RequestListRefresh();
			UpdateStatus(FString::Printf(TEXT("搜索完成，找到 %d 首歌曲"), Results.Num()));
		});
	});
}

void SLyricsWindow::OnRowDoubleClicked(FSongItemPtr Item)
{
	if (!Item.IsValid()) return;

	UpdateStatus(FString::Printf(TEXT("正在下载: %s..."), *Item->Title));
	LyricsPreviewBox->SetText(FText::FromString(TEXT("加载中...")));

	FLyricsTool::GetLyric(Item->ID, CurrentPlatform, [this](bool bSuccess, const FLyricResult& Result)
	{
		AsyncTask(ENamedThreads::GameThread, [this, bSuccess, Result]()
		{
			if (bSuccess)
			{
				FString FinalText = TEXT("[原始内容]\n") + Result.Lyric;
				if (!Result.TLyric.IsEmpty())
				{
					FinalText += TEXT("\n\n[翻译]\n") + Result.TLyric;
				}
				LyricsPreviewBox->SetText(FText::FromString(FinalText));
				UpdateStatus(TEXT("下载成功"));
			}
			else
			{
				LyricsPreviewBox->SetText(FText::FromString(TEXT("下载失败")));
				UpdateStatus(TEXT("下载失败"));
			}
		});
	});
}

TSharedRef<ITableRow> SLyricsWindow::OnGenerateRow(FSongItemPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SLyricsResultRow, OwnerTable, Item);
}

void SLyricsWindow::UpdateStatus(const FString& Message)
{
	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(FText::FromString(Message));
	}
}
