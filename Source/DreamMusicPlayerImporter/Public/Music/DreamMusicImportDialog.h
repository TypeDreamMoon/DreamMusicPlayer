#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"
#include "DreamMusicTag.h"

class DREAMMUSICPLAYERIMPORTER_API SDreamMusicImportDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDreamMusicImportDialog)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetParentWindow(TWeakPtr<SWindow> InParentWindow) { ParentWindow = InParentWindow; }

	/** 初始化数据 */
	void InitFromTag(const FDreamMusicTag& InTag, const TArray<uint8>& InCoverData, const FString& InFilePath);

	bool ShouldImport() const { return bShouldImport; }

	/** 获取用户编辑后的标签 */
	FDreamMusicTag GetResultTag() const;

	/** 获取原始封面数据 */
	const TArray<uint8>& GetCoverData() const { return RawCoverData; }

private:
	TWeakPtr<SWindow> ParentWindow;
	bool bShouldImport = false;
	FString SourceFilePath;

	// 编辑框
	TSharedPtr<class SEditableTextBox> TitleTextBox;
	TSharedPtr<class SEditableTextBox> ArtistTextBox;
	TSharedPtr<class SEditableTextBox> AlbumTextBox;
	TSharedPtr<class SEditableTextBox> GenreTextBox;
	TSharedPtr<class SEditableTextBox> YearTextBox;
	TSharedPtr<class SEditableTextBox> TrackTextBox;

	// 预览数据
	FDreamMusicTag OriginalTag;
	TArray<uint8> RawCoverData;

	// 封面预览
	TSharedPtr<FSlateBrush> CoverBrush;
	UTexture2D* PreviewTexture = nullptr; // 用于 UI 显示的临时贴图
	TSharedPtr<SImage> CoverImageWidget;

	FReply OnImportClicked();
	FReply OnCancelClicked();

	// 辅助：生成封面预览
	void UpdateCoverPreview();
};
