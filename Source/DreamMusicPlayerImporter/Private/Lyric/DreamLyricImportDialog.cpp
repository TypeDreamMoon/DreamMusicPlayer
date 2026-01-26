#include "Lyric/DreamLyricImportDialog.h"

#include "DreamMusicPlayerLog.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/SlateStyle.h"
#include "Styling/AppStyle.h"

#include "Misc/FileHelper.h"

void SLyricImportDialog::Construct(const FArguments& InArgs)
{
	// 初始化默认解析选项
	ParserOptions = FDreamLyricParserOptions::GetDefault(false);

	// 对于非 LRC 文件，默认展开高级选项
	if (FileFormat != TEXT("lrc"))
	{
		bShowAdvancedOptions = true;
	}
	if (FileFormat == TEXT("ass"))
	{
		bShowAdvancedOptions = false;
	}

	// 创建回退角色选项列表
	FallbackRoleOptions.Add(MakeShared<FString>(TEXT("Lyric")));
	FallbackRoleOptions.Add(MakeShared<FString>(TEXT("Romanization")));
	FallbackRoleOptions.Add(MakeShared<FString>(TEXT("Translation")));

	ChildSlot
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.3f))
		.Padding(16.0f)
		[
			SNew(SVerticalBox)

			// Content
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SSplitter)
				.Orientation(Orient_Horizontal)

				+ SSplitter::Slot()
				.Value(0.5f)
				[
					SNew(SScrollBox)

					+ SScrollBox::Slot()
					[
						SNew(SBox)
						.WidthOverride(520.0f)
						[
							SNew(SVerticalBox)
							// 标题区域
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0, 0, 0, 16)
							[
								SNew(SBorder)
								.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f))
								.Padding(12, 10)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(STextBlock)
										.Text_Lambda([this]()
										{
											if (FileFormat == TEXT("lrc")) return FText::FromString(TEXT("📝 Import LRC File"));
											if (FileFormat == TEXT("ass")) return FText::FromString(TEXT("📝 Import ASS File"));
											if (FileFormat == TEXT("srt")) return FText::FromString(TEXT("📝 Import SRT File"));
											return FText::FromString(TEXT("📝 Import Lyric File"));
										})
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
										.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 6, 0, 0)
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("LyricImport", "Description", "Please configure import options:"))
										.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
										.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
									]
								]
							]
							// LRC 模式选择区域
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0, 0, 0, 12)
							[
								SNew(SBorder)
								.Visibility_Lambda([this]() { return FileFormat == TEXT("lrc") ? EVisibility::Visible : EVisibility::Collapsed; })
								.Padding(12, 10)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 0, 0, 8)
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("LyricImport", "ImportMode", "Import Mode:"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
										.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
									]
									// ... (Checkbox 保持不变) ...
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 4)
									[
										SNew(SCheckBox)
										.IsChecked_Lambda([this]() { return SelectedMode == ELrcImportMode::ESLyric ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
										.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { if (NewState == ECheckBoxState::Checked) SelectedMode = ELrcImportMode::ESLyric; })
										.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("RadioButton"))
										.Content()
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("LyricImport", "ESLyric", "ESLyric"))
											.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
										]
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 4)
									[
										SNew(SCheckBox)
										.IsChecked_Lambda([this]() { return SelectedMode == ELrcImportMode::WordByWord ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
										.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { if (NewState == ECheckBoxState::Checked) SelectedMode = ELrcImportMode::WordByWord; })
										.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("RadioButton"))
										.Content()
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("LyricImport", "WordByWord", "WordByWord"))
											.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
										]
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 4)
									[
										SNew(SCheckBox)
										.IsChecked_Lambda([this]() { return SelectedMode == ELrcImportMode::LineByLine ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
										.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { if (NewState == ECheckBoxState::Checked) SelectedMode = ELrcImportMode::LineByLine; })
										.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("RadioButton"))
										.Content()
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("LyricImport", "LineByLine", "LineByLine"))
											.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
										]
									]
								]
							]

							// ASS 模式选择区域
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0, 0, 0, 12)
							[
								SNew(SBorder)
								.Visibility_Lambda([this]() { return FileFormat == TEXT("ass") ? EVisibility::Visible : EVisibility::Collapsed; })
								.Padding(12, 10)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 0, 0, 8)
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("LyricImport", "AssRoleKey", "Role Key"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
										.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 4)
									[
										SAssignNew(AssKeyRoleTextBox_Lyric, SEditableTextBox)
										.HintText(NSLOCTEXT("LyricImport", "AssKeyLyric", "Lyric"))
										.Text(NSLOCTEXT("LyricImport", "AssKeyLyric", "orig"))
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 4)
									[
										SAssignNew(AssKeyRoleTextBox_Romanization, SEditableTextBox)
										.HintText(NSLOCTEXT("LyricImport", "AssKeyRomanization", "Romanization"))
										.Text(NSLOCTEXT("LyricImport", "AssKeyRomanization", "roma"))
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 4)
									[
										SAssignNew(AssKeyRoleTextBox_Translation, SEditableTextBox)
										.HintText(NSLOCTEXT("LyricImport", "AssKeyTranslation", "Translation"))
										.Text(NSLOCTEXT("LyricImport", "AssKeyTranslation", "ts"))
									]
								]
							]

							// 高级选项按钮
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0, 0, 0, 8)
							[
								SNew(SButton)
								.Text_Lambda([this]()
								{
									return bShowAdvancedOptions
										       ? NSLOCTEXT("LyricImport", "HideAdvanced", "▼ Hide Advanced Options")
										       : NSLOCTEXT("LyricImport", "ShowAdvanced", "▶ Show Advanced Options");
								})
								.OnClicked(this, &SLyricImportDialog::OnToggleAdvancedOptions)
								.ContentPadding(FMargin(8, 6))
								.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton"))
								.HAlign(HAlign_Left)
								.Visibility_Lambda([this]()
								{
									if (FileFormat == TEXT("ass"))
									{
										return EVisibility::Collapsed;
									}
									else
									{
										return EVisibility::Visible;
									}
								})
							]

							// 高级选项区域
							// Grouping Sequence
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0, 0, 0, 8)
							[
								SNew(SBorder)
								.Visibility_Lambda([this]()
								{
									if (FileFormat == TEXT("ass"))
									{
										return EVisibility::Collapsed;
									}
									else
									{
										return bShowAdvancedOptions ? EVisibility::Visible : EVisibility::Collapsed;
									}
								})
								.Padding(12, 10)
								[
									SNew(SVerticalBox)
									// 树状图结构标题
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 0, 0, 8)
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("LyricImport", "GroupingTree", "Grouping Structure:"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
										.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
									]

									// 树状图列表
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 0, 0, 10)
									[
										SAssignNew(GroupingSequenceList, SVerticalBox)
									]

									// 底部添加组按钮
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 0, 0, 10)
									[
										SNew(SButton)
										.Text(NSLOCTEXT("LyricImport", "AddGroup", "+ Add New Group"))
										.OnClicked(this, &SLyricImportDialog::OnAddGroupClicked)
										.HAlign(HAlign_Center)
									]

									// 回退角色区域
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 8, 0, 0)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(0, 0, 0, 6)
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("LyricImport", "FallbackRole", "Fallback Role:"))
											.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
											.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SAssignNew(FallbackRoleComboBox, SComboBox<TSharedPtr<FString>>)
											.OptionsSource(&FallbackRoleOptions)
											.InitiallySelectedItem(FallbackRoleOptions[0])
											.OnGenerateWidget(this, &SLyricImportDialog::MakeFallbackRoleWidget)
											.OnSelectionChanged(this, &SLyricImportDialog::OnFallbackRoleSelectionChanged)
											.ContentPadding(FMargin(6, 4))
											[
												SNew(STextBlock)
												.Text(this, &SLyricImportDialog::GetFallbackRoleText)
												.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
											]
										]
									]
								]
							]
						]
					]
				]

				+ SSplitter::Slot()
				.Value(0.5f)
				[
					SNew(SBorder)
					             .Padding(0) // 移除内边距以使列表贴边
					             .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder")) // 更深的背景
					[
						SNew(SVerticalBox)
						// 预览标题
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(12, 10, 12, 8)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("LyricImport", "PreviewTitle", "📄 File Preview"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
							.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
						]
						// 文件信息
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(12, 0, 12, 8)
						[
							SNew(STextBlock)
							.Text_Lambda([this]()
							{
								if (!FilePath.IsEmpty())
								{
									return FText::FromString(FString::Printf(TEXT("File: %s"), *FPaths::GetCleanFilename(FilePath)));
								}
								return FText::FromString(TEXT("No file selected"));
							})
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
							.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
						]
						// 预览内容
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Fill)
						.FillHeight(1.0f)
						[
							// 使用横向 ScrollBox 包裹 ListView 以支持横向滚动
							SNew(SScrollBox)
							.Orientation(Orient_Horizontal)
							.ScrollBarVisibility(EVisibility::Visible) // 总是显示横向滚动条以便发现
							+ SScrollBox::Slot()
							.FillSize(1.0f)
							[
								SNew(SBox)
								// 移除 WidthOverride，让内容撑开
								.MinDesiredWidth(300.0f)
								[
									SAssignNew(PreviewListView, SListView<TSharedPtr<FString>>)
									.ListItemsSource(&PreviewLines)
									.OnGenerateRow(this, &SLyricImportDialog::OnGenerateRowForPreview)
									.SelectionMode(ESelectionMode::Single)
									// 移除内置滚动条的背景，使其更干净
									// .ScrollbarStyle(...) 
								]
							]
						]
					]
				]
			]
			// Footer (Import/Cancel buttons)
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0, 16, 0, 0)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("NoBorder"))
				.Padding(0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.HAlign(HAlign_Right)
					.Padding(0, 0, 8, 0)
					[
						SNew(SButton)
						.Text(NSLOCTEXT("LyricImport", "Import", "✓ Import"))
						.OnClicked(this, &SLyricImportDialog::OnImportClicked)
						.ContentPadding(FMargin(16, 8))
						.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("PrimaryButton"))
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(NSLOCTEXT("LyricImport", "Cancel", "Cancel"))
						.OnClicked(this, &SLyricImportDialog::OnCancelClicked)
						.ContentPadding(FMargin(16, 8))
						.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
					]
				]
			]
		]
	];

	// 初始化分组序列列表显示
	RefreshGroupingSequenceList();
}

void SLyricImportDialog::GetAssFileRoleKeys(FString& OutKey_Lyric, FString& OutKey_Translation, FString& OutKey_Romanization) const
{
	OutKey_Lyric = AssKeyRoleTextBox_Lyric->GetText().ToString();
	OutKey_Translation = AssKeyRoleTextBox_Translation->GetText().ToString();
	OutKey_Romanization = AssKeyRoleTextBox_Romanization->GetText().ToString();
}

void SLyricImportDialog::LoadPreviewContent()
{
	if (!FilePath.IsEmpty() && FPaths::FileExists(FilePath))
	{
		if (FFileHelper::LoadFileToString(PreviewContent, *FilePath))
		{
			ProcessPreviewLines();
		}
		else
		{
			PreviewContent = TEXT("无法读取文件内容");
			ProcessPreviewLines();
		}
	}
}

void SLyricImportDialog::ProcessPreviewLines()
{
	PreviewLines.Empty();

	// 限制预览内容行数
	const int32 MaxPreviewLines = 2000;

	TArray<FString> Lines;
	PreviewContent.ParseIntoArray(Lines, TEXT("\n"), false); // 保留空行

	for (int32 i = 0; i < Lines.Num() && i < MaxPreviewLines; ++i)
	{
		// 移除 \r
		Lines[i].ReplaceInline(TEXT("\r"), TEXT(""));
		PreviewLines.Add(MakeShared<FString>(Lines[i]));
	}

	if (Lines.Num() > MaxPreviewLines)
	{
		PreviewLines.Add(MakeShared<FString>(TEXT("... (文件过长，仅显示前 2000 行)")));
	}

	if (PreviewListView.IsValid())
	{
		PreviewListView->RequestListRefresh();
	}
}

TSharedRef<ITableRow> SLyricImportDialog::OnGenerateRowForPreview(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	// 简单的颜色逻辑：
	// 带 [] 的通常是 LRC 标签 -> 绿色
	// 带 {} 的可能是 ASS/特效 -> 蓝色
	// 空行 -> 透明
	// 其他 -> 白色

	FLinearColor TextColor = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
	FString LineText = *Item;

	if (LineText.TrimStartAndEnd().IsEmpty())
	{
		// 空行
	}
	else if (LineText.StartsWith(TEXT("[")))
	{
		TextColor = FLinearColor(0.4f, 1.0f, 0.4f, 1.0f); // 亮绿色
	}
	else if (LineText.StartsWith(TEXT("Dialogue:")))
	{
		TextColor = FLinearColor(0.4f, 0.8f, 1.0f, 1.0f); // 浅蓝色 (ASS)
	}
	else if (LineText.Contains(TEXT("-->")))
	{
		TextColor = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f); // 金色 (SRT 时间轴)
	}

	// 获取行索引以应用交替颜色
	const int32 Index = PreviewLines.Find(Item);
	const bool bIsOddRow = (Index % 2 != 0);

	// 奇数行给予一个微弱的背景色，偶数行保持透明
	// 使用白色带极低透明度，这样在深色背景下是提亮
	FLinearColor RowBackgroundColor = bIsOddRow ? FLinearColor(1.0f, 1.0f, 1.0f, 0.04f) : FLinearColor::Transparent;

	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
	                                                       .Padding(0) // 让 Border 填满，不留内边距
	                                                       .Style(&FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row"))
		[
			SNew(SBorder)
			             .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")) // 使用纯色笔刷
			             .BorderBackgroundColor(RowBackgroundColor) // 设置背景颜色
			             .Padding(FMargin(4, 1)) // 内容内边距
			[
				SNew(STextBlock)
				                .Text(FText::FromString(*Item))
				                .Font(FCoreStyle::GetDefaultFontStyle("Mono", 9)) // 等宽字体对齐更好
				                .ColorAndOpacity(TextColor)
				// 重要：禁止自动换行，允许父容器 ScrollBox 处理横向滚动
				                .AutoWrapText(false)
			]
		];
}

FReply SLyricImportDialog::OnImportClicked()
{
	// 确保至少有一个分组
	if (FileFormat != TEXT("ass"))
	{
		if (ParserOptions.IsEmpty())
		{
			ParserOptions.Clear();
			ParserOptions += EDreamMusicLyricTextRole::Lyric;
		}
	}

	bShouldImport = true;
	if (TSharedPtr<SWindow> Window = ParentWindow.Pin())
	{
		Window->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FReply SLyricImportDialog::OnCancelClicked()
{
	bShouldImport = false;
	if (TSharedPtr<SWindow> Window = ParentWindow.Pin())
	{
		Window->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FReply SLyricImportDialog::OnToggleAdvancedOptions()
{
	bShowAdvancedOptions = !bShowAdvancedOptions;
	return FReply::Handled();
}

void SLyricImportDialog::OnFallbackRoleSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection.IsValid())
	{
		if (*NewSelection == TEXT("Lyric"))
		{
			ParserOptions.FallbackRole = EDreamMusicLyricTextRole::Lyric;
		}
		else if (*NewSelection == TEXT("Romanization"))
		{
			ParserOptions.FallbackRole = EDreamMusicLyricTextRole::Romanization;
		}
		else if (*NewSelection == TEXT("Translation"))
		{
			ParserOptions.FallbackRole = EDreamMusicLyricTextRole::Translation;
		}
	}
	UpdateParserOptions();
}

TSharedRef<SWidget> SLyricImportDialog::MakeFallbackRoleWidget(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock).Text(FText::FromString(*InOption));
}

FText SLyricImportDialog::GetFallbackRoleText() const
{
	return GetRoleEnumText(ParserOptions.FallbackRole);
}

void SLyricImportDialog::UpdateParserOptions()
{
	RefreshGroupingSequenceList();
}

void SLyricImportDialog::RefreshGroupingSequenceList()
{
	if (!GroupingSequenceList.IsValid())
	{
		return;
	}

	TWeakPtr<SLyricImportDialog> mWeakThis = SharedThis(this);
	GroupingSequenceList->ClearChildren();

	// 遍历所有分组
	for (int32 i = 0; i < ParserOptions.GroupingSequence.Num(); ++i)
	{
		const FDreamLyricParserOptionGroup& Group = ParserOptions.GroupingSequence[i];
		int32 GroupIndex = i;

		// 构建该组下的角色列表
		TSharedPtr<SVerticalBox> RolesVerticalBox;
		SAssignNew(RolesVerticalBox, SVerticalBox);

		// 角色列表头部
		if (Group.Roles.Num() == 0)
		{
			RolesVerticalBox->AddSlot()
			                .AutoHeight()
			                .Padding(20, 4, 0, 4)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("LyricImport", "NoRoles", "No roles in this group"))
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
				.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f))
			];
		}
		else
		{
			for (int32 r = 0; r < Group.Roles.Num(); ++r)
			{
				EDreamMusicLyricTextRole Role = Group.Roles[r];
				int32 RoleIndex = r;

				RolesVerticalBox->AddSlot()
				                .AutoHeight()
				                .Padding(20, 2, 0, 2)
				[
					SNew(SHorizontalBox)
					// 树状图连接线 (简单的视觉缩进)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0, 0, 8, 0)
					[
						SNew(STextBlock)
						                .Text(FText::FromString(TEXT("└─"))) // 简单的 ASCII 风格树线
						                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
						                .ColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f))
					]
					// 角色名称
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(GetRoleEnumText(Role))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					]
					// Role 上移按钮
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(4, 0)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("↑")))
						.IsEnabled(r > 0)
						.OnClicked_Lambda([mWeakThis, GroupIndex, RoleIndex]()
						{
							if (auto Pinned = mWeakThis.Pin())
								return Pinned->OnMoveRoleUpClicked(GroupIndex, RoleIndex);
							return FReply::Handled();
						})
						.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
						.ContentPadding(FMargin(4, 1))
					]
					// Role 下移按钮
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0, 0, 4, 0)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("↓")))
						.IsEnabled(r < Group.Roles.Num() - 1)
						.OnClicked_Lambda([mWeakThis, GroupIndex, RoleIndex]()
						{
							if (auto Pinned = mWeakThis.Pin())
								return Pinned->OnMoveRoleDownClicked(GroupIndex, RoleIndex);
							return FReply::Handled();
						})
						.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
						.ContentPadding(FMargin(4, 1))
					]
					// 移除角色按钮
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("×")))
						.ToolTipText(NSLOCTEXT("LyricImport", "RemoveRole", "Remove Role"))
						.OnClicked_Lambda([mWeakThis, GroupIndex, RoleIndex]()
						{
							if (auto Pinned = mWeakThis.Pin())
								return Pinned->OnRemoveRoleClicked(GroupIndex, RoleIndex);
							return FReply::Handled();
						})
						.ContentPadding(FMargin(4, 1))
						.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("HoverHintOnly"))
					]
				];
			}
		}

		// 构建组的完整 UI
		GroupingSequenceList->AddSlot()
		                    .AutoHeight()
		                    .Padding(0, 0, 0, 8)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
			.Padding(4)
			[
				SNew(SVerticalBox)
				// Group Header (标题行)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4)
				[
					SNew(SHorizontalBox)
					// Group 序号
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0, 0, 8, 0)
					[
						SNew(SBorder)
						.BorderBackgroundColor(FLinearColor(0.0f, 0.5f, 0.8f, 1.0f))
						.Padding(FMargin(6, 2))
						[
							SNew(STextBlock)
							.Text(FText::FromString(FString::Printf(TEXT("%d"), i + 1)))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
					// 标题 "Group"
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Group")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
					]
					// 添加 Role 按钮 (Combo Button)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(4, 0)
					[
						SNew(SComboButton)
						.ContentPadding(FMargin(6, 2))
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("+ Role")))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
						]
						.OnGetMenuContent_Lambda([mWeakThis, GroupIndex]()
						{
							if (auto Pinned = mWeakThis.Pin())
								return Pinned->MakeRolePickerMenu(GroupIndex);
							return SNullWidget::NullWidget;
						})
					]
					// 上移 Group
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("↑")))
						.IsEnabled(i > 0)
						.OnClicked_Lambda([mWeakThis, GroupIndex]()
						{
							if (auto Pinned = mWeakThis.Pin())
								return Pinned->OnMoveGroupUpClicked(GroupIndex);
							return FReply::Handled();
						})
					]
					// 下移 Group
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("↓")))
						.IsEnabled(i < ParserOptions.GroupingSequence.Num() - 1)
						.OnClicked_Lambda([mWeakThis, GroupIndex]()
						{
							if (auto Pinned = mWeakThis.Pin())
								return Pinned->OnMoveGroupDownClicked(GroupIndex);
							return FReply::Handled();
						})
					]
					// 删除 Group
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(8, 0, 0, 0)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("×")))
						.ToolTipText(NSLOCTEXT("LyricImport", "RemoveGroup", "Remove Group"))
						.OnClicked_Lambda([mWeakThis, GroupIndex]()
						{
							if (auto Pinned = mWeakThis.Pin())
								return Pinned->OnRemoveGroupClicked(GroupIndex);
							return FReply::Handled();
						})
						.ContentPadding(FMargin(4, 2))
					]
				]
				// Roles 列表
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 4, 0, 0)
				[
					RolesVerticalBox.ToSharedRef()
				]
			]
		];
	}
}

TSharedRef<SWidget> SLyricImportDialog::MakeRolePickerMenu(int32 GroupIndex)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	auto AddRoleAction = [this, GroupIndex](EDreamMusicLyricTextRole Role)
	{
		OnAddRoleToGroup(GroupIndex, Role);
	};

	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("LyricImport", "AddLyric", "Lyric"),
		FText::GetEmpty(),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(AddRoleAction, EDreamMusicLyricTextRole::Lyric))
	);

	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("LyricImport", "AddRomanization", "Romanization"),
		FText::GetEmpty(),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(AddRoleAction, EDreamMusicLyricTextRole::Romanization))
	);

	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("LyricImport", "AddTranslation", "Translation"),
		FText::GetEmpty(),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda(AddRoleAction, EDreamMusicLyricTextRole::Translation))
	);

	return MenuBuilder.MakeWidget();
}

FReply SLyricImportDialog::OnAddGroupClicked()
{
	// 添加一个新的空组，或者默认带一个 Lyric 的组
	FDreamLyricParserOptionGroup NewGroup(EDreamMusicLyricTextRole::Lyric);
	ParserOptions.GroupingSequence.Add(NewGroup);
	RefreshGroupingSequenceList();
	return FReply::Handled();
}

FReply SLyricImportDialog::OnRemoveGroupClicked(int32 Index)
{
	if (ParserOptions.GroupingSequence.IsValidIndex(Index))
	{
		ParserOptions.GroupingSequence.RemoveAt(Index);
		RefreshGroupingSequenceList();
	}
	return FReply::Handled();
}

FReply SLyricImportDialog::OnMoveGroupUpClicked(int32 Index)
{
	if (Index > 0 && Index < ParserOptions.GroupingSequence.Num())
	{
		ParserOptions.GroupingSequence.Swap(Index, Index - 1);
		RefreshGroupingSequenceList();
	}
	return FReply::Handled();
}

FReply SLyricImportDialog::OnMoveGroupDownClicked(int32 Index)
{
	if (Index >= 0 && Index < ParserOptions.GroupingSequence.Num() - 1)
	{
		ParserOptions.GroupingSequence.Swap(Index, Index + 1);
		RefreshGroupingSequenceList();
	}
	return FReply::Handled();
}

void SLyricImportDialog::OnAddRoleToGroup(int32 GroupIndex, EDreamMusicLyricTextRole Role)
{
	if (ParserOptions.GroupingSequence.IsValidIndex(GroupIndex))
	{
		// 允许重复角色？通常解析器可能需要顺序，所以我们允许添加
		ParserOptions.GroupingSequence[GroupIndex].Roles.Add(Role);
		RefreshGroupingSequenceList();
	}
}

FReply SLyricImportDialog::OnRemoveRoleClicked(int32 GroupIndex, int32 RoleIndex)
{
	if (ParserOptions.GroupingSequence.IsValidIndex(GroupIndex))
	{
		auto& Group = ParserOptions.GroupingSequence[GroupIndex];
		if (Group.Roles.IsValidIndex(RoleIndex))
		{
			Group.Roles.RemoveAt(RoleIndex);
			RefreshGroupingSequenceList();
		}
	}
	return FReply::Handled();
}

FReply SLyricImportDialog::OnMoveRoleUpClicked(int32 GroupIndex, int32 RoleIndex)
{
	if (ParserOptions.GroupingSequence.IsValidIndex(GroupIndex))
	{
		auto& Group = ParserOptions.GroupingSequence[GroupIndex];
		if (RoleIndex > 0 && RoleIndex < Group.Roles.Num())
		{
			Group.Roles.Swap(RoleIndex, RoleIndex - 1);
			RefreshGroupingSequenceList();
		}
	}
	return FReply::Handled();
}

FReply SLyricImportDialog::OnMoveRoleDownClicked(int32 GroupIndex, int32 RoleIndex)
{
	if (ParserOptions.GroupingSequence.IsValidIndex(GroupIndex))
	{
		auto& Group = ParserOptions.GroupingSequence[GroupIndex];
		if (RoleIndex >= 0 && RoleIndex < Group.Roles.Num() - 1)
		{
			Group.Roles.Swap(RoleIndex, RoleIndex + 1);
			RefreshGroupingSequenceList();
		}
	}
	return FReply::Handled();
}

FText SLyricImportDialog::GetRoleEnumText(EDreamMusicLyricTextRole Role) const
{
	switch (Role)
	{
	case EDreamMusicLyricTextRole::Lyric:
		return NSLOCTEXT("LyricImport", "LyricRole", "🎵 Lyric");
	case EDreamMusicLyricTextRole::Romanization:
		return NSLOCTEXT("LyricImport", "RomanizationRole", "🔤 Romanization");
	case EDreamMusicLyricTextRole::Translation:
		return NSLOCTEXT("LyricImport", "TranslationRole", "🌐 Translation");
	default:
		return NSLOCTEXT("LyricImport", "UnknownRole", "Unknown");
	}
}
