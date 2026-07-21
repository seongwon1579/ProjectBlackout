ProjectBlackout
<!-- ======================================================================== [ 메인 이미지 ] ※ 클릭 시 플레이 영상으로 이동하도록 링크 연결 권장 아래 형식으로 교체하세요: [![ProjectBlackout](docs/images/main_thumbnail.png)](플레이영상_URL) ======================================================================== -->

3인칭 슈팅(TPS) 소울라이크 PvE 프로젝트 Unreal Engine 5.7 · GAS 기반 · 데디케이티드 서버 멀티플레이

<br>
목차
프로젝트 개요
게임 소개
전체 아키텍처
담당 파트: AI 시스템
그 외 주요 시스템 (간략)
<br>
프로젝트 개요
	
프로젝트 이름	ProjectBlackout
장르	3인칭 슈팅(TPS) 소울라이크 PvE
엔진	Unreal Engine 5.7 (C++)
인원	팀 프로젝트 (4인)
본인 담당	AI 시스템 (보스/미니언 행동 트리·스테이트 트리, 어그로·페이즈 평가)

팀원별 기여는 각 소스 파일 상단의 구현 내역 주석으로 관리하고 있습니다. 본 문서는 본인(조성원)이 담당한 AI 파트를 중심으로 정리했습니다.

<br>
게임 소개

여러 플레이어가 협동하여 보스와 미니언을 상대하는 PvE 소울라이크입니다. 회피(I-Frame), 근접 콤보, 사격/조준, 무기 스왑 등 소울라이크 특유의 전투 감각을 3인칭 슈팅에 결합했습니다.

<!-- [ 게임플레이 이미지 / GIF 자리 ] --> <!-- ![Gameplay](docs/images/gameplay_overview.png) -->

주요 특징

협동 PvE — 데디케이티드 서버 기반 멀티플레이, 매치메이킹 및 자동 합방 지원
소울라이크 전투 — 회피 무적 프레임, 근접 콤보 윈도우, 다운/부활 시스템
다양한 적 AI — 서로 다른 아키텍처로 설계된 보스 2종과 미니언들
페이즈 기반 보스전 — 체력 구간에 따라 행동 패턴이 전환되는 보스
<br>
전체 아키텍처

프로젝트는 기능별 모듈로 구성되어 있습니다. (본인 담당은 AI 모듈)

Combat
GAS (전투 프레임워크)
AI - 본인 담당
Characters
Framework (게임 흐름)
GameMode / GameState
Matchmaking / Session
Player
Boss / Minion
AI Controllers
Behavior Tree / State Tree
Aggro / Phase Evaluator
Abilities
Attributes
Effects / Cues
Weapons
Combat Component
Object Pool

기술 스택

엔진: Unreal Engine 5.7 / C++
전투: GAS (Gameplay Ability System)
AI: Behavior Tree, State Tree, EQS, AI Perception
네트워크: 데디케이티드 서버, HTTP/WebSocket 매치메이킹
그래픽: DLSS / Reflex (NVIDIA Streamline)
<br>

캐릭터 클래스 계층 (개요)

ACharacter
ABlackoutCharacterBase
+IAbilitySystemInterface
+IBlackoutDamageable
ABlackoutPlayerCharacter
ABlackoutEnemyCharacter
ABlackoutBossCharacter
ABlackoutMinionCharacter
+IBlackoutPoolableInterface
<br>
담당 파트: AI 시스템
<!-- [ AI 시스템 대표 이미지 / 인게임 전투 장면 자리 ] -->

적 AI는 컨트롤러 → 행동 로직(BT/ST) → 어빌리티(GAS) 로 이어지는 구조로 설계했으며, 어그로·페이즈 판정 같은 공통 로직은 별도 평가기(Evaluator)로 분리해 BT·ST 양쪽에서 재사용합니다.

AI 클래스 구조 (UML)

컨트롤러를 계층화해, 공통 기능은 상위 클래스에 두고 적 특성별로 필요한 요소만 하위에서 추가했습니다.

AAIController
ABlackoutAIController
StateTree 컴포넌트 보유
전투 시작 시 AI 구동
ABlackoutBossAIController
어그로 평가기 보유/구독
피해 기록·타겟 전달
ABlackoutRavagerAIController
페이즈별 BT 러너 구동
페이즈 전환 처리
ABlackoutShrewdAIController
StateTree 기반 원거리 보스
ABlackoutMinionAIController
Perception 기반 미니언
UStateTreeAIComponent
UBlackoutAggroEvaluator
UBlackoutBossBTRunner
UBlackoutPhaseEvaluator

설계 포인트 — ABlackoutAIController가 StateTree를, ABlackoutBossAIController가 어그로 평가기를 공통으로 보유합니다. Ravager만 BT 러너·페이즈 평가기를 추가로 소유해, 필요한 적에게만 필요한 시스템을 붙이는 구조입니다.

<br>
두 가지 AI 아키텍처

적의 성격에 맞춰 행동 트리(Behavior Tree) 와 스테이트 트리(State Tree) 를 구분해 적용했습니다.

구분	아키텍처	설계 의도
Ravager (근접 보스)	Behavior Tree	페이즈별 트리 교체, 패턴 위주 전개
Shrewd (원거리 보스)	State Tree	카이팅·텔레포트 등 상태 전환 중심
미니언 (Hollow / Wraith)	State Tree	서브 BT 없이 가볍게 단독 구동
<br>
1. 어그로 평가기 (Aggro Evaluator)

멀티플레이 PvE에서 "적이 누구를 노릴 것인가"를 결정하는 핵심 로직입니다. 단순 근접이 아니라 거리 · DPS · 저체력 을 가중 합산해 타겟을 산정합니다.

플레이어 피해 기록RecordDamage
DPS 윈도우최근 3초만 유지
가중 합산
거리
저체력
최고 점수 타겟 선정
사망/다운 타겟 제외
어그로 타겟 전달OnAggroTargetChanged
DPS 윈도우: 최근 일정 시간(기본 3초) 내 데미지만 누적 → "지금 위협적인 플레이어" 반영
가중치 분리: 거리·DPS·저체력 가중치를 에디터에서 조정 가능(EditAnywhere)
예외 처리: 사망/다운 타겟은 후보에서 제외
BT / ST 공용: 두 아키텍처 모두에서 동일한 평가기 재사용
cpp
// 오래된 데미지 기록은 윈도우를 벗어나면 자동 제거 → 최근 DPS만 반영
float GetDamageInWindow(float CurrentTime, float WindowDuration)
{
    DamageRecords.RemoveAll([=](const FDamageRecord& Record)
    {
        return CurrentTime - Record.Timestamp > WindowDuration;
    });
    // ... 남은 기록 합산
}
<br>
2. 페이즈 평가기 (Phase Evaluator)

보스 체력 구간에 따라 Phase 1 → 2 → 3 으로 행동을 전환합니다. 연출·공격 중 페이즈가 바뀌면 어색하기 때문에, 잠금 태그(Lock Tag) 로 전환을 보류했다가 잠금 해제 시 반영하도록 설계했습니다.

Yes
No
체력 변화 감지
RequestPhaseChange
잠금 태그활성?
PendingPhase 보류
페이즈 적용
잠금 해제 대기
대기 중 페이즈 적용
OnBossPhaseChanged브로드캐스트
BT 러너가 해당 페이즈트리로 교체
<br>
3. Behavior Tree — Ravager (근접 보스)

페이즈별로 서로 다른 Behavior Tree를 보관하고, 페이즈 전환 시 트리를 통째로 교체하는 BT 러너(Runner) 구조로 구현했습니다.

직접 구현한 커스텀 노드

Decorators — 회피 가능 방향 판정(라인 트레이스), 거리 범위 체크, 회전 필요 판정, 랜덤 확률
Services — 타겟 거리/각도 갱신(변경 감지 기반 최소 갱신), 추격 사거리 판정
Tasks — 태그 기반 어빌리티 실행(시작·종료 델리게이트 구독 후 완료 대기), 패턴 선택, 회피/회전 어빌리티 실행

어빌리티 실행 Task는 GAS 어빌리티의 시작·종료 델리게이트를 구독해 실제 어빌리티가 끝날 때까지 노드가 대기하도록 만들어, BT와 GAS의 흐름을 동기화했습니다.

<!-- [ Ravager Behavior Tree 에디터 캡처 자리 ] --> <br>
4. State Tree — Shrewd (원거리 보스) & 미니언

상태 전환이 잦은 원거리/비행 적은 State Tree로 구현했습니다. (카이팅·텔레포트·스트레이프 등 일부 Task는 팀원과 분담)

본인 담당 State Tree 요소

Evaluator — 어그로 타겟을 State Tree로 전달(보스/잡몹 어그로 분리)
Task — 태그로 어빌리티를 실행하고 부여 대기(타임아웃)·핸들 추적까지 처리, 하위 BT 실행 Task
공용 헬퍼 — 컨트롤러/폰/블랙보드/ASC를 일관되게 가져오는 정적 헬퍼(BTNodeHelper), AI 거리·회전 계산 헬퍼(BOAICalcHelper)
<!-- [ Shrewd State Tree 에디터 캡처 자리 ] --> <br>
5. EQS (환경 쿼리)
IsHigher 테스트 — 후보 지점의 고도를 기준으로 점수화. 비행/원거리 보스가 높은 지형으로 텔레포트할 위치를 고르는 데 사용
<!-- [ EQS 시각화 이미지 자리 ] --> <br>
6. 설계에서 신경 쓴 점
공통 로직 중앙화 — 어그로/페이즈 평가, 거리·회전 계산, 노드 접근 헬퍼를 분리해 BT·ST 양쪽에서 재사용
BT/ST ↔ GAS 동기화 — 어빌리티 실행을 델리게이트로 추적해 행동 노드가 실제 어빌리티 종료까지 대기
디버그 편의 — 회피 가능 영역 등 판정을 에디터에서 시각화(디버그 드로우 토글)
<br>
그 외 주요 시스템 (간략)

AI 외 시스템은 팀원들이 담당했으며, 참고용으로 간단히만 정리합니다.

GAS 전투 — 플레이어/적 어빌리티, 어트리뷰트 셋, 데미지·보상 계산(ExecCalc), 히트 큐
전투/무기 — 사격·조준·연사·반동, 무기 스왑, 근접 콤보 윈도우, 착탄 인디케이터
네트워크 — 데디케이티드 세션, WebSocket 로비·매치메이킹, 재접속, 정원별 보스 HP 배율
오브젝트 풀링 — 발사체 등 반복 스폰 액터의 풀 기반 재사용(GC 추적 포함)
UI/HUD — 체력·탄약·소모품 HUD, 로비/파티/매치 결과, 다운/부활 표시
연출/그래픽 — 보스 인트로 시퀀스, 카메라 셰이크, DLSS/Reflex 지원
