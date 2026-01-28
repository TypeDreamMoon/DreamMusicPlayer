#include "Lyric/Search/DreamLyricsSearcher.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Misc/Base64.h"
#include "Misc/SecureHash.h"
#include "Misc/App.h"
#include "Policies/CondensedJsonPrintPolicy.h"

DEFINE_LOG_CATEGORY_STATIC(LogLyrics, Log, All);

// --- OpenSSL Includes ---
#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include "openssl/aes.h"
#include "openssl/evp.h"
THIRD_PARTY_INCLUDES_END
#undef UI

// ============================================================================
// 1. Netease EAPI 加密库
// ============================================================================

namespace NeteaseEAPI
{
    static const FString EAPI_KEY = TEXT("e82ckenh8dichen8");
    static const FString EAPI_MAGIC_1 = TEXT("nobody");
    static const FString EAPI_MAGIC_2 = TEXT("use");
    static const FString EAPI_MAGIC_3 = TEXT("md5forencrypt");

    struct FSessionState
    {
        FString Cookies;        
        FString DeviceId;
        FString AppVer = TEXT("3.1.3.203419");
        FString OsVer;
        FString ClientSign;     
        FString WNMCID; 
        FString Mode;
        double ExpireTime = 0.0;

        bool IsValid() const 
        { 
            return !Cookies.IsEmpty() && FPlatformTime::Seconds() < ExpireTime; 
        }
    };
    static FSessionState GSession;

    FString RandomHex(int32 Bytes, bool bColonSeparated = false)
    {
        FString Result;
        for (int32 i = 0; i < Bytes; i++)
        {
            if (bColonSeparated && i > 0) Result += TEXT(":");
            Result += FString::Printf(TEXT("%02X"), FMath::RandRange(0, 255));
        }
        return Result;
    }

    FString RandomString(int32 Length, bool bLowerCase = false)
    {
        const FString Chars = bLowerCase ? TEXT("abcdefghijklmnopqrstuvwxyz") : TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        FString Res;
        for (int i = 0; i < Length; ++i) Res.AppendChar(Chars[FMath::RandRange(0, Chars.Len() - 1)]);
        return Res;
    }

    void InitSessionIfNeeded()
    {
        if (!GSession.DeviceId.IsEmpty()) return;

        GSession.DeviceId = RandomString(20).ToLower();
        GSession.OsVer = FString::Printf(TEXT("Microsoft-Windows-10--build-%d00-64bit"), FMath::RandRange(200, 300));
        
        TArray<FString> Modes = { TEXT("MS-iCraft B760M WIFI"), TEXT("ASUS ROG STRIX Z790"), TEXT("MSI MAG B550 TOMAHAWK"), TEXT("ASRock X670E Taichi") };
        GSession.Mode = Modes[FMath::RandRange(0, Modes.Num() - 1)];

        FString Mac = RandomHex(6, true); 
        FString RndStr = RandomString(8); 
        FString HashPart = RandomHex(32).ToLower().Replace(TEXT(":"), TEXT("")); 
        GSession.ClientSign = FString::Printf(TEXT("%s@@@%s@@@@@@%s"), *Mac, *RndStr, *HashPart);

        FString RandomPrefix = RandomString(6, true);
        int64 Timestamp = FDateTime::UtcNow().ToUnixTimestamp() * 1000 - FMath::RandRange(1000, 10000);
        GSession.WNMCID = FString::Printf(TEXT("%s.%lld.01.0"), *RandomPrefix, Timestamp);
    }

    FString BuildFullCookieString()
    {
        InitSessionIfNeeded();
        // 确保包含 WEVNSM，且顺序尽量接近 LDDC
        FString ClientCookies = FString::Printf(TEXT("WEVNSM=1.0.0; os=pc; deviceId=%s; osver=%s; appver=%s; channel=netease; mode=%s; clientSign=%s; WNMCID=%s;"), 
            *GSession.DeviceId, 
            *GSession.OsVer, 
            *GSession.AppVer,
            *GSession.Mode,
            *GSession.ClientSign,
            *GSession.WNMCID
        );
        
        // 合并 Server Cookie 和 Client Cookie
        return GSession.Cookies + ClientCookies;
    }

    // AES-128-ECB 加密
    TArray<uint8> AES128ECB_Encrypt(const TArray<uint8>& Data, const FString& Key)
    {
        FTCHARToUTF8 KeyUtf8(*Key);
        EVP_CIPHER_CTX* Ctx = EVP_CIPHER_CTX_new();
        if (!Ctx) return {};

        if (1 != EVP_EncryptInit_ex(Ctx, EVP_aes_128_ecb(), NULL, (const unsigned char*)KeyUtf8.Get(), NULL))
        {
            EVP_CIPHER_CTX_free(Ctx);
            return {};
        }

        EVP_CIPHER_CTX_set_padding(Ctx, 1);

        int MaxOutLen = Data.Num() + 16;
        TArray<uint8> OutData;
        OutData.SetNumUninitialized(MaxOutLen);

        int OutLen1 = 0, OutLen2 = 0;
        EVP_EncryptUpdate(Ctx, OutData.GetData(), &OutLen1, Data.GetData(), Data.Num());
        EVP_EncryptFinal_ex(Ctx, OutData.GetData() + OutLen1, &OutLen2);
        
        EVP_CIPHER_CTX_free(Ctx);
        OutData.SetNum(OutLen1 + OutLen2);
        return OutData;
    }

    // AES-128-ECB 解密 (含容错处理)
    FString AES128ECB_Decrypt_Internal(const TArray<uint8>& Data, const FString& Key, bool bEnablePadding)
    {
        FTCHARToUTF8 KeyUtf8(*Key);
        EVP_CIPHER_CTX* Ctx = EVP_CIPHER_CTX_new();
        if (!Ctx) return TEXT("");

        if (1 != EVP_DecryptInit_ex(Ctx, EVP_aes_128_ecb(), NULL, (const unsigned char*)KeyUtf8.Get(), NULL))
        {
            EVP_CIPHER_CTX_free(Ctx);
            return TEXT("");
        }

        EVP_CIPHER_CTX_set_padding(Ctx, bEnablePadding ? 1 : 0);

        TArray<uint8> OutData;
        OutData.SetNumUninitialized(Data.Num() + 32); 
        int OutLen1 = 0, OutLen2 = 0;

        if (1 != EVP_DecryptUpdate(Ctx, OutData.GetData(), &OutLen1, Data.GetData(), Data.Num()))
        {
             EVP_CIPHER_CTX_free(Ctx);
             return TEXT("");
        }
        
        if (1 != EVP_DecryptFinal_ex(Ctx, OutData.GetData() + OutLen1, &OutLen2))
        {
            EVP_CIPHER_CTX_free(Ctx);
            return TEXT("DECRYPT_FAIL");
        }
        
        EVP_CIPHER_CTX_free(Ctx);
        
        int32 TotalLen = OutLen1 + OutLen2;
        OutData.SetNum(TotalLen);

        // 手动截断 JSON 结束符 '}' 之后的内容
        int32 JsonEndIndex = -1;
        for (int32 i = TotalLen - 1; i >= 0; --i)
        {
            if (OutData[i] == 0x7D) // '}'
            {
                JsonEndIndex = i;
                break;
            }
        }

        if (JsonEndIndex != -1) OutData.SetNum(JsonEndIndex + 1);
        OutData.Add(0);
        
        return FString(UTF8_TO_TCHAR((const char*)OutData.GetData()));
    }

    FString AES128ECB_Decrypt(const TArray<uint8>& Data, const FString& Key)
    {
        if (Data.Num() > 0 && Data[0] == '{')
        {
            FString RawJson = FString(UTF8_TO_TCHAR((const char*)Data.GetData()));
            UE_LOG(LogLyrics, Warning, TEXT("收到明文响应: %s"), *RawJson);
            return RawJson;
        }

        FString Result = AES128ECB_Decrypt_Internal(Data, Key, true);
        if (Result == TEXT("DECRYPT_FAIL"))
        {
            Result = AES128ECB_Decrypt_Internal(Data, Key, false);
            if (Result == TEXT("DECRYPT_FAIL")) return TEXT("");
        }
        return Result;
    }

    FString CalcMD5(const FString& Input)
    {
        FTCHARToUTF8 Utf8(*Input);
        uint8 Digest[16];
        FMD5 Md5Gen;
        Md5Gen.Update((const uint8*)Utf8.Get(), Utf8.Length());
        Md5Gen.Final(Digest);
        FString Hex;
        for (int i = 0; i < 16; i++) Hex += FString::Printf(TEXT("%02x"), Digest[i]);
        return Hex;
    }

    TArray<uint8> EncryptParams(const FString& ApiPath, const FString& JsonStr)
    {
        FString Message = EAPI_MAGIC_1 + ApiPath + EAPI_MAGIC_2 + JsonStr + EAPI_MAGIC_3;
        FString Digest = CalcMD5(Message);
        FString RawData = ApiPath + JsonStr + Digest;

        FTCHARToUTF8 RawUtf8(*RawData);
        TArray<uint8> InputBytes;
        InputBytes.Append((const uint8*)RawUtf8.Get(), RawUtf8.Length());

        return AES128ECB_Encrypt(InputBytes, EAPI_KEY);
    }

    FString SerializeJsonCompact(const TSharedPtr<FJsonObject>& JsonObj)
    {
        FString OutputString;
        TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutputString);
        FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
        return OutputString;
    }

    // 手动构造 Header JSON 以保证与 LDDC 完全一致 (避免 TJsonWriter 的顺序或数字格式问题)
    FString GetHeaderJson()
    {
        InitSessionIfNeeded();
        // 严格参照 LDDC ne.py 的 _get_params_header 结构
        // 注意: 移除了 channel，确保 requestId 为 0
        return FString::Printf(TEXT("{\"clientSign\":\"%s\",\"os\":\"pc\",\"appver\":\"%s\",\"deviceId\":\"%s\",\"requestId\":0,\"osver\":\"%s\"}"),
            *GSession.ClientSign,
            *GSession.AppVer,
            *GSession.DeviceId,
            *GSession.OsVer
        );
    }
}

// ============================================================================
// 2. RequestHandler 实现
// ============================================================================

void FLyricsTool::Search(const FString& Keyword, ELyricsPlatform Platform, FOnSearchComplete Callback)
{
    TSharedRef<FLyricsRequestHandler> Handler = MakeShared<FLyricsRequestHandler>();
    Handler->Search(Keyword, Platform, Callback);
}

void FLyricsTool::GetLyric(const FString& SongID, ELyricsPlatform Platform, FOnLyricComplete Callback)
{
    TSharedRef<FLyricsRequestHandler> Handler = MakeShared<FLyricsRequestHandler>();
    Handler->GetLyric(SongID, Platform, Callback);
}

void SetupCommonHeaders(const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Request)
{
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
    Request->SetHeader(TEXT("User-Agent"), TEXT("Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Safari/537.36 Chrome/91.0.4472.164 NeteaseMusicDesktop/3.1.3.203419"));
    Request->SetHeader(TEXT("origin"), TEXT("orpheus://orpheus"));
    Request->SetHeader(TEXT("mconfig-info"), TEXT("{\"IuRPVVmc3WWul9fT\":{\"version\":733184,\"appver\":\"3.1.3.203419\"}}"));
}

void FLyricsRequestHandler::Search(const FString& Keyword, ELyricsPlatform Platform, FOnSearchComplete Callback)
{
    if (Platform == ELyricsPlatform::Netease)
    {
        CheckLoginAndExecute([this, Keyword, Callback]() {
            PerformNeteaseSearch(Keyword, Callback);
        });
    }
    else
    {
        PerformLrclibSearch(Keyword, Callback);
    }
}

void FLyricsRequestHandler::GetLyric(const FString& SongID, ELyricsPlatform Platform, FOnLyricComplete Callback)
{
    if (Platform == ELyricsPlatform::Netease)
    {
        CheckLoginAndExecute([this, SongID, Callback]() {
            PerformNeteaseLyric(SongID, Callback);
        });
    }
    else
    {
        PerformLrclibLyric(SongID, Callback);
    }
}

void FLyricsRequestHandler::CheckLoginAndExecute(TFunction<void()> OnReady)
{
    if (NeteaseEAPI::GSession.IsValid())
    {
        OnReady();
        return;
    }

    UE_LOG(LogLyrics, Log, TEXT("正在匿名登录..."));
    NeteaseEAPI::InitSessionIfNeeded();

    FString ApiPath = TEXT("/api/register/anonimous");
    
    TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
    FString RandomUser = NeteaseEAPI::RandomString(16);
    JsonObj->SetStringField(TEXT("username"), FBase64::Encode(RandomUser));
    JsonObj->SetBoolField(TEXT("e_r"), true);
    // 使用手动构造的 Header 字符串
    JsonObj->SetStringField(TEXT("header"), NeteaseEAPI::GetHeaderJson());

    FString JsonStr = NeteaseEAPI::SerializeJsonCompact(JsonObj);
    TArray<uint8> PostData = NeteaseEAPI::EncryptParams(ApiPath, JsonStr);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("https://interface.music.163.com/eapi/register/anonimous"));
    Request->SetVerb(TEXT("POST"));
    Request->SetContent(PostData);
    SetupCommonHeaders(Request);
    // 登录也带上基础 Cookie
    Request->SetHeader(TEXT("Cookie"), NeteaseEAPI::BuildFullCookieString());

    TSharedRef<FLyricsRequestHandler> Self = AsShared();
    Request->OnProcessRequestComplete().BindLambda([Self, OnReady](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bConnected)
    {
        if (bConnected && Res.IsValid() && Res->GetResponseCode() == 200)
        {
            FString Decrypted = NeteaseEAPI::AES128ECB_Decrypt(Res->GetContent(), NeteaseEAPI::EAPI_KEY);
            
            FString AllCookies;
            for (const FString& Cookie : Res->GetAllHeaders())
            {
                if (Cookie.StartsWith(TEXT("Set-Cookie:")))
                {
                    FString CookieContent = Cookie.RightChop(11).TrimStartAndEnd();
                    int32 SemiIdx;
                    if (CookieContent.FindChar(';', SemiIdx)) CookieContent = CookieContent.Left(SemiIdx);
                    
                    if (CookieContent.Contains(TEXT("MUSIC_A")) || 
                        CookieContent.Contains(TEXT("NMTID")) || 
                        CookieContent.Contains(TEXT("__csrf")))
                    {
                        AllCookies += CookieContent + TEXT("; ");
                    }
                }
            }
            
            NeteaseEAPI::GSession.Cookies = AllCookies;
            NeteaseEAPI::GSession.ExpireTime = FPlatformTime::Seconds() + 86400.0;
            UE_LOG(LogLyrics, Log, TEXT("登录成功"));
            OnReady();
        }
        else
        {
            UE_LOG(LogLyrics, Error, TEXT("登录失败 Code: %d"), Res.IsValid() ? Res->GetResponseCode() : 0);
        }
    });
    Request->ProcessRequest();
}

void FLyricsRequestHandler::PerformNeteaseSearch(const FString& Keyword, FOnSearchComplete Callback)
{
    FString ApiPath = TEXT("/api/search/song/list/page");
    FString RealPath = TEXT("/eapi/search/song/list/page");

    TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
    JsonObj->SetStringField(TEXT("keyword"), Keyword);
    JsonObj->SetStringField(TEXT("limit"), TEXT("20"));
    JsonObj->SetStringField(TEXT("offset"), TEXT("0"));
    JsonObj->SetStringField(TEXT("scene"), TEXT("NORMAL"));
    JsonObj->SetStringField(TEXT("needCorrect"), TEXT("true"));
    JsonObj->SetBoolField(TEXT("e_r"), true);
    JsonObj->SetStringField(TEXT("header"), NeteaseEAPI::GetHeaderJson());

    FString JsonStr = NeteaseEAPI::SerializeJsonCompact(JsonObj);
    TArray<uint8> PostData = NeteaseEAPI::EncryptParams(ApiPath, JsonStr);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("https://interface.music.163.com") + RealPath);
    Request->SetVerb(TEXT("POST"));
    Request->SetContent(PostData);
    SetupCommonHeaders(Request);
    Request->SetHeader(TEXT("Cookie"), NeteaseEAPI::BuildFullCookieString());

    TSharedRef<FLyricsRequestHandler> Self = AsShared();
    Request->OnProcessRequestComplete().BindLambda([Self, Callback](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bConnected)
    {
        if (bConnected && Res.IsValid() && Res->GetResponseCode() == 200)
        {
            FString Decrypted = NeteaseEAPI::AES128ECB_Decrypt(Res->GetContent(), NeteaseEAPI::EAPI_KEY);
            // 成功才打印前 300 字符，避免刷屏
            UE_LOG(LogLyrics, Log, TEXT("Search Response: %s"), *Decrypted.Left(300)); 
            TArray<FSongInfo> Results = Self->ParseNeteaseSearch(Decrypted);
            Callback(true, Results);
        }
        else
        {
            UE_LOG(LogLyrics, Error, TEXT("搜索失败 Code: %d"), Res.IsValid() ? Res->GetResponseCode() : 0);
            Callback(false, {});
        }
    });
    Request->ProcessRequest();
}

void FLyricsRequestHandler::PerformNeteaseLyric(const FString& SongID, FOnLyricComplete Callback)
{
    FString ApiPath = TEXT("/api/song/lyric/v1");
    FString RealPath = TEXT("/eapi/song/lyric/v1");

    TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
    JsonObj->SetNumberField(TEXT("id"), FCString::Atoi(*SongID));
    JsonObj->SetStringField(TEXT("lv"), TEXT("-1"));
    JsonObj->SetStringField(TEXT("tv"), TEXT("-1"));
    JsonObj->SetStringField(TEXT("rv"), TEXT("-1"));
    JsonObj->SetStringField(TEXT("kv"), TEXT("-1"));
    JsonObj->SetBoolField(TEXT("e_r"), true);
    JsonObj->SetStringField(TEXT("header"), NeteaseEAPI::GetHeaderJson());

    FString JsonStr = NeteaseEAPI::SerializeJsonCompact(JsonObj);
    TArray<uint8> PostData = NeteaseEAPI::EncryptParams(ApiPath, JsonStr);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("https://interface.music.163.com") + RealPath);
    Request->SetVerb(TEXT("POST"));
    Request->SetContent(PostData);
    SetupCommonHeaders(Request);
    Request->SetHeader(TEXT("Cookie"), NeteaseEAPI::BuildFullCookieString());

    TSharedRef<FLyricsRequestHandler> Self = AsShared();
    Request->OnProcessRequestComplete().BindLambda([Self, SongID, Callback](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bConnected)
    {
        if (bConnected && Res.IsValid() && Res->GetResponseCode() == 200)
        {
            FString Decrypted = NeteaseEAPI::AES128ECB_Decrypt(Res->GetContent(), NeteaseEAPI::EAPI_KEY);
            FLyricResult Result = Self->ParseNeteaseLyric(Decrypted, SongID);
            Callback(true, Result);
        }
        else
        {
            Callback(false, FLyricResult());
        }
    });
    Request->ProcessRequest();
}

// --- Lrclib & Parsers (保持不变) ---

void FLyricsRequestHandler::PerformLrclibSearch(const FString& Keyword, FOnSearchComplete Callback)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(FString::Printf(TEXT("https://lrclib.net/api/search?q=%s"), *FGenericPlatformHttp::UrlEncode(Keyword)));
    Request->SetVerb(TEXT("GET"));
    TSharedRef<FLyricsRequestHandler> Self = AsShared();
    Request->OnProcessRequestComplete().BindLambda([Self, Callback](FHttpRequestPtr, FHttpResponsePtr Res, bool bConnected) {
        if (bConnected && Res.IsValid() && Res->GetResponseCode() == 200) Callback(true, Self->ParseLrclibSearch(Res->GetContentAsString()));
        else Callback(false, {});
    });
    Request->ProcessRequest();
}

void FLyricsRequestHandler::PerformLrclibLyric(const FString& SongID, FOnLyricComplete Callback)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(FString::Printf(TEXT("https://lrclib.net/api/get/%s"), *SongID));
    Request->SetVerb(TEXT("GET"));
    TSharedRef<FLyricsRequestHandler> Self = AsShared();
    Request->OnProcessRequestComplete().BindLambda([Self, SongID, Callback](FHttpRequestPtr, FHttpResponsePtr Res, bool bConnected) {
        if (bConnected && Res.IsValid() && Res->GetResponseCode() == 200) Callback(true, Self->ParseLrclibLyric(Res->GetContentAsString(), SongID));
        else Callback(false, FLyricResult());
    });
    Request->ProcessRequest();
}

TArray<FSongInfo> FLyricsRequestHandler::ParseNeteaseSearch(const FString& Json)
{
    TArray<FSongInfo> List;
    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
    {
        const TSharedPtr<FJsonObject>* DataObj;
        if (Root->TryGetObjectField(TEXT("data"), DataObj))
        {
            const TArray<TSharedPtr<FJsonValue>>* ResArray;
            if ((*DataObj)->TryGetArrayField(TEXT("resources"), ResArray))
            {
                for (auto& Val : *ResArray)
                {
                    TSharedPtr<FJsonObject> BaseInfo = Val->AsObject()->GetObjectField(TEXT("baseInfo"));
                    if (!BaseInfo.IsValid()) continue;
                    TSharedPtr<FJsonObject> SongObj = BaseInfo->GetObjectField(TEXT("simpleSongData"));
                    if (!SongObj.IsValid()) continue;

                    FSongInfo Info;
                    Info.ID = FString::FromInt(SongObj->GetIntegerField(TEXT("id")));
                    Info.Title = SongObj->GetStringField(TEXT("name"));
                    Info.Duration = SongObj->GetIntegerField(TEXT("dt"));
                    Info.Source = TEXT("Netease");

                    const TArray<TSharedPtr<FJsonValue>>* Ar;
                    if (SongObj->TryGetArrayField(TEXT("ar"), Ar) && Ar->Num() > 0)
                        Info.Artist = (*Ar)[0]->AsObject()->GetStringField(TEXT("name"));

                    const TSharedPtr<FJsonObject>* Al;
                    if (SongObj->TryGetObjectField(TEXT("al"), Al))
                        Info.Album = (*Al)->GetStringField(TEXT("name"));

                    List.Add(Info);
                }
            }
        }
        else
        {
            UE_LOG(LogLyrics, Warning, TEXT("Netease API 解析失败 Code: %d, Msg: %s"), 
                Root->GetIntegerField(TEXT("code")), *Root->GetStringField(TEXT("message")));
        }
    }
    return List;
}

FLyricResult FLyricsRequestHandler::ParseNeteaseLyric(const FString& Json, const FString& SongID)
{
    FLyricResult Res;
    Res.SongID = SongID;
    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
    {
        const TSharedPtr<FJsonObject>* DataObj;
        const TSharedPtr<FJsonObject>* TargetRoot = &Root;
        if (Root->TryGetObjectField(TEXT("data"), DataObj)) TargetRoot = DataObj;

        const TSharedPtr<FJsonObject>* Lrc;
        if ((*TargetRoot)->TryGetObjectField(TEXT("lrc"), Lrc))
            Res.Lyric = (*Lrc)->GetStringField(TEXT("lyric"));
            
        const TSharedPtr<FJsonObject>* Tlrc;
        if ((*TargetRoot)->TryGetObjectField(TEXT("tlyric"), Tlrc))
            Res.TLyric = (*Tlrc)->GetStringField(TEXT("lyric"));
            
        const TSharedPtr<FJsonObject>* Yrc;
        if ((*TargetRoot)->TryGetObjectField(TEXT("yrc"), Yrc))
            Res.KLyric = (*Yrc)->GetStringField(TEXT("lyric"));
    }
    return Res;
}

TArray<FSongInfo> FLyricsRequestHandler::ParseLrclibSearch(const FString& Json)
{
    TArray<FSongInfo> List;
    TArray<TSharedPtr<FJsonValue>> Arr;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (FJsonSerializer::Deserialize(Reader, Arr))
    {
        for (auto& Val : Arr)
        {
            auto Obj = Val->AsObject();
            if (!Obj.IsValid()) continue;
            FSongInfo Info;
            Info.ID = FString::FromInt(Obj->GetIntegerField(TEXT("id")));
            Info.Title = Obj->GetStringField(TEXT("trackName"));
            Info.Artist = Obj->GetStringField(TEXT("artistName"));
            Info.Album = Obj->GetStringField(TEXT("albumName"));
            Info.Duration = (int32)(Obj->GetNumberField(TEXT("duration")) * 1000);
            Info.Source = TEXT("Lrclib");
            List.Add(Info);
        }
    }
    return List;
}

FLyricResult FLyricsRequestHandler::ParseLrclibLyric(const FString& Json, const FString& SongID)
{
    FLyricResult Res;
    Res.SongID = SongID;
    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
    {
        Res.Lyric = Root->GetStringField(TEXT("plainLyrics"));
        Res.TLyric = Root->GetStringField(TEXT("syncedLyrics"));
    }
    return Res;
}