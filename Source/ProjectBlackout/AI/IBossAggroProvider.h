#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IBossAggroProvider.generated.h"

UINTERFACE(MinimalAPI)
class UBossAggroProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 보스 어그로 시스템 공급자 인터페이스.
 *
 * 보스 Pawn이 구현한다. AggroTargetHandler는 이 인터페이스에만 의존하므로
 * 구체 어그로 구현(컴포넌트, 서버 로직 등)이 바뀌어도 핸들러 코드는 변경 불필요.
 *
 * 미구현 상태에서는 AggroTargetHandler가 기존 Data.Target을 유지(fallback).
 */
class IBossAggroProvider
{
	GENERATED_BODY()

public:
	/** 현재 어그로 테이블에서 최고 위협 타겟을 반환한다. */
	virtual APawn* GetHighestAggroTarget() const = 0;

	/**
	 * 특정 Pawn의 위협 수치를 누적한다.
	 * GA 실행 후 데미지/힐 이벤트에서 호출해 어그로를 갱신한다.
	 */
	virtual void AddThreat(APawn* Source, float Amount) = 0;
};
