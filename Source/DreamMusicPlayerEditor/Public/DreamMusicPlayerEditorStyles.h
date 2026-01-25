#pragma once

#define DREAMMUSICPLAYEREDITOR_STYLE_NAME "DreamMusicPlayerEditorStyle"

class DREAMMUSICPLAYEREDITOR_API FDreamMusicPlayerEditorStyles
{
public:
	static FName StyleName();
	static void Initialize();
	static void Register();
	static void Unregister();
	static TSharedRef<FSlateStyleSet> Create();
	static TSharedPtr<ISlateStyle> Get();
	static TSharedPtr<class FSlateStyleSet> StyleInstance;
};
