#include "Music/DreamMusicImportDialog.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "DreamMusicImportDialog"

void SDreamMusicImportDialog::Construct(const FArguments& InArgs)
{
	// 定义通用的 Label 样式
	auto CreateLabel = [](const FText& Text) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
		                       .Text(Text)
		                       .ColorAndOpacity(FSlateColor::UseSubduedForeground()) // 使用 UE 默认的次级文字颜色
		                       .Font(FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont"));
	};

	// 基础 UI 结构
	ChildSlot
	[
		SNew(SBorder)
		             .BorderImage(FAppStyle::Get().GetBrush("Brushes.Panel")) // 使用编辑器面板背景
		             .Padding(0.0f)
		[
			SNew(SVerticalBox)

			// --- 1. 标题栏 ---
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::Get().GetBrush("Brushes.Header"))
				.Padding(FMargin(24.0f, 12.0f))
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DialogTitle", "Music Asset Import Settings"))
					.Font(FAppStyle::Get().GetFontStyle("HeadingExtraSmall"))
					.ColorAndOpacity(FLinearColor::White)
				]
			]

			// --- 2. 主内容区域 ---
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(24.0f)
			[
				SNew(SHorizontalBox)

				// 左侧：封面预览 (卡片式设计)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 24, 0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.WidthOverride(220)
						.HeightOverride(220)
						[
							SNew(SBorder)
							             .BorderImage(FAppStyle::Get().GetBrush("ToolPanel.GroupBorder")) // 深色凹陷背景
							             .Padding(10.0f)
							[
								SNew(SBorder) // 图片边框
								.BorderImage(FAppStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f))
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SOverlay)

									// 默认占位图 (当没有封面时显示)
									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(SImage)
										            .Image(FAppStyle::Get().GetBrush("Icons.Layout")) // 临时使用一个图标作为占位
										            .ColorAndOpacity(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
										            .RenderTransform(FSlateRenderTransform(1.5f)) // 放大一点
									]

									// 实际封面
									+ SOverlay::Slot()
									.HAlign(HAlign_Fill)
									.VAlign(VAlign_Fill)
									[
										SAssignNew(CoverImageWidget, SImage)
									]
								]
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 12, 0, 0)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("CoverPreview", "Cover Art Preview"))
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
						.Font(FAppStyle::Get().GetFontStyle("SmallFont"))
					]
				]

				// 右侧：元数据编辑 (GridPanel 对齐)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SGridPanel)
					.FillColumn(1, 1.0f) // 第二列（输入框）自动填充剩余宽度

					// -- Row 0: Title --
					+ SGridPanel::Slot(0, 0).Padding(0, 8).VAlign(VAlign_Center)
					[
						CreateLabel(LOCTEXT("LabelTitle", "Title"))
					]
					+ SGridPanel::Slot(1, 0).Padding(16, 0, 0, 8)
					[
						SAssignNew(TitleTextBox, SEditableTextBox)
					]

					// -- Row 1: Artist --
					+ SGridPanel::Slot(0, 1).Padding(0, 8).VAlign(VAlign_Center)
					[
						CreateLabel(LOCTEXT("LabelArtist", "Artist"))
					]
					+ SGridPanel::Slot(1, 1).Padding(16, 0, 0, 8)
					[
						SAssignNew(ArtistTextBox, SEditableTextBox)
					]

					// -- Row 2: Album --
					+ SGridPanel::Slot(0, 2).Padding(0, 8).VAlign(VAlign_Center)
					[
						CreateLabel(LOCTEXT("LabelAlbum", "Album"))
					]
					+ SGridPanel::Slot(1, 2).Padding(16, 0, 0, 8)
					[
						SAssignNew(AlbumTextBox, SEditableTextBox)
					]

					// -- Row 3: Genre --
					+ SGridPanel::Slot(0, 3).Padding(0, 8).VAlign(VAlign_Center)
					[
						CreateLabel(LOCTEXT("LabelGenre", "Genre"))
					]
					+ SGridPanel::Slot(1, 3).Padding(16, 0, 0, 8)
					[
						SAssignNew(GenreTextBox, SEditableTextBox)
					]

					// -- Row 4: Year (Short field) --
					+ SGridPanel::Slot(0, 4).Padding(0, 8).VAlign(VAlign_Center)
					[
						CreateLabel(LOCTEXT("LabelYear", "Year"))
					]
					+ SGridPanel::Slot(1, 4).Padding(16, 0, 0, 8)
					[
						SNew(SBox)
						          .MaxDesiredWidth(100.0f) // 年份不需要太宽
						          .HAlign(HAlign_Left)
						[
							SAssignNew(YearTextBox, SEditableTextBox)
						]
					]
				]
			]

			// --- 分割线 ---
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 16)
			[
				SNew(SSeparator)
				.Orientation(Orient_Horizontal)
			]

			// --- 3. 底部按钮栏 ---
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(16, 0, 16, 16)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 8, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("BtnImport", "Import"))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(20, 4))
					.OnClicked(this, &SDreamMusicImportDialog::OnImportClicked)
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("PrimaryButton")) // 蓝色高亮按钮
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("BtnCancel", "Cancel"))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(20, 4))
					.OnClicked(this, &SDreamMusicImportDialog::OnCancelClicked)
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button")) // 普通按钮
				]
			]
		]
	];
}

void SDreamMusicImportDialog::InitFromTag(const FDreamMusicTag& InTag, const TArray<uint8>& InCoverData, const FString& InFilePath)
{
	OriginalTag = InTag;
	RawCoverData = InCoverData;
	SourceFilePath = InFilePath;

	// 填充 UI
	if (TitleTextBox) TitleTextBox->SetText(FText::FromString(InTag.Title));
	if (ArtistTextBox) ArtistTextBox->SetText(FText::FromString(InTag.Artist));
	if (AlbumTextBox) AlbumTextBox->SetText(FText::FromString(InTag.Album));
	if (GenreTextBox) GenreTextBox->SetText(FText::FromString(InTag.Genre));

	// Year 处理：如果是 0 则留空，或者显示 0
	if (InTag.Year > 0)
	{
		if (YearTextBox) YearTextBox->SetText(FText::FromString(FString::FromInt(InTag.Year)));
	}
	else
	{
		if (YearTextBox) YearTextBox->SetText(FText::GetEmpty());
	}

	// 处理封面预览
	UpdateCoverPreview();
}

void SDreamMusicImportDialog::UpdateCoverPreview()
{
	// 默认先隐藏 Image，除非成功加载
	if (CoverImageWidget.IsValid())
	{
		CoverImageWidget->SetColorAndOpacity(FLinearColor::Transparent);
	}

	if (RawCoverData.Num() > 0)
	{
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		EImageFormat Format = ImageWrapperModule.DetectImageFormat(RawCoverData.GetData(), RawCoverData.Num());

		if (Format != EImageFormat::Invalid)
		{
			TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);
			if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawCoverData.GetData(), RawCoverData.Num()))
			{
				TArray<uint8> UncompressedBGRA;
				if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
				{
					// 创建临时 Texture 用于预览
					PreviewTexture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
					if (PreviewTexture)
					{
						void* TextureData = PreviewTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
						FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
						PreviewTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
						PreviewTexture->UpdateResource();

						// 创建 Slate Brush
						CoverBrush = MakeShareable(new FSlateBrush());
						CoverBrush->SetResourceObject(PreviewTexture);
						CoverBrush->ImageSize = FVector2D(200, 200);

						if (CoverImageWidget.IsValid())
						{
							CoverImageWidget->SetImage(CoverBrush.Get());
							CoverImageWidget->SetColorAndOpacity(FLinearColor::White); // 加载成功，显示
						}
					}
				}
			}
		}
	}
}

FReply SDreamMusicImportDialog::OnImportClicked()
{
	bShouldImport = true;
	if (ParentWindow.IsValid())
	{
		ParentWindow.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FReply SDreamMusicImportDialog::OnCancelClicked()
{
	bShouldImport = false;
	if (ParentWindow.IsValid())
	{
		ParentWindow.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FDreamMusicTag SDreamMusicImportDialog::GetResultTag() const
{
	FDreamMusicTag Result = OriginalTag;

	// 从 UI 获取更新的值
	if (TitleTextBox) Result.Title = TitleTextBox->GetText().ToString();
	if (ArtistTextBox) Result.Artist = ArtistTextBox->GetText().ToString();
	if (AlbumTextBox) Result.Album = AlbumTextBox->GetText().ToString();
	if (GenreTextBox) Result.Genre = GenreTextBox->GetText().ToString();

	if (YearTextBox)
	{
		FString YearStr = YearTextBox->GetText().ToString();
		Result.Year = YearStr.IsNumeric() ? FCString::Atoi(*YearStr) : 0;
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
