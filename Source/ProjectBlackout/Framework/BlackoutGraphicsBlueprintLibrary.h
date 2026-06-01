#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Framework/BlackoutGraphicsUserSettings.h"
#include "BlackoutGraphicsBlueprintLibrary.generated.h"

/**
 * 블루프린트에서 그래픽 사용자 설정에 접근하기 위한 도우미 라이브러리입니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGraphicsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Blackout|Graphics")
	static UBlackoutGraphicsUserSettings* GetBlackoutGraphicsUserSettings();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Graphics")
	static bool ApplyBlackoutGraphicsUserSettings(bool bSaveSettings = true);

	UFUNCTION(BlueprintPure, Category = "Blackout|Graphics")
	static bool IsDLSSRuntimeAvailable();

	UFUNCTION(BlueprintPure, Category = "Blackout|Graphics")
	static bool IsReflexRuntimeAvailable();
};
