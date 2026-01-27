#pragma once

#include "CoreMinimal.h"
#include "DreamMusicFileType.generated.h"

UENUM(BlueprintType)
enum class EDreamMusicPlayerTagLibFileFormat : uint8
{
	Unknown,
	RIFF_AIFF,
	APE,
	ASF,
	DSDIFF,
	DSF,
	FLAC,
	IT,
	Matroska,
	Mod,
	MP4,
	MP3,
	MPC,
	MPEG,
	Ogg,
	Ogg_FLAC,
	Ogg_Opus,
	RIFF,
	S3M,
	Shorten,
	Ogg_Speex,
	TrueAudio,
	Vorbis,
	RIFF_WAV,
	XM
};
