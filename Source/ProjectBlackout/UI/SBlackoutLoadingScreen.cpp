#include "SBlackoutLoadingScreen.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	// 16:9 기준 영역 너비 대비 로고 너비 비율
	constexpr float LogoWidthRatio = 0.5f;
	// 16:9 종횡비 (너비 / 높이)
	constexpr float ReferenceAspectRatio = 16.0f / 9.0f;
	constexpr float FallbackScreenWidth = 1920.0f;
	// 로고 텍스처(1915x821) 기준 종횡비. GetSizeX()가 일시적으로 0을 반환할 때만 사용됩니다.
	constexpr float FallbackLogoAspectRatio = 821.0f / 1915.0f;

	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& FallbackImageSize)
	{
		Brush.SetResourceObject(Texture);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.SetImageSize(Texture
			? FVector2D(Texture->GetSizeX(), Texture->GetSizeY())
			: FallbackImageSize);
	}
}

void SBlackoutLoadingScreen::Construct(const FArguments& InArgs)
{
	// 첫 프레임(Tick 이전)부터 올바른 크기를 쓰도록 현재 뷰포트 크기로 초기화. 실패 시 GetReferenceSize 의 fallback 사용.
	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize = FVector2D::ZeroVector;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		if (ViewportSize.X > 0.0f && ViewportSize.Y > 0.0f)
		{
			CachedScreenSize = ViewportSize;
		}
	}

	// 미리 로드된 텍스처가 주입되면 그대로 사용하고, 아니면 직접 로드(폴백).
	UTexture2D* BackgroundTex = InArgs._BackgroundTexture
		? InArgs._BackgroundTexture
		: LoadObject<UTexture2D>(nullptr, BlackoutLoadingScreen::BackgroundTexturePath);
	UTexture2D* LogoTex = InArgs._LogoTexture
		? InArgs._LogoTexture
		: LoadObject<UTexture2D>(nullptr, BlackoutLoadingScreen::LogoTexturePath);

	BackgroundTexture.Reset(BackgroundTex);
	LogoTexture.Reset(LogoTex);

	ConfigureImageBrush(BackgroundBrush, BackgroundTexture.Get(), FVector2D(1920.0f, 1080.0f));
	ConfigureImageBrush(LogoBrush, LogoTexture.Get(), FVector2D(1915.0f, 821.0f));

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	const FSlateBrush* BackgroundImageBrush = BackgroundTexture.IsValid() ? &BackgroundBrush : WhiteBrush;
	const FSlateBrush* LogoImageBrush = LogoTexture.IsValid() ? &LogoBrush : WhiteBrush;
	const FLinearColor BackgroundTint = FLinearColor(0.01f, 0.01f, 0.01f, 1.0f);
	const FLinearColor LogoTint = FLinearColor::White;

	ChildSlot
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFill)
			[
				SNew(SImage)
				.Image(BackgroundImageBrush)
				.ColorAndOpacity(BackgroundTint)
			]
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				// SBox가 너비·높이를 텍스처 종횡비에 맞춰 강제하므로, SImage를 직접 채워 그려도 왜곡이 없습니다.
				// (SScaleBox 불필요 → 첫 프레임 스케일 팝도 없음)
				SNew(SBox)
				.WidthOverride_Lambda([this]()
				{
					return FOptionalSize(GetLogoWidth());
				})
				.HeightOverride_Lambda([this]()
				{
					return FOptionalSize(GetLogoHeight());
				})
				[
					SNew(SImage)
					.Image(LogoImageBrush)
					.ColorAndOpacity(LogoTint)
				]
			]

			// 로딩 텍스트는 로고 바로 아래에 배치합니다.
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("LOADING...")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
				.ColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
				.Justification(ETextJustify::Center)
				.ShadowOffset(FVector2D(0.0f, 2.0f))
				.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f))
			]
		]
	];
}

void SBlackoutLoadingScreen::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	CachedScreenSize = AllottedGeometry.GetLocalSize();
}

FVector2D SBlackoutLoadingScreen::GetReferenceSize() const
{
	const FVector2D ScreenSize = CachedScreenSize.X > 0.0f && CachedScreenSize.Y > 0.0f
		? CachedScreenSize
		: FVector2D(FallbackScreenWidth, FallbackScreenWidth / ReferenceAspectRatio);

	// 화면 안에 들어가는 가장 큰 16:9 영역(레터박스)을 구합니다.
	const float ScreenAspectRatio = ScreenSize.X / ScreenSize.Y;
	if (ScreenAspectRatio >= ReferenceAspectRatio)
	{
		// 화면이 16:9보다 넓음 → 높이에 맞춤
		return FVector2D(ScreenSize.Y * ReferenceAspectRatio, ScreenSize.Y);
	}

	// 화면이 16:9보다 좁음 → 너비에 맞춤
	return FVector2D(ScreenSize.X, ScreenSize.X / ReferenceAspectRatio);
}

float SBlackoutLoadingScreen::GetLogoWidth() const
{
	return GetReferenceSize().X * LogoWidthRatio;
}

float SBlackoutLoadingScreen::GetLogoHeight() const
{
	const float LogoAspectRatio = LogoTexture.IsValid() && LogoTexture->GetSizeX() > 0
		? static_cast<float>(LogoTexture->GetSizeY()) / static_cast<float>(LogoTexture->GetSizeX())
		: FallbackLogoAspectRatio;

	return GetLogoWidth() * LogoAspectRatio;
}
