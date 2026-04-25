#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BossPhaseManager.generated.h"

class ABlackoutBossAIController;
class UStateTreeAIComponent;

/**
 * 보스 AI의 StateTree(페이즈) 생명주기를 담당한다.
 * - StateTree 시작 / 정지
 * - 페이즈 전환 요청 (BTRunner에 안전 종료 신호)
 *
 * 서버 전용(Dedicated Server): 모든 public 메서드는 Authority 검증 후 실행된다.
 * 컨트롤러가 Owner이므로 GC는 컨트롤러 수명에 묶인다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBossPhaseManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(ABlackoutBossAIController* InOwner, UStateTreeAIComponent* InStateTreeComp);

	/** StateTree 실행 시작 (서버 전용) */
	void Start();

	/** StateTree 정지 */
	void Stop(const FString& Reason);

private:
	bool HasAuthority() const;

	TWeakObjectPtr<ABlackoutBossAIController> OwnerController;
	TWeakObjectPtr<UStateTreeAIComponent>     StateTreeComp;
};