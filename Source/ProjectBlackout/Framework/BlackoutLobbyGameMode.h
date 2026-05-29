#pragma once

#include "CoreMinimal.h"
#include "BlackoutGameMode.h"
#include "UObject/SoftObjectPath.h"
#include "BlackoutLobbyGameMode.generated.h"

UCLASS()
class PROJECTBLACKOUT_API ABlackoutLobbyGameMode : public ABlackoutGameMode
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|Lobby")
	virtual void StartBattle();

	// [테스트 전용] 정원/Ready 무시 강제 ServerTravel. seamless 검증용 (2인 PIE 등).
	UFUNCTION(Exec)
	void BO_ForceStartBattle();

	// 복귀 로비에서 보존된 SelectedClassTag 로 spawn (없으면 DefaultPawnClass).
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

protected:
	
	virtual void BeginPlay() override;
	
	virtual void OnPlayerJoined(APlayerController* NewPlayer) override;

	// 전원 Ready 성립 시 StartBattle 트리거.
	virtual void OnAllPlayersReady() override;

	virtual void OnSeamlessArrival(APlayerController* PC) override;
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Lobby")
	TArray<FSoftObjectPath> BossStageMapPaths;
	
	// ServerTravel 중복 실행 방지 플래그 , StartBattle 최초 1회만
	UPROPERTY(BlueprintReadOnly , Category = "Blackout|Lobby")
	bool bTravelInitiated = false;

private:
	// 로비 도착 공통 처리 (입장=PostLogin, 복귀=seamless). 회복 + 전원 도착 시 ShelterPrep.
	void HandleLobbyArrival(APlayerController* PC);
	
	// 캐릭선택 프리뷰 서브 레벨 
	FName PreviewLevelName = TEXT("L_CharacterPreview");
};
