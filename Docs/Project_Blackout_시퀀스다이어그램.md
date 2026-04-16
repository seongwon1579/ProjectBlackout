# Project Blackout — 시퀀스 다이어그램

> Mermaid 문법 기반. GDD vFinal + TDD v5 + 서버 RnD v2 참조.

---

## ⚔️ 전투 플로우

### 1. 데미지 파이프라인 (사격 → 피격 → 사망 → 드롭)

```mermaid
sequenceDiagram
    actor Attacker as 공격자 (클라이언트)
    participant Server as 데디케이트 서버
    participant Target as 미니언 (서버)
    participant Pool as PoolSubsystem

    Attacker->>Server: 사격 입력 (Server RPC)
    Server->>Server: GA_FireWeapon 활성화

    alt 히트스캔
        Server->>Server: 서버 레이캐스트 + 되감기 보정
    else 프로젝타일
        Server->>Server: 서버 발사체 스폰 + 물리 시뮬
    end

    Server->>Target: GE_Damage 적용
    Server->>Server: ExecCalc_DamageCalc
    Note over Server: FinalDmg = Base x Crit x Zone x (1-Reduction)

    Server-->>Attacker: GCN_HitImpact (혈흔/파편)
    Server-->>Attacker: Client_ShowDamageNumber (데미지 텍스트)

    alt Target HP <= 0
        Server->>Server: ExecCalc_CombatReward 판정
        Note over Server: Kill.Melee / Kill.MultiTarget.Count>=3 / Kill.WeakSpot

        Server->>Pool: GetFromPool(탄약 박스)
        Server->>Pool: GetFromPool(소모품)
        Pool-->>Server: 드롭 아이템 스폰

        Server->>Target: OnReturnToPool() (래그돌 N초 후)
    end
```

---

### 2. 플레이어 다운 → 부활 / 완전 사망

```mermaid
sequenceDiagram
    actor Downed as 다운된 플레이어
    actor Rescuer as 구출자
    participant Server as BattleGameMode (서버)

    Note over Server: HP 0 도달

    Server->>Downed: GE_Downed + GE_BleedOut 적용
    Note over Server: State.Downed 태그, 이동/액션 봉쇄

    par 다운 타이머 진행 (항상)
        loop 매초
            Server->>Server: GE_BleedOut Tick (HP 감소)
        end
    and 부활 시도
        Rescuer->>Server: GA_Revive 시작 (상호작용)
        loop 부활 게이지 진행
            Server->>Server: Revive Progress 증가
        end
    end

    alt 부활 완료 (게이지 100%)
        Server->>Downed: GE_Downed + GE_BleedOut 해제
        Server->>Downed: HP 기본값 복구
        Server->>Rescuer: RelicCharges -1 차감
        Downed->>Downed: 전투 복귀
    else 부활 캔슬
        Note over Server: 게이지 리셋, 타이머 유지 (줄어든 상태)
    else GE_BleedOut 타이머 만료 (타이머 우선)
        Server->>Server: GA_Revive 강제 캔슬
        Server->>Downed: 완전 사망
        Server->>Downed: NAME_Spectating 전환
        Server->>Downed: SetViewTargetWithBlend(아군)
        Note over Downed: 관전 모드 + 재시작 투표 UI
    end
```

---

### 3. 파티 전멸 → 체크포인트 복귀

```mermaid
sequenceDiagram
    participant Server as BattleGameMode (서버)
    participant PS as PlayerState
    actor P1 as Player1
    actor Others as Player2-4

    Server->>Server: Tick — 생존자 카운트 체크

    Note over Server: 생존자 == 0 (전원 완전 사망)

    Server->>Server: Server_RestartAtCheckpoint()
    Server->>Server: CurrentCheckpointTag 참조 (MidBoss or MainBoss)

    Server->>P1: 화톳불 위치로 텔레포트
    Server->>Others: 화톳불 위치로 텔레포트

    Server->>P1: GE_Downed/BleedOut 제거
    Server->>Others: GE_Downed/BleedOut 제거

    Server->>PS: ApplyBattleTransitionPolicy()
    Note over PS: HP/Stamina/유물 최대치, 소모품 최소 1 보정, 보스 HP/페이즈 리셋
```

---

### 4. 관전 중 과반수 재시작 투표

```mermaid
sequenceDiagram
    actor Spec1 as 관전자1
    actor Spec2 as 관전자2
    actor Alive as 생존자
    participant Server as BattleGameMode (서버)

    Spec1->>Server: Server_VoteRestart()
    Server->>Server: VoteCount++ (현재 1/2 필요)

    Spec2->>Server: Server_VoteRestart()
    Server->>Server: VoteCount++ (2/2 → 과반수 달성)

    Server->>Server: Server_RestartAtCheckpoint() 경로 재사용
    Note over Server: 전멸 복귀와 동일 흐름

    Server-->>Spec1: 활성 폰으로 복구
    Server-->>Spec2: 활성 폰으로 복구
    Server-->>Alive: 화톳불로 텔레포트
```

---

## 🧠 전투 서브시스템

### 5. 보스 어그로 시스템 타겟 전환

```mermaid
sequenceDiagram
    participant Aggro as AggroComponent (0.25초 주기)
    participant Boss as BossCharacter
    actor P1 as Player1 (어썰트)
    actor P2 as Player2 (스나이퍼)

    Note over Aggro: 누적 피해: P1=500, P2=200 (격차 60% > 15%)

    Aggro->>Aggro: 1순위 판정 → P1 (누적 피해 최대)
    Aggro->>Boss: BB_CurrentTarget = P1
    Boss->>P1: 공격 패턴 실행

    Note over P1: P1 다운됨!

    Aggro->>Aggro: 현 타겟 다운 → 쿨다운 무시 즉시 전환
    Aggro->>Aggro: 1순위 재평가 → P2만 생존
    Aggro->>Aggro: 2순위 (거리) → P2
    Aggro->>Boss: BB_CurrentTarget = P2
    Boss->>P2: 공격 패턴 실행

    Note over Aggro: 1초마다 DamageAccumulator 2% 감쇠
```

---

### 6. 슈루드 씨앗 기믹 (SeedDrop + 무적)

```mermaid
sequenceDiagram
    participant Shrewd as 슈루드 (서버)
    participant Pool as PoolSubsystem
    actor Players as 플레이어들

    Shrewd->>Shrewd: GA_Shrewd_SeedDrop 발동 (포효)
    Shrewd->>Shrewd: State.Invulnerable 태그 부여

    loop 10-12개 씨앗
        Shrewd->>Pool: GetFromPool(ASeedPod)
        Pool-->>Shrewd: 씨앗 포드 액터
        Shrewd->>Shrewd: 방사형 위치 계산 후 스폰
    end

    Note over Players: 씨앗 포드 파괴 시작

    par 플레이어 씨앗 파괴
        Players->>Pool: 씨앗 HP 0 → ReturnToPool
    and 부화 타이머 만료 시
        Note over Shrewd: 남은 씨앗 → 잡몹 부화
    end

    Note over Shrewd: 전량 파괴 완료

    Shrewd->>Shrewd: State.Invulnerable 태그 해제
    Note over Shrewd: 다시 피격 가능
```

---

### 7. 기둥 파괴 동기화 (Chaos Destruction)

```mermaid
sequenceDiagram
    participant Boss as 약탈자 (서버)
    participant Pillar as ABlackoutDestructiblePillar
    participant GS as GameState
    participant C1 as 클라이언트1
    participant C2 as 클라이언트2

    Boss->>Boss: GA_Ravager_PillarCharge (돌진)
    Boss->>Pillar: 히트박스 충돌 감지

    Pillar->>Pillar: BOPillarHealth <= 0
    Pillar->>Pillar: bIsShattered = true

    Pillar->>C1: Multicast_Shatter(ImpactPoint, ImpactForce)
    Pillar->>C2: Multicast_Shatter(ImpactPoint, ImpactForce)

    Note over C1: Chaos 시뮬레이션 독립 실행
    Note over C2: Chaos 시뮬레이션 독립 실행
    Note over Pillar: 서버는 파괴 플래그만 리플리케이트

    Pillar->>GS: DestroyedPillarIds에 추가 (Replicated)

    Note over C1,C2: 5초 후 잔해 정적화
```

---

## 🗺️ 게임 플로우

### 8. 로비 → 캐릭터 선택 → 전투 맵 진입

```mermaid
sequenceDiagram
    actor P1 as Player1
    actor Others as Player2-4
    participant Server as LobbyGameMode (서버)
    participant PS as PlayerState

    Server->>P1: Client_OpenClassSelectUI (RPC)
    Server->>Others: Client_OpenClassSelectUI (RPC)

    P1->>Server: Server_SelectClass(Tag: Assault)
    Server->>PS: SelectedClassTag = Assault
    PS-->>Others: OnRep_SelectedClassTag (UI 갱신)

    Note over P1: 화톳불 상호작용으로 재선택 가능
    P1->>Server: Server_RequestReopenClassSelect
    Server->>P1: Client_OpenClassSelectUI

    Note over P1,Others: 전원 포털 상호작용

    P1->>Server: Interact(Portal)
    Others->>Server: Interact(Portal)
    Server->>Server: AllPlayersReady() == true

    Server->>PS: ApplyBattleTransitionPolicy()
    Note over PS: 탄약/유물 완전 초기화, 소모품 최소 1 보정

    Server->>Server: ServerTravel("/Maps/Battle_MidBoss")
    Server-->>P1: 맵 전환
    Server-->>Others: 맵 전환
```

---

### 9. 슈루드 → 메인 보스 전환 (StreamLevel)

```mermaid
sequenceDiagram
    actor Players as 플레이어 전원
    participant Server as BattleGameMode (서버)
    participant PS as PlayerState
    participant GS as GameState

    Note over Server: 슈루드 HP == 0

    Server->>Server: OnMidBossDefeated() 델리게이트
    Server->>GS: bMidBossDefeated = true (Replicated)

    Server->>Server: LoadStreamLevel("Level_MainBoss")
    Server->>Server: 메인 보스 구역 게이트 언락

    Server->>Server: CurrentCheckpointTag = Checkpoint.MainBoss
    Note over Server: 장비/어트리뷰트 지속 (레벨 트래블 없음)

    Players->>Players: 화톳불 #2로 이동
    Players->>Server: 포털 상호작용

    Server->>PS: ApplyBattleTransitionPolicy()
    Note over PS: 탄약/유물 완전 초기화, 소모품 보정

    Note over Players: 메인 보스전 시작
```

---

### 10. 메인 보스 클리어 → 승리 → 메인 메뉴

```mermaid
sequenceDiagram
    actor Players as 플레이어 전원
    participant Server as BattleGameMode (서버)
    participant API as Nest.js API

    Note over Server: 약탈자 HP == 0

    Server->>Server: OnMainBossDefeated() 델리게이트
    Server-->>Players: GCN_Victory 승리 연출 (5초)
    Server-->>Players: Client_ShowVictoryScreen RPC

    Note over Server: 5초 대기

    Server->>Players: ClientReturnToMainMenuWithTextReason
    Server->>API: POST /sessions/:id/finish
    API->>API: Redis 세션 삭제, 서버 idle 복귀

    Players->>Players: 메인 메뉴 화면
```

---

## 🌐 매칭/서버 인프라

### 11. 매치메이킹 → 게임 시작 전체 흐름

```mermaid
sequenceDiagram
    actor Player as 클라이언트
    participant API as Nest.js API :3000
    participant WS as EventsGateway :3001
    participant Redis as Redis :6379
    participant Dedi as 데디케이트 서버 :7777

    Player->>API: POST /auth/login {id, pw}
    API-->>Player: {jwt}

    Player->>API: POST /sessions {playerName} [JWT]
    API->>Redis: SET session:{id} (status: waiting, TTL 3분)
    API->>Redis: SET player:{name} → sessionId
    API-->>Player: {sessionId, status: waiting}

    Player->>WS: WebSocket 접속
    WS-->>Player: {event: hi, data: {message: connect to lobby}}

    Player->>WS: {event: join_session, data: {sessionId}}
    WS-->>Player: {event: joined_session, data: {sessionId, count}}

    loop 다른 플레이어 참가
        Player->>API: POST /sessions/:id/join {playerName} [JWT]
        API->>Redis: 세션 players 업데이트
        API->>Redis: SET player:{name} → sessionId
        WS-->>Player: {event: player_joined, players: [...]}
    end

    Note over API,Redis: 4명 도달

    API->>Redis: SET session:{id} (status: playing, TTL 제거)
    API->>Redis: SET token:{token} (TTL 30초)
    WS-->>Player: {event: game_start, serverIp, port, token}

    Player->>Dedi: ClientTravel(serverIp:port) + 입장 토큰
    Dedi->>API: POST /tokens/verify {token}
    API->>Redis: GET token:{token}

    alt 토큰 유효
        API-->>Dedi: {valid: true, playerName}
        Dedi-->>Player: 입장 허용
    else 토큰 무효 또는 만료
        API-->>Dedi: {valid: false}
        Dedi-->>Player: 연결 거부
    end
```

---

### 12. 매치메이킹 타임아웃 + 좀비 키 정리

```mermaid
sequenceDiagram
    actor Player as 클라이언트
    participant Listener as SessionExpirationListener
    participant WS as EventsGateway :3001
    participant Redis as Redis :6379

    Note over Redis: session:{id} TTL 3분 만료

    Redis-->>Listener: __keyevent@0__:expired (session:{id})

    Listener->>Redis: SCAN player:* (sessionId 역추적)
    Redis-->>Listener: 매칭되는 player:{name} 목록

    Listener->>Redis: DEL player:{name1}, player:{name2} ...
    Note over Listener: 좀비 player 키 정리 완료

    Listener->>WS: emitToSession(sessionId, matchmaking_failed)
    WS-->>Player: {event: matchmaking_failed, message: 매칭 시간 초과}

    Player->>Player: 메인 메뉴로 복귀
```

---

### 13. 데디케이트 서버 ↔ API 서버 통신

```mermaid
sequenceDiagram
    participant Dedi as 데디케이트 서버
    participant API as Nest.js API
    participant Redis as Redis

    Note over Dedi: 서버 프로세스 시작

    Dedi->>API: POST /servers/register {ip, port}
    API->>Redis: SET server:{id} (status: idle)

    Note over Dedi: 매치메이킹 완료 → 4명 접속

    Dedi->>API: POST /sessions/:id/start
    API->>Redis: SET session:{id} (status: playing)
    API->>Redis: SET server:{id} (status: playing)

    Note over Dedi: 게임 진행 중...

    opt 플레이어 이탈
        Dedi->>API: POST /sessions/:id/leave {playerName}
        API->>Redis: DEL player:{name}
    end

    Note over Dedi: 게임 종료

    Dedi->>API: POST /sessions/:id/finish
    API->>Redis: DEL session:{id}
    API->>Redis: SET server:{id} (status: idle)

    Note over Dedi: 다음 방 배정 대기
```
