#include "LyricImportDialog.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyle.h"
#include "Styling/AppStyle.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

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
		.Padding(10.0f)
		[
			SNew(SBox)
			.WidthOverride(450.0f)
			[
				SNew(SVerticalBox)
				// 标题
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 10)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() 
					{ 
						if (FileFormat == TEXT("lrc"))
						{
							return FText::FromString(TEXT("Import LRC File"));
						}
						else if (FileFormat == TEXT("ass"))
						{
							return FText::FromString(TEXT("Import ASS File"));
						}
						else if (FileFormat == TEXT("srt"))
						{
							return FText::FromString(TEXT("Import SRT File"));
						}
						return FText::FromString(TEXT("Import Lyric File"));
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				]
				// 描述
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 10)
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
				]
				// LRC 模式选择（仅 LRC 文件显示）
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 5)
				[
					SNew(SCheckBox)
					.Visibility_Lambda([this]() { return FileFormat == TEXT("lrc") ? EVisibility::Visible : EVisibility::Collapsed; })
					.IsChecked_Lambda([this]() { return SelectedMode == ELrcImportMode::ESLyric ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { if (NewState == ECheckBoxState::Checked) SelectedMode = ELrcImportMode::ESLyric; })
					.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("RadioButton"))
					.Content()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("LyricImport", "ESLyric", "ESLyric"))
					]
				]
				// WordByWord 选项（仅 LRC 文件显示）
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 5)
				[
					SNew(SCheckBox)
					.Visibility_Lambda([this]() { return FileFormat == TEXT("lrc") ? EVisibility::Visible : EVisibility::Collapsed; })
					.IsChecked_Lambda([this]() { return SelectedMode == ELrcImportMode::WordByWord ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { if (NewState == ECheckBoxState::Checked) SelectedMode = ELrcImportMode::WordByWord; })
					.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("RadioButton"))
					.Content()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("LyricImport", "WordByWord", "WordByWord"))
					]
				]
				// LineByLine 选项（仅 LRC 文件显示）
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 5)
				[
					SNew(SCheckBox)
					.Visibility_Lambda([this]() { return FileFormat == TEXT("lrc") ? EVisibility::Visible : EVisibility::Collapsed; })
					.IsChecked_Lambda([this]() { return SelectedMode == ELrcImportMode::LineByLine ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { if (NewState == ECheckBoxState::Checked) SelectedMode = ELrcImportMode::LineByLine; })
					.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("RadioButton"))
					.Content()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("LyricImport", "LineByLine", "LineByLine"))
					]
				]
				// 分隔线
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 10, 0, 5)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("Menu.Separator"))
					.Padding(0)
				]
				// 高级选项按钮（对于非 LRC 文件，默认展开）
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 5)
				[
					SNew(SButton)
					.Text_Lambda([this]() { return bShowAdvancedOptions ? NSLOCTEXT("LyricImport", "HideAdvanced", "Hide Advanced Options") : NSLOCTEXT("LyricImport", "ShowAdvanced", "Show Advanced Options"); })
					.OnClicked(this, &SLyricImportDialog::OnToggleAdvancedOptions)
					.ContentPadding(FMargin(5, 2))
				]
				// 高级选项区域
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(10, 5, 0, 5)
				[
					SNew(SVerticalBox)
					// 分组序列标题
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 5)
					[
						SNew(STextBlock)
						.Visibility_Lambda([this]() { return bShowAdvancedOptions ? EVisibility::Visible : EVisibility::Collapsed; })
						.Text(NSLOCTEXT("LyricImport", "GroupingSequence", "Select roles to include (order matters):"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					]
					// 角色选择复选框区域
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(5, 2)
					[
						SNew(SHorizontalBox)
						.Visibility_Lambda([this]() { return bShowAdvancedOptions ? EVisibility::Visible : EVisibility::Collapsed; })
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(LyricRoleCheckBox, SCheckBox)
							.IsChecked_Lambda([this]() 
							{ 
								return ParserOptions.GroupingSequence.Contains(ELyricTextRole::Lyric) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; 
							})
							.OnCheckStateChanged(this, &SLyricImportDialog::OnRoleCheckBoxChanged, ELyricTextRole::Lyric)
							.Content()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("LyricImport", "LyricRole", "Lyric"))
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10, 0, 0, 0)
						[
							SAssignNew(RomanizationRoleCheckBox, SCheckBox)
							.IsChecked_Lambda([this]() 
							{ 
								return ParserOptions.GroupingSequence.Contains(ELyricTextRole::Romanization) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; 
							})
							.OnCheckStateChanged(this, &SLyricImportDialog::OnRoleCheckBoxChanged, ELyricTextRole::Romanization)
							.Content()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("LyricImport", "RomanizationRole", "Romanization"))
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10, 0, 0, 0)
						[
							SAssignNew(TranslationRoleCheckBox, SCheckBox)
							.IsChecked_Lambda([this]() 
							{ 
								return ParserOptions.GroupingSequence.Contains(ELyricTextRole::Translation) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; 
							})
							.OnCheckStateChanged(this, &SLyricImportDialog::OnRoleCheckBoxChanged, ELyricTextRole::Translation)
							.Content()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("LyricImport", "TranslationRole", "Translation"))
							]
						]
					]
					// 分组序列顺序列表标题
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 10, 0, 5)
					[
						SNew(STextBlock)
						.Visibility_Lambda([this]() { return bShowAdvancedOptions && ParserOptions.GroupingSequence.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
						.Text(NSLOCTEXT("LyricImport", "SequenceOrder", "Current Sequence (use ↑↓ to reorder, × to remove):"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					]
					// 分组序列顺序列表
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(5, 2)
					[
						SAssignNew(GroupingSequenceList, SVerticalBox)
						.Visibility_Lambda([this]() { return bShowAdvancedOptions && ParserOptions.GroupingSequence.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
					]
					// 回退角色标题
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 10, 0, 5)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("LyricImport", "FallbackRole", "Fallback Role:"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					]
					// 回退角色下拉框
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(5, 2)
					[
						SAssignNew(FallbackRoleComboBox, SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&FallbackRoleOptions)
						.InitiallySelectedItem(FallbackRoleOptions[0])
						.OnGenerateWidget(this, &SLyricImportDialog::MakeFallbackRoleWidget)
						.OnSelectionChanged(this, &SLyricImportDialog::OnFallbackRoleSelectionChanged)
						[
							SNew(STextBlock)
							.Text(this, &SLyricImportDialog::GetFallbackRoleText)
						]
					]
				]
				// 按钮区域
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 10, 0, 0)
				[
					SNew(SUniformGridPanel)
					.SlotPadding(5)
					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.Text(NSLOCTEXT("LyricImport", "Import", "Import"))
						.OnClicked(this, &SLyricImportDialog::OnImportClicked)
					]
					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.Text(NSLOCTEXT("LyricImport", "Cancel", "Cancel"))
						.OnClicked(this, &SLyricImportDialog::OnCancelClicked)
					]
				]
			]
		]
	];
	
	// 初始化分组序列列表显示
	RefreshGroupingSequenceList();
}

FReply SLyricImportDialog::OnImportClicked()
{
	// 确保至少有一个角色在序列中（不刷新UI，因为窗口即将关闭）
	if (ParserOptions.GroupingSequence.Num() == 0)
	{
		ParserOptions.GroupingSequence.Add(ELyricTextRole::Lyric);
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
			ParserOptions.FallbackRole = ELyricTextRole::Lyric;
		}
		else if (*NewSelection == TEXT("Romanization"))
		{
			ParserOptions.FallbackRole = ELyricTextRole::Romanization;
		}
		else if (*NewSelection == TEXT("Translation"))
		{
			ParserOptions.FallbackRole = ELyricTextRole::Translation;
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
	case ELyricTextRole::Lyric:
		return FText::FromString(TEXT("Lyric"));
	case ELyricTextRole::Romanization:
		return FText::FromString(TEXT("Romanization"));
	case ELyricTextRole::Translation:
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
		ParserOptions.GroupingSequence.Add(ELyricTextRole::Lyric);
	}
	// 刷新分组序列列表显示
	RefreshGroupingSequenceList();
}

void SLyricImportDialog::OnRoleCheckBoxChanged(ECheckBoxState NewState, ELyricTextRole Role)
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
		ELyricTextRole Role = ParserOptions.GroupingSequence[i];
		int32 CurrentIndex = i; // 捕获索引
		
		GroupingSequenceList->AddSlot()
			.AutoHeight()
			.Padding(2, 2)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(8, 5)
				[
					SNew(SHorizontalBox)
					// 序号
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("%d."), i + 1)))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						.MinDesiredWidth(25)
					]
					// 角色名称
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					.Padding(8, 0, 0, 0)
					[
						SNew(STextBlock)
						.Text(GetRoleDisplayText(Role))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
					]
					// 上移按钮
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5, 0, 2, 0)
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
						.ContentPadding(FMargin(8, 3))
						.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
					]
					// 下移按钮
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0, 2, 0)
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
						.ContentPadding(FMargin(8, 3))
						.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
					]
					// 移除按钮
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0, 0, 0)
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
						.ContentPadding(FMargin(8, 3))
						.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
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
		ELyricTextRole RoleToRemove = ParserOptions.GroupingSequence[Index];
		ParserOptions.GroupingSequence.RemoveAt(Index);
		
		// 更新对应的复选框状态
		if (RoleToRemove == ELyricTextRole::Lyric && LyricRoleCheckBox.IsValid())
		{
			LyricRoleCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
		}
		else if (RoleToRemove == ELyricTextRole::Romanization && RomanizationRoleCheckBox.IsValid())
		{
			RomanizationRoleCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
		}
		else if (RoleToRemove == ELyricTextRole::Translation && TranslationRoleCheckBox.IsValid())
		{
			TranslationRoleCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
		}
		
		UpdateParserOptions();
	}
	return FReply::Handled();
}

FText SLyricImportDialog::GetRoleDisplayText(ELyricTextRole Role) const
{
	switch (Role)
	{
	case ELyricTextRole::Lyric:
		return NSLOCTEXT("LyricImport", "LyricRole", "Lyric");
	case ELyricTextRole::Romanization:
		return NSLOCTEXT("LyricImport", "RomanizationRole", "Romanization");
	case ELyricTextRole::Translation:
		return NSLOCTEXT("LyricImport", "TranslationRole", "Translation");
	default:
		return FText::GetEmpty();
	}
}

int32 SLyricImportDialog::GetRoleIndexInSequence(ELyricTextRole Role) const
{
	return ParserOptions.GroupingSequence.IndexOfByKey(Role);
}
