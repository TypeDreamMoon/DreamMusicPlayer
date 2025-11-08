#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DreamMusicPlayerCommon.h"
#include "DreamLyricGroupWrapper.generated.h"

/**
 * @brief 临时包装器类，用于在 Detail 视图中编辑单个歌词组
 */
UCLASS()
class DREAMMUSICPLAYEREDITOR_API UDreamLyricGroupWrapper : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	FDreamMusicLyricGroup Group;
};


