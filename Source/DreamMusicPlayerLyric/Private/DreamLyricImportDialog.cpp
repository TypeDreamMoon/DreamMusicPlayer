#include "DreamLyricImportDialog.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Images/SImage.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyle.h"
#include "Styling/AppStyle.h"

#include "Misc/FileHelper.h"

void SLyricImportDialog::Construct(const FArguments& InArgs)
{
	// 初始化默认解析选项
	ParserOptions = FDreamLyricParserOptions::GetDefault();

	// 对于非 LRC 文件，默认展开高级选项
	if (FileFormat != TEXT("lrc"))
	{
		bShowAdvancedOptions = true;
	}

	// 创建回退角色选项列表
	FallbackRoleOptions.Add(MakeShared<FString>(TEXT("Lyric")));
	FallbackRoleOptions.Add(MakeShared<FString>(TEXT("Romanization")));
	FallbackRoleOptions.Add(MakeShared<FString>(TEXT("Translation")));

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.3f))
		.Padding(16.0f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)
			+ SSplitter::Slot()
			.Value(0.5f)
			[
				SNew(SBox)
				.WidthOverride(520.0f)
				[
					SNew(SVerticalBox)
					// 标题区域（带背景色）
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 16)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
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
									if (FileFormat == TEXT("lrc"))
									{
										return FText::FromString(TEXT("📝 Import LRC File"));
									}
									else if (FileFormat == TEXT("ass"))
									{
										return FText::FromString(TEXT("📝 Import ASS File"));
									}
									else if (FileFormat == TEXT("srt"))
									{
										return FText::FromString(TEXT("📝 Import SRT File"));
									}
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
								.Text_Lambda([this]()
								{
									if (FileFormat == TEXT("lrc"))
									{
										return NSLOCTEXT("LyricImport", "Description", "Please select the import mode for this LRC file:");
									}
									else if (FileFormat == TEXT("ass"))
									{
										return NSLOCTEXT("LyricImport", "DescriptionASS", "Please configure import options for this ASS file:");
									}
									else if (FileFormat == TEXT("srt"))
									{
										return NSLOCTEXT("LyricImport", "DescriptionSRT", "Please configure import options for this SRT file:");
									}
									return NSLOCTEXT("LyricImport", "Description", "Please configure import options:");
								})
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
								.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
							]
						]
					]
					// LRC 模式选择区域（仅 LRC 文件显示）
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 12)
					[
						SNew(SBorder)
						.Visibility_Lambda([this]() { return FileFormat == TEXT("lrc") ? EVisibility::Visible : EVisibility::Collapsed; })
						.BorderImage(FAppStyle::GetBrush("ToolPanel.LightGroupBorder"))
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
					// 高级选项区域标题
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 8, 0, 8)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("Menu.Separator"))
						.Padding(0)
					]
					// 高级选项按钮（对于非 LRC 文件，默认展开）
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
					]
					// 高级选项区域
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 8)
					[
						SNew(SBorder)
						.Visibility_Lambda([this]() { return bShowAdvancedOptions ? EVisibility::Visible : EVisibility::Collapsed; })
						.BorderImage(FAppStyle::GetBrush("ToolPanel.LightGroupBorder"))
						.Padding(12, 10)
						[
							SNew(SVerticalBox)
							// 分组序列标题
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0, 0, 0, 8)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("LyricImport", "GroupingSequence", "Select roles to include (order matters):"))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
							]
							// 角色选择复选框区域
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0, 0, 0, 10)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SAssignNew(LyricRoleCheckBox, SCheckBox)
									.IsChecked_Lambda([this]()
									{
										return ParserOptions.GroupingSequence.Contains(EDreamMusicLyricTextRole::Lyric) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
									})
									.OnCheckStateChanged(this, &SLyricImportDialog::OnRoleCheckBoxChanged, EDreamMusicLyricTextRole::Lyric)
									.Content()
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("LyricImport", "LyricRole", "🎵 Lyric"))
										.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
									]
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(12, 0, 0, 0)
								[
									SAssignNew(RomanizationRoleCheckBox, SCheckBox)
									.IsChecked_Lambda([this]()
									{
										return ParserOptions.GroupingSequence.Contains(EDreamMusicLyricTextRole::Romanization) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
									})
									.OnCheckStateChanged(this, &SLyricImportDialog::OnRoleCheckBoxChanged, EDreamMusicLyricTextRole::Romanization)
									.Content()
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("LyricImport", "RomanizationRole", "🔤 Romanization"))
										.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
									]
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(12, 0, 0, 0)
								[
									SAssignNew(TranslationRoleCheckBox, SCheckBox)
									.IsChecked_Lambda([this]()
									{
										return ParserOptions.GroupingSequence.Contains(EDreamMusicLyricTextRole::Translation) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
									})
									.OnCheckStateChanged(this, &SLyricImportDialog::OnRoleCheckBoxChanged, EDreamMusicLyricTextRole::Translation)
									.Content()
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("LyricImport", "TranslationRole", "🌐 Translation"))
										.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
									]
								]
							]
							// 分组序列顺序列表标题
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0, 8, 0, 6)
							[
								SNew(STextBlock)
								.Visibility_Lambda([this]() { return ParserOptions.GroupingSequence.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
								.Text(NSLOCTEXT("LyricImport", "SequenceOrder", "Current Sequence (use ↑↓ to reorder, × to remove):"))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
								.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
							]
							// 分组序列顺序列表（带滚动）
							+ SVerticalBox::Slot()
							.AutoHeight()
							.MaxHeight(150.0f)
							.Padding(0, 0, 0, 10)
							[
								SNew(SScrollBox)
								.Visibility_Lambda([this]() { return ParserOptions.GroupingSequence.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
								+ SScrollBox::Slot()
								[
									SAssignNew(GroupingSequenceList, SVerticalBox)
								]
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
					// 按钮区域
					+ SVerticalBox::Slot()
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
			]
			+ SSplitter::Slot()
			.Value(0.5f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.LightGroupBorder"))
				.Padding(12, 10)
				[
					SNew(SVerticalBox)
					// 预览标题
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 8)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("LyricImport", "PreviewTitle", "📄 File Preview"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
					]
					// 文件信息
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 8)
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
					// 预览内容（使用滚动框）
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SScrollBox)
						.Orientation(Orient_Vertical)
						+ SScrollBox::Slot()
						[
							SAssignNew(PreviewTextWidget, SMultiLineEditableText)
							.Text_Lambda([this]() { return FText::FromString(PreviewContent); })
							.IsReadOnly(true)
							.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
							.WrapTextAt(450.0f)
							.AutoWrapText(true)
						]
					]
				]
			]
		]
	];

	// 初始化分组序列列表显示
	RefreshGroupingSequenceList();
}

void SLyricImportDialog::LoadPreviewContent()
{
	if (!FilePath.IsEmpty() && FPaths::FileExists(FilePath))
	{
		if (FFileHelper::LoadFileToString(PreviewContent, *FilePath))
		{
			// 限制预览内容长度（避免显示过大的文件）
			const int32 MaxPreviewLength = 10000;
			if (PreviewContent.Len() > MaxPreviewLength)
			{
				PreviewContent = PreviewContent.Left(MaxPreviewLength) + TEXT("\n\n... (文件内容过长，已截断)");
			}

			// 更新预览控件
			if (PreviewTextWidget.IsValid())
			{
				PreviewTextWidget->SetText(FText::FromString(PreviewContent));
			}
		}
		else
		{
			PreviewContent = TEXT("无法读取文件内容");
		}
	}
}

FReply SLyricImportDialog::OnImportClicked()
{
	// 确保至少有一个角色在序列中（不刷新UI，因为窗口即将关闭）
	if (ParserOptions.GroupingSequence.Num() == 0)
	{
		ParserOptions.GroupingSequence.Add(EDreamMusicLyricTextRole::Lyric);
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
	switch (ParserOptions.FallbackRole)
	{
	case EDreamMusicLyricTextRole::Lyric:
		return FText::FromString(TEXT("Lyric"));
	case EDreamMusicLyricTextRole::Romanization:
		return FText::FromString(TEXT("Romanization"));
	case EDreamMusicLyricTextRole::Translation:
		return FText::FromString(TEXT("Translation"));
	default:
		return FText::FromString(TEXT("Lyric"));
	}
}

void SLyricImportDialog::UpdateParserOptions()
{
	// 确保至少有一个角色在序列中
	if (ParserOptions.GroupingSequence.Num() == 0)
	{
		ParserOptions.GroupingSequence.Add(EDreamMusicLyricTextRole::Lyric);
	}
	// 刷新分组序列列表显示
	RefreshGroupingSequenceList();
}

void SLyricImportDialog::OnRoleCheckBoxChanged(ECheckBoxState NewState, EDreamMusicLyricTextRole Role)
{
	if (NewState == ECheckBoxState::Checked)
	{
		// 如果角色不在序列中，添加到末尾
		if (!ParserOptions.GroupingSequence.Contains(Role))
		{
			ParserOptions.GroupingSequence.Add(Role);
		}
	}
	else
	{
		// 从序列中移除角色
		ParserOptions.GroupingSequence.Remove(Role);
	}
	UpdateParserOptions();
}

void SLyricImportDialog::RefreshGroupingSequenceList()
{
	if (!GroupingSequenceList.IsValid())
	{
		return;
	}

	// 使用 TWeakPtr 安全地捕获 this
	TWeakPtr<SLyricImportDialog> mWeakThis = SharedThis(this);

	// 清空现有内容
	GroupingSequenceList->ClearChildren();

	// 为序列中的每个角色创建列表项
	for (int32 i = 0; i < ParserOptions.GroupingSequence.Num(); ++i)
	{
		EDreamMusicLyricTextRole Role = ParserOptions.GroupingSequence[i];
		int32 CurrentIndex = i; // 捕获索引

		GroupingSequenceList->AddSlot()
		                    .AutoHeight()
		                    .Padding(0, 0, 0, 6)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.LightGroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f, 0.5f))
			.Padding(10, 8)
			[
				SNew(SHorizontalBox)
				// 序号徽章
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
					.BorderBackgroundColor(FLinearColor(0.2f, 0.4f, 0.6f, 0.8f))
					.Padding(6, 4)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("%d"), i + 1)))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						.ColorAndOpacity(FLinearColor::White)
						.MinDesiredWidth(20)
						.Justification(ETextJustify::Center)
					]
				]
				// 角色名称
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(10, 0, 0, 0)
				[
					SNew(STextBlock)
					.Text(GetRoleDisplayText(Role))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
					.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
				]
				// 上移按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(6, 0, 3, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("↑")))
					.ToolTipText(NSLOCTEXT("LyricImport", "MoveUp", "Move Up"))
					.IsEnabled_Lambda([CurrentIndex]() { return CurrentIndex > 0; })
					.OnClicked_Lambda([mWeakThis, CurrentIndex]()
					{
						if (TSharedPtr<SLyricImportDialog> PinnedThis = mWeakThis.Pin())
						{
							return PinnedThis->OnMoveUpClicked(CurrentIndex);
						}
						return FReply::Handled();
					})
					.ContentPadding(FMargin(6, 4))
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
				]
				// 下移按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(3, 0, 3, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("↓")))
					.ToolTipText(NSLOCTEXT("LyricImport", "MoveDown", "Move Down"))
					.IsEnabled_Lambda([mWeakThis, CurrentIndex]()
					{
						if (TSharedPtr<SLyricImportDialog> PinnedThis = mWeakThis.Pin())
						{
							return CurrentIndex < PinnedThis->ParserOptions.GroupingSequence.Num() - 1;
						}
						return false;
					})
					.OnClicked_Lambda([mWeakThis, CurrentIndex]()
					{
						if (TSharedPtr<SLyricImportDialog> PinnedThis = mWeakThis.Pin())
						{
							return PinnedThis->OnMoveDownClicked(CurrentIndex);
						}
						return FReply::Handled();
					})
					.ContentPadding(FMargin(6, 4))
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
				]
				// 移除按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(3, 0, 0, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("×")))
					.ToolTipText(NSLOCTEXT("LyricImport", "Remove", "Remove"))
					.OnClicked_Lambda([mWeakThis, CurrentIndex]()
					{
						if (TSharedPtr<SLyricImportDialog> PinnedThis = mWeakThis.Pin())
						{
							return PinnedThis->OnRemoveClicked(CurrentIndex);
						}
						return FReply::Handled();
					})
					.ContentPadding(FMargin(6, 4))
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
				]
			]
		];
	}
}

FReply SLyricImportDialog::OnMoveUpClicked(int32 Index)
{
	if (Index > 0 && Index < ParserOptions.GroupingSequence.Num())
	{
		// 交换位置
		ParserOptions.GroupingSequence.Swap(Index, Index - 1);
		RefreshGroupingSequenceList();
	}
	return FReply::Handled();
}

FReply SLyricImportDialog::OnMoveDownClicked(int32 Index)
{
	if (Index >= 0 && Index < ParserOptions.GroupingSequence.Num() - 1)
	{
		// 交换位置
		ParserOptions.GroupingSequence.Swap(Index, Index + 1);
		RefreshGroupingSequenceList();
	}
	return FReply::Handled();
}

FReply SLyricImportDialog::OnRemoveClicked(int32 Index)
{
	if (Index >= 0 && Index < ParserOptions.GroupingSequence.Num())
	{
		EDreamMusicLyricTextRole RoleToRemove = ParserOptions.GroupingSequence[Index];
		ParserOptions.GroupingSequence.RemoveAt(Index);

		// 更新对应的复选框状态
		if (RoleToRemove == EDreamMusicLyricTextRole::Lyric && LyricRoleCheckBox.IsValid())
		{
			LyricRoleCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
		}
		else if (RoleToRemove == EDreamMusicLyricTextRole::Romanization && RomanizationRoleCheckBox.IsValid())
		{
			RomanizationRoleCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
		}
		else if (RoleToRemove == EDreamMusicLyricTextRole::Translation && TranslationRoleCheckBox.IsValid())
		{
			TranslationRoleCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
		}

		UpdateParserOptions();
	}
	return FReply::Handled();
}

FText SLyricImportDialog::GetRoleDisplayText(EDreamMusicLyricTextRole Role) const
{
	switch (Role)
	{
	case EDreamMusicLyricTextRole::Lyric:
		return NSLOCTEXT("LyricImport", "LyricRole", "Lyric");
	case EDreamMusicLyricTextRole::Romanization:
		return NSLOCTEXT("LyricImport", "RomanizationRole", "Romanization");
	case EDreamMusicLyricTextRole::Translation:
		return NSLOCTEXT("LyricImport", "TranslationRole", "Translation");
	default:
		return FText::GetEmpty();
	}
}

int32 SLyricImportDialog::GetRoleIndexInSequence(EDreamMusicLyricTextRole Role) const
{
	return ParserOptions.GroupingSequence.IndexOfByKey(Role);
}
