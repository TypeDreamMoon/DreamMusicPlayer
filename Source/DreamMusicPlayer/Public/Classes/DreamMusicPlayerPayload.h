#pragma once

#include "CoreMinimal.h"
#include "DreamMusicPlayerPayload.generated.h"

UCLASS(EditInlineNew, Abstract, Blueprintable)
class DREAMMUSICPLAYER_API UDreamMusicPlayerPayload : public UObject
{
	GENERATED_BODY()
};


UCLASS(DisplayName = "Bool Payload")
class UDreamMusicPlayerPayload_Bool : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	bool Value;
};

UCLASS(DisplayName = "String Payload")
class UDreamMusicPlayerPayload_String : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	FString Value;
};

UCLASS(DisplayName = "Integer Payload")
class UDreamMusicPlayerPayload_Integer : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	int Value;
};

UCLASS(DisplayName = "Float Payload")
class UDreamMusicPlayerPayload_Float : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	float Value;
};

UCLASS(DisplayName = "Text Payload")
class UDreamMusicPlayerPayload_Text : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	FText Value;
};

UCLASS(DisplayName = "Byte Payload")
class UDreamMusicPlayerPayload_Byte : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	uint8 Value;
};

UCLASS(DisplayName = "Material Payload")
class UDreamMusicPlayerPayload_Material : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	TSoftObjectPtr<UMaterialInterface> Value;

	UFUNCTION(BlueprintPure)
	UMaterialInterface* GetMaterial() const;
};

inline UMaterialInterface* UDreamMusicPlayerPayload_Material::GetMaterial() const
{
	return Value.LoadSynchronous();
}

UCLASS(DisplayName = "Texture Payload")
class UDreamMusicPlayerPayload_Texture : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	TSoftObjectPtr<UTexture2D> Value;

	UFUNCTION(BlueprintPure)
	UTexture2D* GetTexture() const;
};

inline UTexture2D* UDreamMusicPlayerPayload_Texture::GetTexture() const
{
	return Value.LoadSynchronous();
}

UCLASS(DisplayName = "Vector2 Payload")
class UDreamMusicPlayerPayload_Vector2 : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	FVector2D Value;
};

UCLASS(DisplayName = "Vector3 Payload")
class UDreamMusicPlayerPayload_Vector3 : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	FVector Value;
};

UCLASS(DisplayName = "Transform Payload")
class UDreamMusicPlayerPayload_Transform : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	FTransform Value;
};

UCLASS(DisplayName = "Rotator Payload")
class UDreamMusicPlayerPayload_Rotator : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	FRotator Value;
};

UCLASS(DisplayName = "Name Payload")
class UDreamMusicPlayerPayload_Name : public UDreamMusicPlayerPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
	FName Value;
};
