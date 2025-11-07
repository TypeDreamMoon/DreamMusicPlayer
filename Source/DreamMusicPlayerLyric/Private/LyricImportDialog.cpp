#include "LyricImportDialog.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyle.h"
#include "Styling/AppStyle.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

void SLyricImportDialog::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(10.0f)
		[
			SNew(SBox)
			.WidthOverride(400.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 10)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("LyricImport", "Title", "Import LRC File"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 10)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("LyricImport", "Description", "Please select the import mode for this LRC file:"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 5)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this]() { return SelectedMode == ELrcImportMode::ESLyric ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { if (NewState == ECheckBoxState::Checked) SelectedMode = ELrcImportMode::ESLyric; })
					.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("RadioButton"))
					.Content()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("LyricImport", "ESLyric", "ESLyric"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 5)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this]() { return SelectedMode == ELrcImportMode::WordByWord ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { if (NewState == ECheckBoxState::Checked) SelectedMode = ELrcImportMode::WordByWord; })
					.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("RadioButton"))
					.Content()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("LyricImport", "WordByWord", "WordByWord"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 5)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this]() { return SelectedMode == ELrcImportMode::LineByLine ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) { if (NewState == ECheckBoxState::Checked) SelectedMode = ELrcImportMode::LineByLine; })
					.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("RadioButton"))
					.Content()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("LyricImport", "LineByLine", "LineByLine"))
					]
				]
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
}

FReply SLyricImportDialog::OnImportClicked()
{
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

