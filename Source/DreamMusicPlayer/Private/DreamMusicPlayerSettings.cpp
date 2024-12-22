// Copyright © Dream Moon Studio . Dream Moon All rights reserved


#include "DreamMusicPlayerSettings.h"

UDreamMusicPlayerSettings::UDreamMusicPlayerSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterialAsset(TEXT("Material'/DreamMusicPlayer/Asset/Material/M_KMeansTexture.M_KMeansTexture'"));
    
	if (DefaultMaterialAsset.Succeeded())
	{
		KMeansTextureMaterial = DefaultMaterialAsset.Object;
	}
}
