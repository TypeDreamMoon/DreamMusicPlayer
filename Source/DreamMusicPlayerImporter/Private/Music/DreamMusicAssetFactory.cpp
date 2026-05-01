#include "Music/DreamMusicAssetFactory.h"
#include "Music/DreamMusicImportDialog.h"
#include "Classes/DreamMusicDataAsset.h"
#include "DreamMusicPlayerTagLib.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "EditorFramework/AssetImportData.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "AssetToolsModule.h"
#include "Factories/SoundFactory.h"
#include "Factories/TextureFactory.h"
#include "UObject/SavePackage.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/ScopedSlowTask.h"

#include "DreamMusicPlayerDecoderWrapper.h"
#include "DreamMusicDecryptor.h"

#define LOCTEXT_NAMESPACE "DreamMusicAssetFactory"

namespace
{
	void CreateWavHeader(TArray<uint8>& OutBuffer, int32 SampleRate, int32 NumChannels, int32 BitsPerSample, int32 PCMDataSize)
	{
		// WAV Header Structure (44 bytes)
		OutBuffer.SetNumUninitialized(44 + PCMDataSize);

		uint8* Header = OutBuffer.GetData();
		FMemory::Memzero(Header, 44);

		// RIFF Chunk
		FMemory::Memcpy(Header, "RIFF", 4);
		uint32 FileSize = 36 + PCMDataSize;
		FMemory::Memcpy(Header + 4, &FileSize, 4);
		FMemory::Memcpy(Header + 8, "WAVE", 4);

		// fmt Subchunk
		FMemory::Memcpy(Header + 12, "fmt ", 4);
		uint32 Subchunk1Size = 16;
		FMemory::Memcpy(Header + 16, &Subchunk1Size, 4);
		uint16 AudioFormat = 1; // PCM
		FMemory::Memcpy(Header + 20, &AudioFormat, 2);
		uint16 Channels = (uint16)NumChannels;
		FMemory::Memcpy(Header + 22, &Channels, 2);
		uint32 SampleRate32 = (uint32)SampleRate;
		FMemory::Memcpy(Header + 24, &SampleRate32, 4);

		uint32 ByteRate = SampleRate * NumChannels * (BitsPerSample / 8);
		FMemory::Memcpy(Header + 28, &ByteRate, 4);

		uint16 BlockAlign = NumChannels * (BitsPerSample / 8);
		FMemory::Memcpy(Header + 32, &BlockAlign, 2);
		uint16 BitsPerSample16 = (uint16)BitsPerSample;
		FMemory::Memcpy(Header + 34, &BitsPerSample16, 2);

		// data Subchunk
		FMemory::Memcpy(Header + 36, "data", 4);
		uint32 DataSize = PCMDataSize;
		FMemory::Memcpy(Header + 40, &DataSize, 4);
	}
}

UDreamMusicAssetFactory::UDreamMusicAssetFactory()
{
	// Supported formats
	Formats.Add(TEXT("mp3;MP3 Audio File"));
	Formats.Add(TEXT("wav;WAV Audio File"));
	Formats.Add(TEXT("flac;FLAC Audio File"));
	Formats.Add(TEXT("ogg;OGG Vorbis File"));

	// Encrypted Formats
	Formats.Add(TEXT("ncm;Netease Cloud Music"));
	Formats.Add(TEXT("qmcflac;QQ Music Flac"));
	Formats.Add(TEXT("qmc0;QQ Music MP3"));
	Formats.Add(TEXT("mflac;QQ Music MFLAC"));
	Formats.Add(TEXT("mgg;QQ Music MGG"));
	Formats.Add(TEXT("kgm;Kugou Music"));
	Formats.Add(TEXT("kwm;Kuwo Music"));
	Formats.Add(TEXT("xm;Xiami Music"));

	bCreateNew = false;
	bEditAfterNew = true;
	bEditorImport = false;
	SupportedClass = UDreamMusicDataAsset::StaticClass();
}

UObject* UDreamMusicAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	bOutOperationCanceled = false;

	// 定义变量以保存数据
	FDreamMusicTag FileTag;
	TArray<uint8> CoverData;
	FString ActualImportFilename = Filename; // 实际用于导入的文件路径 (可能是临时文件)
	bool bIsTempFile = false; // 标记是否需要删除临时文件

	// 检查是否为 NCM 格式
	FString Extension = FPaths::GetExtension(Filename).ToLower();

	// 判断是否为需要解密的格式 (非标准音频格式)
	const bool bIsEncrypted = (Extension != "mp3" && Extension != "wav" && Extension != "flac" && Extension != "ogg");

	if (bIsEncrypted)
	{
		FScopedSlowTask SlowTask(2.0f, LOCTEXT("DecryptingMusic", "Decrypting Music File..."));
		SlowTask.MakeDialog();
		SlowTask.EnterProgressFrame(1.0f);

		// === 统一调用解密工厂 ===
		FDreamMusicDecryptionResult DecryptResult = FDreamMusicDecryptorFactory::DecryptFile(Filename);

		if (DecryptResult.bSuccess)
		{
			FileTag = DecryptResult.Tag;
			CoverData = DecryptResult.CoverData;

			// 设置文件类型
			if (DecryptResult.Format == TEXT("flac")) FileTag.FileType = EDreamMusicPlayerTagLibFileFormat::FLAC;
			else if (DecryptResult.Format == TEXT("mp3")) FileTag.FileType = EDreamMusicPlayerTagLibFileFormat::MP3;
			else if (DecryptResult.Format == TEXT("ogg")) FileTag.FileType = EDreamMusicPlayerTagLibFileFormat::Ogg;

			// 写入临时文件
			SlowTask.EnterProgressFrame(1.0f, LOCTEXT("WritingTemp", "Writing temporary file..."));

			FString TempDir = FPaths::ProjectSavedDir() / TEXT("TempImport");
			IFileManager::Get().MakeDirectory(*TempDir, true);
			FString TempFileName = FString::Printf(TEXT("%s.%s"), *FGuid::NewGuid().ToString(), *DecryptResult.Format);
			ActualImportFilename = TempDir / TempFileName;

			if (FFileHelper::SaveArrayToFile(DecryptResult.AudioData, *ActualImportFilename))
			{
				bIsTempFile = true;

				// 尝试再次提取元数据（TagLib 对解密后的文件支持更好）
				{
					FDreamMusicPlayerTagLib TempTagLib(ActualImportFilename);
					if (TempTagLib.IsValid())
					{
						FDreamMusicTag TempTag = TempTagLib.GetTagData(false);
						if (FileTag.Title.IsEmpty()) FileTag.Title = TempTag.Title;
						if (FileTag.Artist.IsEmpty()) FileTag.Artist = TempTag.Artist;
						if (FileTag.Album.IsEmpty()) FileTag.Album = TempTag.Album;
						if (CoverData.Num() == 0)
						{
							FString Mime;
							TempTagLib.GetCoverArt(CoverData, Mime);
						}
					}
				}
			}
			else
			{
				Warn->Logf(ELogVerbosity::Error, TEXT("Failed to save temp file: %s"), *ActualImportFilename);
				return nullptr;
			}
		}
		else
		{
			Warn->Logf(ELogVerbosity::Error, TEXT("Decryption failed: %s"), *DecryptResult.ErrorMessage);
			// 失败则无法继续
			return nullptr;
		}
	}
	else
	{
		// 标准文件处理
		FDreamMusicPlayerTagLib TagLib(Filename);
		if (TagLib.IsValid())
		{
			FileTag = TagLib.GetTagData(false);
			FString CoverMime;
			TagLib.GetCoverArt(CoverData, CoverMime);
		}
	}

	// 2. Show Import Dialog
	TSharedPtr<SDreamMusicImportDialog> ImportDialog = SNew(SDreamMusicImportDialog);
	// 注意：如果是 NCM，FileTag 中已经包含了基本的 Title/Artist 等信息，Dialog 会自动填充
	ImportDialog->InitFromTag(FileTag, CoverData, Filename);

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::Format(LOCTEXT("ImportWindowTitle", "Import Music: {0}"), FText::FromString(FPaths::GetCleanFilename(Filename))))
		.ClientSize(FVector2D(600, 500))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SizingRule(ESizingRule::UserSized)
		[
			ImportDialog.ToSharedRef()
		];

	ImportDialog->SetParentWindow(Window);
	FSlateApplication::Get().AddModalWindow(Window, nullptr);

	if (!ImportDialog->ShouldImport())
	{
		bOutOperationCanceled = true;
		if (bIsTempFile) IFileManager::Get().Delete(*ActualImportFilename);
		return nullptr;
	}

	// 3. Create the Data Asset
	UDreamMusicDataAsset* NewAsset = NewObject<UDreamMusicDataAsset>(InParent, InClass, InName, Flags);

	// Get final data from dialog
	FDreamMusicTag FinalTag = ImportDialog->GetResultTag();
	const TArray<uint8>& FinalCoverData = ImportDialog->GetCoverData();

	// 4. Import Dependencies (Audio & Cover)
	FString PackagePath = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName());

	FString AssetName = FileTag.Title.IsEmpty() ? *(FileTag.Title + TEXT("_") + FileTag.Artist) : *InName.ToString();

	// Import Audio (as SW_AssetName)
	// 这里传入 ActualImportFilename (可能是原始文件，也可能是 NCM 解密后的临时 FLAC/MP3)
	FString AudioAssetName = FString::Printf(TEXT("SW_%s"), *AssetName);
	USoundBase* ImportedSound = ImportAudioFile(ActualImportFilename, PackagePath, AudioAssetName);
	if (ImportedSound) ImportedSound->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;

	// Import Cover (as TX_AssetName)
	UTexture2D* ImportedCover = nullptr;
	if (FinalCoverData.Num() > 0)
	{
		FString CoverAssetName = FString::Printf(TEXT("TX_%s_Cover"), *AssetName);
		ImportedCover = CreateCoverArtTexture(PackagePath, CoverAssetName, FinalCoverData, TEXT("image/jpeg")); // NCM covers are usually jpg/png, texture factory handles binary sniff
	}

	// 5. Link everything together
	ApplyImportData(NewAsset, FinalTag, ImportedSound, ImportedCover);

	// Cleanup Temp File
	if (bIsTempFile)
	{
		IFileManager::Get().Delete(*ActualImportFilename);
	}

	return NewAsset;
}

bool UDreamMusicAssetFactory::FactoryCanImport(const FString& Filename)
{
	FString Extension = FPaths::GetExtension(Filename).ToLower();
	const TSet<FString> SupportedExtensions = {
		TEXT("mp3"),
		TEXT("wav"),
		TEXT("flac"),
		TEXT("ogg"),

		TEXT("ncm"), // 网易云
		TEXT("qmcflac"), // QQ音乐
		TEXT("mflac"), // QQ音乐
		TEXT("qmc0"), // QQ音乐
		TEXT("qmc"), // QQ音乐
		TEXT("mgg"), // QQ音乐
		TEXT("kgm"), // 酷狗
		TEXT("kwm"), // 酷我
		TEXT("xm"), // 虾米
	};
	return SupportedExtensions.Contains(Extension);
}

FText UDreamMusicAssetFactory::GetDisplayName() const
{
	return LOCTEXT("FactoryDisplayName", "Dream Music Asset");
}

USoundBase* UDreamMusicAssetFactory::ImportAudioFile(const FString& InFilename, const FString& InTargetPackagePath, const FString& InAssetName)
{
	USoundFactory* SoundFactory = NewObject<USoundFactory>();
	SoundFactory->bAutoCreateCue = false;

	FString PackageName = FPaths::Combine(InTargetPackagePath, InAssetName);
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package) return nullptr;

	Package->FullyLoad();

	UObject* Result = nullptr;
	bool bCancelled = false;

	// Check Extension (could be the temp file extension now)
	FString Extension = FPaths::GetExtension(InFilename).ToLower();

	if (Extension == TEXT("flac"))
	{
		// FLAC: 使用自定义解码器
		FFlacResult DecodeResult = FDreamMusicPlayerFlacDecoderWrapper::Decode(InFilename);

		if (DecodeResult.bSuccess && DecodeResult.PcmData.Num() > 0)
		{
			TArray<uint8> WavMemoryBuffer;
			int32 PCMBytes = DecodeResult.PcmData.Num() * sizeof(int16);

			CreateWavHeader(WavMemoryBuffer, DecodeResult.SampleRate, DecodeResult.Channels, 16, PCMBytes);

			void* DataDest = WavMemoryBuffer.GetData() + 44;
			FMemory::Memcpy(DataDest, DecodeResult.PcmData.GetData(), PCMBytes);

			const uint8* BufferBegin = WavMemoryBuffer.GetData();
			const uint8* BufferEnd = BufferBegin + WavMemoryBuffer.Num();

			Result = SoundFactory->FactoryCreateBinary(
				USoundWave::StaticClass(),
				Package,
				FName(*InAssetName),
				RF_Public | RF_Standalone,
				nullptr,
				TEXT("wav"),
				BufferBegin,
				BufferEnd,
				GWarn
			);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("DreamMusicFactory: Failed to decode FLAC file: %s"), *InFilename);
		}
	}
	else
	{
		// MP3/WAV/OGG: 使用 UE 原生 FactoryCreateFile
		// 注意：如果是 NCM 解密出来的 MP3 临时文件，这里也能正常工作
		Result = SoundFactory->FactoryCreateFile(
			USoundWave::StaticClass(),
			Package,
			FName(*InAssetName),
			RF_Public | RF_Standalone,
			InFilename,
			nullptr,
			GWarn,
			bCancelled
		);
	}

	if (Result)
	{
		FAssetRegistryModule::AssetCreated(Result);
		bool bSaved = Package->MarkPackageDirty();
	}

	return Cast<USoundBase>(Result);
}

UTexture2D* UDreamMusicAssetFactory::CreateCoverArtTexture(const FString& InTargetPackagePath, const FString& InAssetName, const TArray<uint8>& InData, const FString& InMimeType)
{
	if (InData.Num() == 0) return nullptr;

	UTextureFactory* TexFactory = NewObject<UTextureFactory>();

	FString PackageName = FPaths::Combine(InTargetPackagePath, InAssetName);
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package) return nullptr;

	Package->FullyLoad();

	const uint8* Buffer = InData.GetData();
	const uint8* BufferEnd = Buffer + InData.Num();

	// bool bCancelled = false; // Unused
	UObject* Result = TexFactory->FactoryCreateBinary(
		UTexture2D::StaticClass(),
		Package,
		FName(*InAssetName),
		RF_Public | RF_Standalone,
		nullptr,
		*InMimeType,
		Buffer,
		BufferEnd,
		GWarn
	);

	if (Result)
	{
		FAssetRegistryModule::AssetCreated(Result);
		bool bSaved = Package->MarkPackageDirty();
	}

	return Cast<UTexture2D>(Result);
}

void UDreamMusicAssetFactory::ApplyImportData(UObject* Asset, const FDreamMusicTag& ImportTag, USoundBase* ImportedSound, UTexture2D* ImportedCover)
{
	UDreamMusicDataAsset* MusicAsset = Cast<UDreamMusicDataAsset>(Asset);
	if (!MusicAsset) return;

	// Copy Tag Data
	MusicAsset->Data.Tag = ImportTag;

	// Set Dependencies
	MusicAsset->Data.Music = ImportedSound;
	MusicAsset->Data.Tag.CoverArt = ImportedCover;

	// Ensure FilePath is set
	if (MusicAsset->Data.Tag.FilePath.IsEmpty())
	{
		MusicAsset->Data.Tag.FilePath = ImportTag.FilePath;
	}
}

#undef LOCTEXT_NAMESPACE
