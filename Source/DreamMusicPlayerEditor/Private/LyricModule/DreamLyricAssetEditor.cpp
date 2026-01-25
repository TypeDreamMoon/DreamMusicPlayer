#include "LyricModule/DreamLyricAssetEditor.h"
#include "DreamLyricAsset.h"
#include "LyricModule/DreamLyricGroupWrapper.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "ToolMenus.h"
#include "DreamMusicPlayerCommon.h"
#include "DreamMusicPlayerEditorStyles.h"
#include "DreamMusicPlayerLog.h"
#include "EditorAssetLibrary.h"
#include "UObject/SavePackage.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/SNullWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "PropertyPath.h"
#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "DreamLyricAssetEditor"

static const FName PropertiesTabId(TEXT("DreamLyricAssetEditor_Properties"));
static const FName LyricListTabId(TEXT("DreamLyricAssetEditor_LyricList"));
static const FName StatisticsTabId(TEXT("DreamLyricAssetEditor_Statistics"));
static const FName EditTabId(TEXT("DreamLyricAssetEditor_Edit"));

void FDreamLyricAssetEditor::InitLyricAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UDreamLyricAsset* InLyricAsset)
{
	LyricAsset = InLyricAsset;
	SelectedGroupIndex = INDEX_NONE;
	SearchText = FText::GetEmpty();

	// 创建属性详情视图
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bShowOptions = true;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bShowPropertyMatrixButton = false;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bSearchInitialKeyFocus = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bShowModifiedPropertiesOption = false;

	DetailsView = PropertyModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(LyricAsset);

	// 注册属性变更回调
	PropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddSP(this, &FDreamLyricAssetEditor::OnObjectPropertyChanged);

	// 创建编辑视图的详情视图
	FPropertyEditorModule& EditPropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs EditDetailsViewArgs;
	EditDetailsViewArgs.bUpdatesFromSelection = false;
	EditDetailsViewArgs.bShowOptions = true;
	EditDetailsViewArgs.bAllowSearch = true;
	EditDetailsViewArgs.bShowPropertyMatrixButton = false;
	EditDetailsViewArgs.bHideSelectionTip = true;
	EditDetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	EditDetailsViewArgs.bSearchInitialKeyFocus = false;
	EditDetailsViewArgs.bLockable = false;
	EditDetailsViewArgs.bShowModifiedPropertiesOption = false;

	EditDetailsView = EditPropertyModule.CreateDetailView(EditDetailsViewArgs);

	// 创建标签页布局
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_DreamLyricAssetEditor_Layout_v2")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			                             ->Split
			                             (
				                             FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				                                                       ->SetSizeCoefficient(0.7f)
				                                                       ->Split
				                                                       (
					                                                       FTabManager::NewStack()
					                                                       ->SetSizeCoefficient(0.5f)
					                                                       ->AddTab(LyricListTabId, ETabState::OpenedTab)
				                                                       )
				                                                       ->Split
				                                                       (
					                                                       FTabManager::NewStack()
					                                                       ->SetSizeCoefficient(0.5f)
					                                                       ->AddTab(EditTabId, ETabState::OpenedTab)
				                                                       )
			                             )
			                             ->Split
			                             (
				                             FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				                                                       ->SetSizeCoefficient(0.3f)
				                                                       ->Split
				                                                       (
					                                                       FTabManager::NewStack()
					                                                       ->SetSizeCoefficient(0.5f)
					                                                       ->AddTab(PropertiesTabId, ETabState::OpenedTab)
				                                                       )
				                                                       ->Split
				                                                       (
					                                                       FTabManager::NewStack()
					                                                       ->SetSizeCoefficient(0.5f)
					                                                       ->AddTab(StatisticsTabId, ETabState::OpenedTab)
				                                                       )
			                             )
		);

	InitAssetEditor(Mode, InitToolkitHost, TEXT("DreamLyricAssetEditorApp"), StandaloneDefaultLayout, true, true, InLyricAsset);

	ExtendToolBar();

	// 刷新视图
	RefreshAllViews();
}

void FDreamLyricAssetEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(PropertiesTabId, FOnSpawnTab::CreateSP(this, &FDreamLyricAssetEditor::SpawnPropertiesTab))
	            .SetDisplayName(LOCTEXT("PropertiesTab", "Properties"))
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(LyricListTabId, FOnSpawnTab::CreateSP(this, &FDreamLyricAssetEditor::SpawnLyricListTab))
	            .SetDisplayName(LOCTEXT("LyricListTab", "Lyrics"))
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(StatisticsTabId, FOnSpawnTab::CreateSP(this, &FDreamLyricAssetEditor::SpawnStatisticsTab))
	            .SetDisplayName(LOCTEXT("StatisticsTab", "Statistics"))
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Info"));

	InTabManager->RegisterTabSpawner(EditTabId, FOnSpawnTab::CreateSP(this, &FDreamLyricAssetEditor::SpawnEditTab))
	            .SetDisplayName(LOCTEXT("EditTab", "Edit"))
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Edit"));
}

void FDreamLyricAssetEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(PropertiesTabId);
	InTabManager->UnregisterTabSpawner(LyricListTabId);
	InTabManager->UnregisterTabSpawner(StatisticsTabId);
	InTabManager->UnregisterTabSpawner(EditTabId);
}

FName FDreamLyricAssetEditor::GetToolkitFName() const
{
	return FName("DreamLyricAssetEditor");
}

FText FDreamLyricAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Lyric Asset Editor");
}

FString FDreamLyricAssetEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Lyric Asset ").ToString();
}

FLinearColor FDreamLyricAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f);
}

void FDreamLyricAssetEditor::SaveAsset_Execute()
{
	if (LyricAsset)
	{
		LyricAsset->MarkPackageDirty();
		UEditorAssetLibrary::SaveAsset(LyricAsset->GetPathName());
		RefreshAllViews();
	}
}

bool FDreamLyricAssetEditor::OnRequestClose(EAssetEditorCloseReason InCloseReason)
{
	// 清理委托
	if (PropertyChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(PropertyChangedHandle);
		PropertyChangedHandle.Reset();
	}

	return FAssetEditorToolkit::OnRequestClose(InCloseReason);
}

void FDreamLyricAssetEditor::ExtendToolBar()
{
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	// "Asset" 是工具栏的标准扩展点，或者使用 "Settings"
	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection("MyCustomSection");
			{
				ToolbarBuilder.AddWidget(
					SNew(SBox)
					.WidthOverride(256.0f)
					.HeightOverride(50.0f)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.OnClicked_Lambda([]()
						{
							UKismetSystemLibrary::LaunchURL(FString(TEXT("https://github.com/TypeDreamMoon")));
							return FReply::Handled();
						})
						[
							SNew(SImage)
							.Image(FDreamMusicPlayerEditorStyles::Get()->GetBrush("DreamToolkit"))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				);

				ToolbarBuilder.AddSeparator();

				ToolbarBuilder.AddWidget(
					SNew(SBox)
					.WidthOverride(180.0f)
					.HeightOverride(50.0f)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.OnClicked_Lambda([]()
						{
							UKismetSystemLibrary::LaunchURL(FString(TEXT("https://dmstudio.top")));
							return FReply::Handled();
						})
						[
							SNew(SImage)
							.Image(FDreamMusicPlayerEditorStyles::Get()->GetBrush("DreamDev"))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				);
			}
			ToolbarBuilder.EndSection();
		})
	);

	AddToolbarExtender(ToolbarExtender);

	RegenerateMenusAndToolbars();
}

TSharedRef<SDockTab> FDreamLyricAssetEditor::SpawnPropertiesTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PropertiesTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("PropertiesTitle", "Properties"))
		.TabColorScale(GetTabColorScale())
		[
			DetailsView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FDreamLyricAssetEditor::SpawnLyricListTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == LyricListTabId);

	RefreshLyricList();

	return SNew(SDockTab)
		.Label(LOCTEXT("LyricListTitle", "Lyrics"))
		.TabColorScale(GetTabColorScale())
		[
			SNew(SVerticalBox)
			// 工具栏
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(8.0f)
				[
					SNew(SHorizontalBox)
					// 搜索框
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(0.0f, 0.0f, 8.0f, 0.0f)
					[
						SAssignNew(SearchBox, SSearchBox)
						.HintText(LOCTEXT("SearchHint", "搜索歌词..."))
						.OnTextChanged(this, &FDreamLyricAssetEditor::OnSearchTextChanged)
					]
					// 添加按钮
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(4.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("AddGroup", "添加组"))
						.ToolTipText(LOCTEXT("AddGroupTooltip", "添加新的歌词组"))
						.OnClicked(this, &FDreamLyricAssetEditor::OnAddNewGroup)
						.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
					]
					// 删除按钮
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(4.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("DeleteGroup", "删除组"))
						.ToolTipText(LOCTEXT("DeleteGroupTooltip", "删除选中的歌词组"))
						.OnClicked(this, &FDreamLyricAssetEditor::OnDeleteSelectedGroup)
						.ButtonStyle(FAppStyle::Get(), "FlatButton.Danger")
						.IsEnabled_Lambda([this]() { return SelectedGroupIndex != INDEX_NONE && LyricAsset && LyricAsset->Groups.IsValidIndex(SelectedGroupIndex); })
					]
					// 排序按钮
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(4.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("SortGroups", "排序"))
						.ToolTipText(LOCTEXT("SortGroupsTooltip", "按时间戳排序所有组"))
						.OnClicked(this, &FDreamLyricAssetEditor::OnSortGroups)
						.ButtonStyle(FAppStyle::Get(), "FlatButton")
					]
				]
			]

			// 列表视图
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(8.0f, 0.0f, 8.0f, 8.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(4.0f)
				[
					SAssignNew(LyricListView, SListView<TSharedPtr<FDreamMusicLyricGroup>>)
					.ListItemsSource(&LyricListItems)
					.SelectionMode(ESelectionMode::Type::Single)
					.OnGenerateRow(this, &FDreamLyricAssetEditor::GenerateLyricListRow)
					.OnSelectionChanged_Lambda([this](TSharedPtr<FDreamMusicLyricGroup> Item, ESelectInfo::Type SelectInfo)
					{
						if (Item.IsValid())
						{
							SelectedGroupIndex = LyricAsset->Groups.IndexOfByPredicate([&](const FDreamMusicLyricGroup& Group)
							{
								return Group == Item.Get();
							});

							DMP_LOG(Log, TEXT("SelectedItemIdx: %d"), SelectedGroupIndex);
						}
						else
						{
							SelectedGroupIndex = INDEX_NONE;
						}
						RefreshEditView();
					})
				]
			]
		];
}

TSharedRef<SDockTab> FDreamLyricAssetEditor::SpawnStatisticsTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == StatisticsTabId);

	// 如果还没有创建，先创建
	if (!StatisticsTextBlock.IsValid())
	{
		StatisticsTextBlock = SNew(STextBlock)
			.AutoWrapText(true);
	}

	RefreshStatistics();

	return SNew(SDockTab)
		.Label(LOCTEXT("StatisticsTitle", "Statistics"))
		.TabColorScale(GetTabColorScale())
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(12.0f)
			[
				SNew(SVerticalBox)
				// 统计信息卡片
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 12.0f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(12.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 8.0f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("StatisticsTitle", "统计信息"))
							.TextStyle(FAppStyle::Get(), "ContentBrowser.TopBar.Font")
							.Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							StatisticsTextBlock.ToSharedRef()
						]
					]
				]
			]
		];
}

TSharedRef<SDockTab> FDreamLyricAssetEditor::SpawnEditTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == EditTabId);

	RefreshEditView();

	return SNew(SDockTab)
		.Label(LOCTEXT("EditTabTitle", "Edit"))
		.TabColorScale(GetTabColorScale())
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(12.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 12.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("EditTabDescription", "选择左侧列表中的歌词组进行编辑"))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					EditDetailsView.ToSharedRef()
				]
			]
		];
}

void FDreamLyricAssetEditor::RefreshAllViews()
{
	RefreshLyricList();
	RefreshStatistics();
	RefreshEditView();
	if (DetailsView.IsValid())
	{
		DetailsView->SetObject(LyricAsset, true);
	}
}

void FDreamLyricAssetEditor::RefreshLyricList()
{
	LyricListItems.Empty();
	LyricGroupData.Empty();

	if (!LyricAsset)
	{
		return;
	}

	// 创建索引映射（按时间排序，但保留原始索引）
	struct FGroupWithIndex
	{
		const FDreamMusicLyricGroup* Group;
		int32 OriginalIndex;
	};
	TArray<FGroupWithIndex> GroupsWithIndex;
	for (int32 i = 0; i < LyricAsset->Groups.Num(); ++i)
	{
		GroupsWithIndex.Add({&LyricAsset->Groups[i], i});
	}
	GroupsWithIndex.Sort([](const FGroupWithIndex& A, const FGroupWithIndex& B)
	{
		return A.Group->StartTimestamp < B.Group->StartTimestamp;
	});

	// 生成列表项（保持向后兼容）
	for (int32 i = 0; i < GroupsWithIndex.Num(); ++i)
	{
		const FDreamMusicLyricGroup& Group = *GroupsWithIndex[i].Group;
		int32 OriginalIndex = GroupsWithIndex[i].OriginalIndex;
		FString TimeStr = FString::Printf(TEXT("%02d:%02d.%03d"),
		                                  Group.StartTimestamp.Minute,
		                                  Group.StartTimestamp.Seconds,
		                                  Group.StartTimestamp.Millisecond);

		FString ContentStr;
		for (const FDreamMusicLyricLine& Line : Group.Lines)
		{
			if (!ContentStr.IsEmpty())
			{
				ContentStr += TEXT(" | ");
			}

			FString RoleStr;
			switch (Line.Role)
			{
			case EDreamMusicLyricTextRole::Lyric:
				RoleStr = TEXT("[Lyric]");
				break;
			case EDreamMusicLyricTextRole::Romanization:
				RoleStr = TEXT("[Romanization]");
				break;
			case EDreamMusicLyricTextRole::Translation:
				RoleStr = TEXT("[Translation]");
				break;
			default:
				RoleStr = TEXT("[None]");
				break;
			}

			FString LineText = Line.Text;
			if (LineText.IsEmpty() && Line.Words.Num() > 0)
			{
				// 从 Words 构建文本
				for (const FDreamMusicLyricWord& Word : Line.Words)
				{
					LineText += Word.Content;
				}
			}
			ContentStr += RoleStr + TEXT(" ") + LineText;
		}

		LyricListItems.Add(MakeShareable(new FDreamMusicLyricGroup(LyricAsset->Groups[OriginalIndex])));

		// 创建组显示数据
		LyricGroupData.Add(MakeShareable(new FLyricGroupDisplayData{Group, OriginalIndex}));
	}

	if (LyricListView.IsValid())
	{
		LyricListView->RequestListRefresh();
	}
}

void FDreamLyricAssetEditor::RefreshStatistics()
{
	// 如果还没有创建，先创建
	if (!StatisticsTextBlock.IsValid())
	{
		StatisticsTextBlock = SNew(STextBlock)
			.AutoWrapText(true);
	}

	if (!LyricAsset)
	{
		StatisticsTextBlock->SetText(LOCTEXT("NoAsset", "未加载资产"));
		return;
	}

	FLyricAssetStatistics Stats = LyricAsset->GetStatistics();

	FString StatsText;

	// 基本信息
	StatsText += FString::Printf(
		TEXT("📊 基本信息\n")
		TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")
		TEXT("总组数: %d\n")
		TEXT("总行数: %d\n")
		TEXT("总单词数: %d\n\n"),
		Stats.TotalGroups,
		Stats.TotalLines,
		Stats.TotalWords
	);

	// 时间信息
	StatsText += FString::Printf(
		TEXT("⏱️ 时间信息\n")
		TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")
		TEXT("总时长: %.2f 秒\n")
		TEXT("开始时间: %02d:%02d.%03d\n")
		TEXT("结束时间: %02d:%02d.%03d\n\n"),
		Stats.TotalDurationSeconds,
		Stats.StartTime.Minute, Stats.StartTime.Seconds, Stats.StartTime.Millisecond,
		Stats.EndTime.Minute, Stats.EndTime.Seconds, Stats.EndTime.Millisecond
	);

	// 特性信息
	StatsText += FString::Printf(
		TEXT("✨ 特性\n")
		TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")
		TEXT("包含逐词时间: %s\n")
		TEXT("包含多种角色: %s\n\n"),
		Stats.bHasWordTimings ? TEXT("✓ 是") : TEXT("✗ 否"),
		Stats.bHasMultipleRoles ? TEXT("✓ 是") : TEXT("✗ 否")
	);

	// 元数据信息
	FString Title = LyricAsset->GetTitle();
	FString Artist = LyricAsset->GetArtist();
	FString Album = LyricAsset->GetAlbum();
	FString Creator = LyricAsset->GetCreator();

	if (!Title.IsEmpty() || !Artist.IsEmpty() || !Album.IsEmpty() || !Creator.IsEmpty())
	{
		StatsText += TEXT("📝 元数据\n");
		StatsText += TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

		if (!Title.IsEmpty())
		{
			StatsText += FString::Printf(TEXT("标题: %s\n"), *Title);
		}
		if (!Artist.IsEmpty())
		{
			StatsText += FString::Printf(TEXT("艺术家: %s\n"), *Artist);
		}
		if (!Album.IsEmpty())
		{
			StatsText += FString::Printf(TEXT("专辑: %s\n"), *Album);
		}
		if (!Creator.IsEmpty())
		{
			StatsText += FString::Printf(TEXT("创建者: %s\n"), *Creator);
		}
	}

	StatisticsTextBlock->SetText(FText::FromString(StatsText));
}

TSharedRef<ITableRow> FDreamLyricAssetEditor::GenerateLyricListRow(TSharedPtr<FDreamMusicLyricGroup> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	FDreamMusicLyricGroup Item = *InItem;

	// 检查是否匹配搜索文本
	bool bMatchesSearch = true;
	if (!SearchText.IsEmpty())
	{
		FString SearchStr = SearchText.ToString().ToLower();
		bMatchesSearch = Item.ToString().ToLower().Contains(SearchStr);
	}

	// 查找对应的组数据
	TSharedPtr<FLyricGroupDisplayData> GroupData;
	for (const auto& Data : LyricGroupData)
	{
		if (Data->Group == Item)
		{
			GroupData = Data;
			break;
		}
	}

	// 使用组数据生成更美观的UI
	if (GroupData.IsValid())
	{
		return GenerateLyricGroupRow(GroupData, OwnerTable);
	}

	// 回退到简单显示
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		.Style(FAppStyle::Get(), "TableView.Row")
		[
			SNew(SBorder)
			.BorderImage(bMatchesSearch ? FAppStyle::GetBrush("NoBorder") : FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.0f, 4.0f)
			.Visibility(bMatchesSearch ? EVisibility::Visible : EVisibility::Collapsed)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 12.0f, 0.0f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(6.0f, 4.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Item.StartTimestamp.ToString()))
						.Font(FCoreStyle::GetDefaultFontStyle("Mono", 11))
						.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.9f, 1.0f))
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Item.ToString()))
					.AutoWrapText(true)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				]
			]
		];
}

TSharedRef<SWidget> FDreamLyricAssetEditor::CreateLyricLinesWidget(TSharedPtr<FLyricGroupDisplayData> InGroupData)
{
	TSharedRef<SVerticalBox> LinesBox = SNew(SVerticalBox);
	if (InGroupData.IsValid())
	{
		for (const auto& Line : InGroupData->Group.Lines)
		{
			LinesBox->AddSlot()
			        .AutoHeight()
			        .Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(SHorizontalBox)
				// 角色标签
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.BorderBackgroundColor(GetRoleColor(Line.Role) * 0.3f)
					.Padding(6.0f, 3.0f)
					[
						SNew(STextBlock)
						.Text(GetRoleDisplayName(Line.Role))
						.Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
						.ColorAndOpacity(GetRoleColor(Line.Role))
					]
				]
				// 歌词文本
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Line.Text))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.AutoWrapText(true)
					.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
				]
			];
		}
	}
	return LinesBox;
}

TSharedRef<ITableRow> FDreamLyricAssetEditor::GenerateLyricGroupRow(TSharedPtr<FLyricGroupDisplayData> InGroupData, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!InGroupData.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FLyricGroupDisplayData>>, OwnerTable);
	}

	// 检查是否匹配搜索文本
	bool bMatchesSearch = true;
	if (!SearchText.IsEmpty())
	{
		FString SearchStr = SearchText.ToString().ToLower();
		bMatchesSearch = InGroupData.Get()->Group.ToString().Contains(SearchStr);
		if (!bMatchesSearch)
		{
			for (const auto& Line : InGroupData->Group.Lines)
			{
				if (Line.Text.ToLower().Contains(SearchStr))
				{
					bMatchesSearch = true;
					break;
				}
			}
		}
	}

	return SNew(STableRow<TSharedPtr<FLyricGroupDisplayData>>, OwnerTable)
		.Style(FAppStyle::Get(), "TableView.Row")
		[
			SNew(SBorder)
			.OnMouseButtonDown_Lambda([this, InGroupData, OwnerTable](const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) -> FReply
			{
				if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
				{
					if (InGroupData.IsValid() && LyricAsset && LyricAsset->Groups.IsValidIndex(InGroupData->GroupIndex))
					{
						SelectedGroupIndex = InGroupData->GroupIndex;
						FMenuBuilder MenuBuilder(true, nullptr);
						MenuBuilder.AddMenuEntry(
							FUIAction(FExecuteAction::CreateSP(this, &FDreamLyricAssetEditor::OnEditSelectedGroup)),
							SNew(STextBlock)
							.Text(LOCTEXT("EditGroup", "编辑")),
							NAME_None,
							LOCTEXT("EditGroupTooltip", "编辑选中的歌词组")
						);
						MenuBuilder.AddMenuSeparator();
						MenuBuilder.AddMenuEntry(
							FUIAction(FExecuteAction::CreateSP(this, &FDreamLyricAssetEditor::OnDeleteSelectedGroupMenu)),
							SNew(STextBlock)
							.Text(LOCTEXT("DeleteGroupMenu", "删除")),
							NAME_None,
							LOCTEXT("DeleteGroupMenuTooltip", "删除选中的歌词组")
						);
						FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
						FSlateApplication::Get().PushMenu(
							OwnerTable->AsShared(),
							WidgetPath,
							MenuBuilder.MakeWidget(),
							MouseEvent.GetScreenSpacePosition(),
							FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
						);
						return FReply::Handled();
					}
				}
				return FReply::Unhandled();
			})
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.3f))
			.Padding(12.0f, 8.0f)
			.Visibility(bMatchesSearch ? EVisibility::Visible : EVisibility::Collapsed)
			[
				SNew(SVerticalBox)
				// 时间戳和行内容
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 8.0f)
				[
					SNew(SHorizontalBox)
					// 时间戳（带图标样式）
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Top)
					.Padding(0.0f, 2.0f, 12.0f, 0.0f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
						.BorderBackgroundColor(FLinearColor(0.2f, 0.4f, 0.8f, 0.3f))
						.Padding(8.0f, 6.0f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(InGroupData.Get()->Group.StartTimestamp.ToString()))
							.Font(FCoreStyle::GetDefaultFontStyle("Mono", 12))
							.ColorAndOpacity(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f))
							.ShadowOffset(FVector2D(1.0f, 1.0f))
							.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f))
						]
					]
					// 歌词内容
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						CreateLyricLinesWidget(InGroupData)
					]
				]
			]
		];
}

FLinearColor FDreamLyricAssetEditor::GetRoleColor(EDreamMusicLyricTextRole Role) const
{
	switch (Role)
	{
	case EDreamMusicLyricTextRole::Lyric:
		return FLinearColor(0.4f, 0.8f, 1.0f, 1.0f); // 蓝色
	case EDreamMusicLyricTextRole::Romanization:
		return FLinearColor(1.0f, 0.8f, 0.4f, 1.0f); // 黄色
	case EDreamMusicLyricTextRole::Translation:
		return FLinearColor(0.6f, 1.0f, 0.6f, 1.0f); // 绿色
	default:
		return FLinearColor(0.7f, 0.7f, 0.7f, 1.0f); // 灰色
	}
}

FText FDreamLyricAssetEditor::GetRoleDisplayName(EDreamMusicLyricTextRole Role) const
{
	switch (Role)
	{
	case EDreamMusicLyricTextRole::Lyric:
		return LOCTEXT("RoleLyric", "歌词");
	case EDreamMusicLyricTextRole::Romanization:
		return LOCTEXT("RoleRomanization", "音译");
	case EDreamMusicLyricTextRole::Translation:
		return LOCTEXT("RoleTranslation", "翻译");
	default:
		return LOCTEXT("RoleNone", "无");
	}
}

void FDreamLyricAssetEditor::OnPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	RefreshAllViews();
}

void FDreamLyricAssetEditor::OnObjectPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (ObjectBeingModified == LyricAsset)
	{
		RefreshAllViews();
	}
}

void FDreamLyricAssetEditor::OnSearchTextChanged(const FText& InSearchText)
{
	SearchText = InSearchText;
	if (LyricListView.IsValid())
	{
		LyricListView->RequestListRefresh();
	}
}

FReply FDreamLyricAssetEditor::OnAddNewGroup()
{
	if (!LyricAsset)
	{
		return FReply::Handled();
	}

	// 创建新组，时间戳为当前最后一个组的时间 + 1秒，或者为 00:00.000
	FDreamMusicTimestamp NewTimestamp;
	if (LyricAsset->Groups.Num() > 0)
	{
		// 找到最大时间戳
		FDreamMusicTimestamp MaxTime = LyricAsset->Groups[0].StartTimestamp;
		for (const FDreamMusicLyricGroup& Group : LyricAsset->Groups)
		{
			if (Group.StartTimestamp > MaxTime)
			{
				MaxTime = Group.StartTimestamp;
			}
		}
		// 添加1秒
		NewTimestamp = FDreamMusicTimestamp::FromTotalMilliseconds(MaxTime.ToTotalMilliseconds() + 1000);
	}

	FDreamMusicLyricGroup NewGroup(NewTimestamp);
	FDreamMusicLyricLine NewLine(TEXT("新歌词"), EDreamMusicLyricTextRole::Lyric);
	NewGroup.Lines.Add(NewLine);

	LyricAsset->Groups.Add(NewGroup);
	LyricAsset->MarkPackageDirty();
	RefreshAllViews();

	// 显示通知
	FNotificationInfo Info(LOCTEXT("GroupAdded", "已添加新歌词组"));
	Info.ExpireDuration = 2.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	return FReply::Handled();
}

FReply FDreamLyricAssetEditor::OnDeleteSelectedGroup()
{
	OnDeleteSelectedGroupMenu();
	return FReply::Handled();
}

void FDreamLyricAssetEditor::OnDeleteSelectedGroupMenu()
{
	if (!LyricAsset || SelectedGroupIndex == INDEX_NONE || !LyricAsset->Groups.IsValidIndex(SelectedGroupIndex))
	{
		return;
	}

	LyricAsset->Groups.RemoveAt(SelectedGroupIndex);
	SelectedGroupIndex = INDEX_NONE;
	LyricAsset->MarkPackageDirty();
	RefreshAllViews();

	// 显示通知
	FNotificationInfo Info(LOCTEXT("GroupDeleted", "已删除歌词组"));
	Info.ExpireDuration = 2.0f;
	FSlateNotificationManager::Get().AddNotification(Info);
}

FReply FDreamLyricAssetEditor::OnSortGroups()
{
	if (!LyricAsset)
	{
		return FReply::Handled();
	}

	LyricAsset->SortGroupsByTime();
	LyricAsset->MarkPackageDirty();
	RefreshAllViews();

	// 显示通知
	FNotificationInfo Info(LOCTEXT("GroupsSorted", "已按时间戳排序"));
	Info.ExpireDuration = 2.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	return FReply::Handled();
}

void FDreamLyricAssetEditor::OnEditSelectedGroup()
{
	if (SelectedGroupIndex != INDEX_NONE && LyricAsset && LyricAsset->Groups.IsValidIndex(SelectedGroupIndex))
	{
		// 打开编辑标签页
		RefreshEditView();
	}
}

void FDreamLyricAssetEditor::RefreshEditView()
{
	if (!EditDetailsView.IsValid())
	{
		return;
	}

	if (SelectedGroupIndex != INDEX_NONE && LyricAsset && LyricAsset->Groups.IsValidIndex(SelectedGroupIndex))
	{
		// 创建或更新包装器对象
		if (!IsValid(EditingGroupObject))
		{
			EditingGroupObject = NewObject<UDreamLyricGroupWrapper>(GetTransientPackage(), UDreamLyricGroupWrapper::StaticClass());

			// 注册属性变更回调，以便在编辑时同步回原始数组
			EditDetailsView->OnFinishedChangingProperties().AddSP(this, &FDreamLyricAssetEditor::OnEditGroupPropertyChanged);
		}

		// 将选中的组复制到包装器中
		UDreamLyricGroupWrapper* Wrapper = Cast<UDreamLyricGroupWrapper>(EditingGroupObject);
		if (Wrapper)
		{
			Wrapper->Group = LyricAsset->Groups[SelectedGroupIndex];

			// 设置包装器对象到 Detail 视图
			EditDetailsView->SetObject(Wrapper, true);
		}
	}
	else
	{
		// 没有选中项，清空编辑视图
		EditDetailsView->SetObject(nullptr, true);
		EditingGroupObject = nullptr;
	}
}

void FDreamLyricAssetEditor::OnEditGroupPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	// 当编辑组属性变更时，将更改同步回原始数组
	if (SelectedGroupIndex != INDEX_NONE && LyricAsset && LyricAsset->Groups.IsValidIndex(SelectedGroupIndex))
	{
		UDreamLyricGroupWrapper* Wrapper = Cast<UDreamLyricGroupWrapper>(EditingGroupObject);
		if (Wrapper)
		{
			// 将包装器中的组复制回原始数组
			LyricAsset->Groups[SelectedGroupIndex] = Wrapper->Group;
			LyricAsset->MarkPackageDirty();
		}
	}
}

#undef LOCTEXT_NAMESPACE
