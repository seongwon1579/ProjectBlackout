#pragma once


#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class UTexture2D;

// 로딩 화면 텍스처 경로 — 위젯과 미리 로드를 담당하는 서브시스템이 공유합니다.
namespace BlackoutLoadingScreen
{
	inline constexpr const TCHAR* BackgroundTexturePath =
		TEXT("/Game/Textures/T_Generic_WhiteWeathered.T_Generic_WhiteWeathered");
	inline constexpr const TCHAR* LogoTexturePath =
		TEXT("/Game/Textures/Logo/T_Blackout_TitleLogo_SharpBT_transparent.T_Blackout_TitleLogo_SharpBT_transparent");
}

/**
 * ClientTravel Loading 스크린
 *
 */
class SBlackoutLoadingScreen : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SBlackoutLoadingScreen){}
		// 미리 로드된 텍스처 주입용. 비워두면 위젯이 직접 LoadObject 합니다(폴백).
		SLATE_ARGUMENT(UTexture2D*, BackgroundTexture)
		SLATE_ARGUMENT(UTexture2D*, LogoTexture)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	// MoviePlayer 로딩 중에도 이미지 리소스가 유지되도록 강한 참조를 보관합니다.
	TStrongObjectPtr<UTexture2D> BackgroundTexture;
	TStrongObjectPtr<UTexture2D> LogoTexture;

	FSlateBrush BackgroundBrush;
	FSlateBrush LogoBrush;

	FVector2D CachedScreenSize = FVector2D::ZeroVector;

	// 화면 안에 들어가는 16:9 기준 영역(레터박스 기준)을 계산합니다.
	FVector2D GetReferenceSize() const;
	float GetLogoWidth() const;
	float GetLogoHeight() const;
};
