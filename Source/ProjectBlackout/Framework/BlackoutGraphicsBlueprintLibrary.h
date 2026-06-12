#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Framework/BlackoutUserSettings.h"
#include "BlackoutGraphicsBlueprintLibrary.generated.h"

/**
 * 블루프린트에서 사용자 설정에 접근하기 위한 도우미 라이브러리입니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGraphicsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Blackout|Settings")
	static UBlackoutUserSettings* GetBlackoutUserSettings();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Settings")
	static bool ApplyBlackoutUserSettings(bool bSaveSettings = true);

	UFUNCTION(BlueprintPure, Category = "Blackout|Settings", meta = (DeprecatedFunction, DeprecationMessage = "GetBlackoutUserSettings를 사용하세요."))
	static UBlackoutUserSettings* GetBlackoutGraphicsUserSettings();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Settings", meta = (DeprecatedFunction, DeprecationMessage = "ApplyBlackoutUserSettings를 사용하세요."))
	static bool ApplyBlackoutGraphicsUserSettings(bool bSaveSettings = true);

	UFUNCTION(BlueprintPure, Category = "Blackout|Graphics")
	static bool IsDLSSRuntimeAvailable();

	UFUNCTION(BlueprintPure, Category = "Blackout|Graphics")
	static bool IsReflexRuntimeAvailable();

	UFUNCTION(BlueprintPure, Category = "Blackout|Graphics")
	static bool IsFrameGenerationRuntimeAvailable();

	UFUNCTION(BlueprintPure, Category = "Blackout|Graphics")
	static bool IsFrameGenerationModeRuntimeAvailable(EBlackoutFrameGenerationMode InFrameGenerationMode);
};
