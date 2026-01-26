// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamMusicPlayerTagLib.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

#include "taglib/fileref.h"
#include "taglib/tag.h"
#include "taglib/tpropertymap.h"
#include "taglib/audioproperties.h"

#include "taglib/aifffile.h"
#include "taglib/apefile.h"
#include "taglib/asfattribute.h"
#include "taglib/asffile.h"
#include "taglib/dsdifffile.h"
#include "taglib/dsffile.h"

#include "taglib/flacfile.h"
#include "taglib/itfile.h"
#include "taglib/matroskafile.h"
#include "taglib/modfile.h"
#include "taglib/mp4file.h"
#include "taglib/mpcfile.h"
#include "taglib/mpegfile.h"
#include "taglib/oggflacfile.h"
#include "taglib/opusfile.h"
#include "taglib/s3mfile.h"
#include "taglib/shortenfile.h"
#include "taglib/speexfile.h"
#include "taglib/trueaudiofile.h"
#include "taglib/vorbisfile.h"
#include "taglib/wavfile.h"
#include "taglib/xmfile.h"

#include "taglib/id3v2tag.h"
#include "taglib/attachedpictureframe.h"

#define UE_TO_TAGLIB(Str) TagLib::String(TCHAR_TO_UTF8(*Str), TagLib::String::UTF8)
#define TAGLIB_TO_UE(Str) UTF8_TO_TCHAR(Str.toCString(true))

FDreamMusicPlayerTagLib::FDreamMusicPlayerTagLib(const FString& InFilePath)
{
	// 转换路径。Windows 下通常支持宽字符，但 TagLib 构造函数在不同平台表现不同
	// 建议使用 UTF8 路径构造
#ifdef _WIN32
	const wchar_t* Path = *InFilePath;
	FileRef = new TagLib::FileRef(Path);
#else
	FileRef = new TagLib::FileRef(TCHAR_TO_UTF8(*InFilePath));
#endif

	if (FileRef && !FileRef->isNull())
	{
		Tag = FileRef->tag();
		AudioProperties = FileRef->audioProperties();
	}
}

FDreamMusicPlayerTagLib::~FDreamMusicPlayerTagLib()
{
	// FileRef 析构时会自动释放它拥有的 File 对象
	if (FileRef)
	{
		delete FileRef;
		FileRef = nullptr;
	}
	// Tag 和 AudioProperties 是由 FileRef 管理的，不需要单独 delete
}

#define CAST_RETURN(Type, Format) if (auto* file = dynamic_cast<TagLib::Type::File*>(FileRef->file())) return new FDreamMusicPlayerTagLibFileFormat(EDreamMusicPlayerTagLibFileFormat::Format, file);

FDreamMusicPlayerTagLibFileFormat FDreamMusicPlayerTagLib::DetectFileFormat() const
{
	if (!IsValid())
	{
		return FDreamMusicPlayerTagLibFileFormat::Unknown();
	}

	TagLib::File* File = FileRef->file();
	EDreamMusicPlayerTagLibFileFormat Format = EDreamMusicPlayerTagLibFileFormat::Unknown;

	// 使用 dynamic_cast 识别具体类型
	// 注意：判定顺序很重要，子类应该在父类之前判断 (虽在 TagLib 中并列较多)

	if (dynamic_cast<TagLib::MPEG::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::MPEG;
	else if (dynamic_cast<TagLib::FLAC::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::FLAC;
	else if (dynamic_cast<TagLib::Vorbis::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::Vorbis;
	else if (dynamic_cast<TagLib::RIFF::WAV::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::RIFF_WAV;
	else if (dynamic_cast<TagLib::RIFF::AIFF::File*>(File))Format = EDreamMusicPlayerTagLibFileFormat::RIFF_AIFF;
	else if (dynamic_cast<TagLib::MP4::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::MP4;
	else if (dynamic_cast<TagLib::ASF::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::ASF;
	else if (dynamic_cast<TagLib::Ogg::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::Ogg;
	else if (dynamic_cast<TagLib::APE::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::APE;
	else if (dynamic_cast<TagLib::MPC::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::MPC;
	else if (dynamic_cast<TagLib::Mod::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::Mod;
	else if (dynamic_cast<TagLib::S3M::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::S3M;
	else if (dynamic_cast<TagLib::XM::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::XM;
	else if (dynamic_cast<TagLib::IT::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::IT;
	else if (dynamic_cast<TagLib::TrueAudio::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::TrueAudio;
	else if (dynamic_cast<TagLib::Ogg::FLAC::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::Ogg_FLAC;
	else if (dynamic_cast<TagLib::Ogg::Speex::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::Ogg_Speex;
	else if (dynamic_cast<TagLib::Ogg::Opus::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::Ogg_Opus;
	else if (dynamic_cast<TagLib::DSDIFF::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::DSDIFF;
	else if (dynamic_cast<TagLib::DSF::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::DSF;
	else if (dynamic_cast<TagLib::Matroska::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::Matroska;
	else if (dynamic_cast<TagLib::Shorten::File*>(File)) Format = EDreamMusicPlayerTagLibFileFormat::Shorten;

	return FDreamMusicPlayerTagLibFileFormat(Format, File);
}

#undef CAST_RETURN

bool FDreamMusicPlayerTagLib::IsValid() const
{
	return FileRef && !FileRef->isNull() && Tag;
}

bool FDreamMusicPlayerTagLib::Save()
{
	if (IsValid())
	{
		return FileRef->save();
	}
	return false;
}

// --- Getters ---

FString FDreamMusicPlayerTagLib::GetTitle() const
{
	return IsValid() ? TAGLIB_TO_UE(Tag->title()) : FString();
}

FString FDreamMusicPlayerTagLib::GetArtist() const
{
	return IsValid() ? TAGLIB_TO_UE(Tag->artist()) : FString();
}

FString FDreamMusicPlayerTagLib::GetAlbum() const
{
	return IsValid() ? TAGLIB_TO_UE(Tag->album()) : FString();
}

FString FDreamMusicPlayerTagLib::GetGenre() const
{
	return IsValid() ? TAGLIB_TO_UE(Tag->genre()) : FString();
}

FString FDreamMusicPlayerTagLib::GetComment() const
{
	return IsValid() ? TAGLIB_TO_UE(Tag->comment()) : FString();
}

int FDreamMusicPlayerTagLib::GetYear() const
{
	return IsValid() ? Tag->year() : 0;
}

int FDreamMusicPlayerTagLib::GetTrack() const
{
	return IsValid() ? Tag->track() : 0;
}

// --- Setters ---

void FDreamMusicPlayerTagLib::SetTitle(const FString& InTitle)
{
	if (IsValid()) Tag->setTitle(UE_TO_TAGLIB(InTitle));
}

void FDreamMusicPlayerTagLib::SetArtist(const FString& InArtist)
{
	if (IsValid()) Tag->setArtist(UE_TO_TAGLIB(InArtist));
}

void FDreamMusicPlayerTagLib::SetAlbum(const FString& InAlbum)
{
	if (IsValid()) Tag->setAlbum(UE_TO_TAGLIB(InAlbum));
}

void FDreamMusicPlayerTagLib::SetGenre(const FString& InGenre)
{
	if (IsValid()) Tag->setGenre(UE_TO_TAGLIB(InGenre));
}

void FDreamMusicPlayerTagLib::SetComment(const FString& InComment)
{
	if (IsValid()) Tag->setComment(UE_TO_TAGLIB(InComment));
}

void FDreamMusicPlayerTagLib::SetYear(int InYear)
{
	if (IsValid()) Tag->setYear(InYear);
}

void FDreamMusicPlayerTagLib::SetTrack(int InTrack)
{
	if (IsValid()) Tag->setTrack(InTrack);
}

// --- Audio Properties ---

int FDreamMusicPlayerTagLib::GetDuration() const
{
	return (IsValid() && AudioProperties) ? AudioProperties->lengthInSeconds() : 0;
}

int FDreamMusicPlayerTagLib::GetBitrate() const
{
	return (IsValid() && AudioProperties) ? AudioProperties->bitrate() : 0;
}

int FDreamMusicPlayerTagLib::GetSampleRate() const
{
	return (IsValid() && AudioProperties) ? AudioProperties->sampleRate() : 0;
}

int FDreamMusicPlayerTagLib::GetChannels() const
{
	return (IsValid() && AudioProperties) ? AudioProperties->channels() : 0;
}

// --- Advanced Properties ---

TMap<FString, FString> FDreamMusicPlayerTagLib::GetProperties() const
{
	TMap<FString, FString> Props;
	if (!IsValid()) return Props;

	// TagLib 的 properties() 返回一个 Map<String, StringList>
	TagLib::PropertyMap TagLibProps = FileRef->file()->properties();

	for (auto It = TagLibProps.begin(); It != TagLibProps.end(); ++It)
	{
		FString Key = TAGLIB_TO_UE(It->first);
		FString Value;

		// 如果有多个值，用逗号拼接 (例如多个 Artist)
		if (!It->second.isEmpty())
		{
			Value = TAGLIB_TO_UE(It->second.toString(", "));
		}
		Props.Add(Key, Value);
	}
	return Props;
}

// --- Cover Art Extraction (以 MP3 ID3v2 为例) ---
bool FDreamMusicPlayerTagLib::GetCoverArt(TArray<uint8>& OutData, FString& OutMimeType) const
{
	if (!IsValid()) return false;

	TagLib::File* File = FileRef->file();

	// --- Case A: MP3 (ID3v2) ---
	if (TagLib::MPEG::File* MpegFile = dynamic_cast<TagLib::MPEG::File*>(File))
	{
		if (MpegFile->ID3v2Tag())
		{
			TagLib::ID3v2::FrameList FrameList = MpegFile->ID3v2Tag()->frameList("APIC");
			if (!FrameList.isEmpty())
			{
				auto* Frame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(FrameList.front());
				TagLib::ByteVector PictureData = Frame->picture();

				OutData.SetNumUninitialized(PictureData.size());
				FMemory::Memcpy(OutData.GetData(), PictureData.data(), PictureData.size());
				OutMimeType = TAGLIB_TO_UE(Frame->mimeType());
				return true;
			}
		}
	}
	// --- Case B: FLAC (New!) ---
	else if (TagLib::FLAC::File* FlacFile = dynamic_cast<TagLib::FLAC::File*>(File))
	{
		const TagLib::List<TagLib::FLAC::Picture*>& PictureList = FlacFile->pictureList();
		if (!PictureList.isEmpty())
		{
			// 通常取第一个图片作为封面
			TagLib::FLAC::Picture* Pic = PictureList.front();
			TagLib::ByteVector PictureData = Pic->data();

			OutData.SetNumUninitialized(PictureData.size());
			FMemory::Memcpy(OutData.GetData(), PictureData.data(), PictureData.size());
			OutMimeType = TAGLIB_TO_UE(Pic->mimeType());
			return true;
		}
	}

	// 你可以在这里继续添加 Ogg Vorbis (COVERART 字段) 或 MP4 (covr atom) 的支持

	return false;
}

UTexture2D* FDreamMusicPlayerTagLib::GetCoverArtTexture() const
{
	// 检查是否在主线程 (虽然 CreateTransient 不会立即崩溃，但后续 UpdateResource 需要主线程环境)
	if (!IsInGameThread())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetCoverArtTexture must be called on Game Thread!"));
		return nullptr;
	}

	TArray<uint8> RawData;
	FString MimeType;

	// 先获取原始数据
	if (!GetCoverArt(RawData, MimeType) || RawData.Num() == 0)
	{
		return nullptr;
	}

	// 加载 ImageWrapper 模块
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	// 自动通过数据头判断格式 (JPEG, PNG 等)，比依赖 MimeType 更靠谱
	EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(RawData.GetData(), RawData.Num());

	if (ImageFormat == EImageFormat::Invalid)
	{
		return nullptr;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);

	// 解压图片数据
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawData.GetData(), RawData.Num()))
	{
		TArray<uint8> UncompressedBGRA;
		// 强制转换为 BGRA 格式，这是 Texture2D 最喜欢的格式
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
		{
			// 创建瞬态纹理 (不会保存到磁盘，只在内存存在)
			UTexture2D* NewTexture = UTexture2D::CreateTransient(
				ImageWrapper->GetWidth(),
				ImageWrapper->GetHeight(),
				PF_B8G8R8A8
			);

			if (NewTexture)
			{
				// 锁定纹理内存并写入数据
				void* TextureData = NewTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
				NewTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

				// 更新资源设置
				NewTexture->UpdateResource();
				return NewTexture;
			}
		}
	}

	return nullptr;
}

FDreamMusicTag FDreamMusicPlayerTagLib::GetTagData(bool bLoadCoverTexture)
{
	FDreamMusicTag OutTag;

	// 1. 填充基础信息
	OutTag.Title = GetTitle();
	OutTag.Artist = GetArtist();
	OutTag.Album = GetAlbum();
	OutTag.Genre = GetGenre();
	OutTag.Comment = GetComment();
	OutTag.Year = GetYear();
	OutTag.Track = GetTrack();

	// 2. 填充音频属性
	OutTag.Duration = GetDuration();
	OutTag.Bitrate = GetBitrate();
	OutTag.SampleRate = GetSampleRate();
	OutTag.Channels = GetChannels();

	// 3. 填充文件信息 (需要你把 DetectFileFormat 的结果存一下，或者重新调一次)
	FDreamMusicPlayerTagLibFileFormat Detected = DetectFileFormat();
	OutTag.FileType = Detected.FileFormat;
	// 如果你在类里存了 FilePath 成员变量，赋值给它；如果没有，可以通过 FileRef 获取
	// OutTag.FilePath = ...; 

	// 4. 处理封面 (关键点！)
	if (bLoadCoverTexture)
	{
		if (IsInGameThread())
		{
			// 直接调用之前写的生成 Texture 的方法
			OutTag.CoverArt = GetCoverArtTexture();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GetTagData: Skipped Texture creation because we are not on GameThread."));
			OutTag.CoverArt = nullptr;
		}
	}

	return OutTag;
}
