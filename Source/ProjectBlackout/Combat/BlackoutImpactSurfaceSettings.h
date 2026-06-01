#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "BlackoutImpactSurfaceSettings.generated.h"

/**
 * 피지컬 머티리얼 SurfaceType을 전투 표면 태그로 변환하는 프로젝트 설정.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Blackout Impact Surface Settings"))
class PROJECTBLACKOUT_API UBlackoutImpactSurfaceSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBlackoutImpactSurfaceSettings();

	static const UBlackoutImpactSurfaceSettings* Get();

	FGameplayTag ResolveSurfaceTag(const UPhysicalMaterial* PhysicalMaterial) const;

protected:
	/** 피지컬 머티리얼 SurfaceType별 전투 표면 태그 매핑입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Cue")
	TMap<TEnumAsByte<EPhysicalSurface>, FGameplayTag> SurfaceTagMap;

	/** PhysMaterial이 없거나 매핑되지 않았을 때 사용할 기본 표면 태그입니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Cue")
	FGameplayTag DefaultSurfaceTag;
};
