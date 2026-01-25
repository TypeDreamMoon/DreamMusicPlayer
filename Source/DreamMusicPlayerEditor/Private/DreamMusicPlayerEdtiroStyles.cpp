#include "DreamMusicPlayerEditorStyles.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

FName FDreamMusicPlayerEditorStyles::StyleName()
{
	return DREAMMUSICPLAYEREDITOR_STYLE_NAME;
}

void FDreamMusicPlayerEditorStyles::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
	}
}

void FDreamMusicPlayerEditorStyles::Register()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FDreamMusicPlayerEditorStyles::Unregister()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

TSharedPtr<ISlateStyle> FDreamMusicPlayerEditorStyles::Get()
{
	return StyleInstance;
}

#define IMAGE_BRUSH( RelativePath, ... ) \
new FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

#define SET_STYLE_CLASS_THUMB(ObjectName, FileName, ...) \
Style->Set(*FString::Printf(TEXT("ClassThumbnail.%s"), TEXT(ObjectName)), IMAGE_BRUSH(FileName, __VA_ARGS__));

#define SET_STYLE_CLASS_ICON(ObjectName, FileName, ...) \
Style->Set(*FString::Printf(TEXT("ClassIcon.%s"), TEXT(ObjectName)), IMAGE_BRUSH(FileName, __VA_ARGS__));

#define SET_STYLE_COMMAND_ICON(CommandName, FileName, ...) \
Style->Set(*FString::Printf(TEXT("DreamMusicPlayer.%s"), TEXT(CommandName)), IMAGE_BRUSH(FileName, __VA_ARGS__));

const FVector2D Icon16x16(16.f, 16.f);
const FVector2D Icon20x20(20.f, 20.f);
const FVector2D Icon40x40(40.f, 40.f);


TSharedRef<FSlateStyleSet> FDreamMusicPlayerEditorStyles::Create()
{
	TSharedRef<FSlateStyleSet> Style(MakeShareable(new FSlateStyleSet(StyleName())));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("DreamMusicPlayer")->GetBaseDir() / TEXT("Resources"));

	SET_STYLE_CLASS_ICON("DreamLyricAsset", "/Icons/Icon_Lyric", Icon16x16);
	SET_STYLE_CLASS_THUMB("DreamLyricAsset", "/Icons/Icon_Lyric", Icon40x40);

	SET_STYLE_CLASS_ICON("DreamMusicPlayerComponent", "/Icons/Icon_MusicPlayer", Icon16x16);
	SET_STYLE_CLASS_THUMB("DreamMusicPlayerComponent", "/Icons/Icon_MusicPlayer", Icon40x40);

	Style->Set("DreamToolkit", IMAGE_BRUSH("Images/TOOLKIT", FVector2D(320.f, 568.f)));
	Style->Set("DreamDev", IMAGE_BRUSH("Images/DREAMDEV", FVector2D(500.f, 160.f)));

	return Style;
}


TSharedPtr<FSlateStyleSet> FDreamMusicPlayerEditorStyles::StyleInstance = nullptr;
