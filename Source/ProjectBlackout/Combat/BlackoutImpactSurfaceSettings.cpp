#include "Combat/BlackoutImpactSurfaceSettings.h"

#include "GameplayTags/BlackoutGameplayTags.h"

UBlackoutImpactSurfaceSettings::UBlackoutImpactSurfaceSettings()
{
	DefaultSurfaceTag = BlackoutGameplayTags::Surface_Default;
	SurfaceTagMap.Add(SurfaceType_Default, BlackoutGameplayTags::Surface_Default);
	SurfaceTagMap.Add(SurfaceType1, BlackoutGameplayTags::Surface_Flesh);
	SurfaceTagMap.Add(SurfaceType2, BlackoutGameplayTags::Surface_Metal);
	SurfaceTagMap.Add(SurfaceType3, BlackoutGameplayTags::Surface_Stone);
}

const UBlackoutImpactSurfaceSettings* UBlackoutImpactSurfaceSettings::Get()
{
	return GetDefault<UBlackoutImpactSurfaceSettings>();
}

FGameplayTag UBlackoutImpactSurfaceSettings::ResolveSurfaceTag(const UPhysicalMaterial* PhysicalMaterial) const
{
	const EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(PhysicalMaterial);
	const TEnumAsByte<EPhysicalSurface> SurfaceKey(SurfaceType);
	if (const FGameplayTag* FoundSurfaceTag = SurfaceTagMap.Find(SurfaceKey))
	{
		if (FoundSurfaceTag->IsValid())
		{
			return *FoundSurfaceTag;
		}
	}

	return DefaultSurfaceTag.IsValid() ? DefaultSurfaceTag : BlackoutGameplayTags::Surface_Default;
}
