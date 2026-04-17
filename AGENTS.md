# Project Blackout — AI Agent Instructions

> 이 파일은 모든 AI 코딩 에이전트(Claude Code, Gemini CLI, Copilot, Cursor 등)가 이 프로젝트에서 작업할 때 참조하는 공통 지침입니다.

## 참조 문서 (Just-in-Time 로딩)

> **원칙**: 문서 전체를 무조건 읽지 마세요. 본 AGENTS.md 파일만 항상 로드하고, **작업 유형에 맞는 섹션만** 필요 시 참조하세요. 큰 문서는 `Grep`/`Read`의 `offset`·`limit`으로 해당 섹션만 읽는 것을 권장합니다.

### 문서 인덱스

| 문서 | 경로 | 용도 |
| ---- | ---- | ---- |
| 게임 기획서 (GDD) | [`Docs/게임_기획서_GDD_초안.md`](Docs/게임_기획서_GDD_초안.md) | 게임 플로우, 캐릭터 상성, 보스 설계, UI/UX 기획 |
| 기술 설계서 (TDD) | [`Docs/기술_설계서_TDD_v5.md`](Docs/기술_설계서_TDD_v5.md) | 클래스 계층, GAS, AI BT, 네트워크, 최적화 명세 |
| 개발 컨벤션 | [`Docs/CONVENTIONS.md`](Docs/CONVENTIONS.md) | 에셋/코드 네이밍, Git 브랜치, 커밋, PR 규칙 |
| 클래스 다이어그램 | [`Docs/Project_Blackout_클래스다이어그램.md`](Docs/Project_Blackout_클래스다이어그램.md) | 클래스 관계 시각화 |
| 시퀀스 다이어그램 | [`Docs/Project_Blackout_시퀀스다이어그램.md`](Docs/Project_Blackout_시퀀스다이어그램.md) | 주요 플로우 시퀀스 |

### 작업 유형별 참조 가이드

아래 표에서 본인 작업에 해당하는 행을 찾아 **"반드시 읽기"** 문서만 로드하세요. **"필요 시 참조"** 는 실제로 정보가 부족할 때만 해당 섹션을 열어보세요.

| 작업 유형 | 반드시 읽기 | 필요 시 참조 |
| ---- | ---- | ---- |
| 커밋 / 브랜치 / PR 작성 | 본 파일 §Key Rules | `CONVENTIONS.md` §4 Git 브랜치 전략 및 커밋 메시지 규칙 |
| 새 C++ 클래스 / 파일 추가 | 본 파일 §핵심 클래스 계층 | `CONVENTIONS.md` §2 C++ 코딩 표준, §3 폴더 구조 |
| 에셋 네이밍 / 블루프린트 배치 | 본 파일 §Source & Content Structure | `CONVENTIONS.md` §1 에셋 네이밍, §3 Content 디렉토리 |
| GAS 어빌리티 / 어트리뷰트 구현 | `TDD` §3 어트리뷰트 세트, §4 Gameplay Ability | `TDD` §5 데미지 판정, `GDD` §4 캐릭터 장비 |
| 보스 AI / BT / 어그로 수정 | `TDD` §6 AI BT, §6.1 타겟팅 어그로 | `GDD` §5 슈루드, §6 타락한 약탈자, §6.0 어그로 시스템 |
| 보스 페이즈 / 기믹 설계 | `TDD` §5.2 약점 피해, §8 엄폐물 파괴 | `GDD` §6 메인 보스 3-Phase |
| 매치 플로우 / 체크포인트 / 부활 | `TDD` §7 전투 세션 플로우 | `GDD` §2 핵심 게임 플로우 |
| 네트워크 / 리플리케이션 | `TDD` §1 프레임워크·네트워크, §10 최적화 | `GDD` §7 멀티플레이어 환경 |
| UI / UMG / HUD | `TDD` §9 UI 반응형 바인딩 | `GDD` §8 UI 및 UX 설계 |
| 데이터 에셋 / 밸런스 테이블 | `TDD` §11 데이터 기반 설계 | `GDD` §4 캐릭터 상성 |
| 오브젝트 풀링 / 미니언 스폰 | `TDD` §12 오브젝트 풀링 | — |
| 애니메이션 / 모션 워핑 / 루트 모션 | `TDD` §13 애니메이션 및 로코모션 | — |
| VFX / SFX (Niagara / MetaSounds) | `TDD` §14 시각·음향 효과 | — |
| 단순 버그 수정 / 리팩터 | 변경 파일 주변 코드만 | 관련 섹션만 Grep으로 국소 검색 |
| 문서/주석 수정 (코드 무변경) | — | 해당 문서만 |

> **Tip**: Markdown 섹션을 읽을 때는 `Grep`으로 `^##` 헤더를 찾아 라인 번호를 얻은 뒤 `Read` 도구의 `offset`·`limit`으로 해당 섹션만 로드하면 토큰을 크게 절약할 수 있습니다.

## Project Overview

- **Project Name**: Project Blackout (프로젝트 블랙아웃)
- **Genre**: 3인칭 슈팅(TPS) 기반 다이브 & 소울라이크 4인 협동 PvE (Boss Rush)
- **Platform**: PC (Steam)
- **Engine**: Unreal Engine 5.7.4 (바이너리 빌드, 버전 고정)
- **Network**: Dedicated Server 전용 (Listen Server 미지원), 4인 자동 매치메이킹
- **Language**: C++ with Blueprints
- **Module**: `ProjectBlackout` (Runtime, Default loading phase)
- **Dependencies**: Engine, AIModule, UMG
- **Plugins**: GameplayAbilities, StateTree, GameplayStateTree, ModelingToolsEditorMode
- **Core Systems**: GAS (Gameplay Ability System), Data-Driven Design, Object Pooling

### 게임 플로우

메인 메뉴 → 자동 매치메이킹(4인) → 인게임 로비(캐릭터 선택) → Ready Check → 중간 보스(Shrewd) → 메인 보스(Corrupted Ravager, 3-Phase) → 승리 시 메인 메뉴 복귀. 패배 시 화톳불(체크포인트)에서 즉시 부활하여 빠른 재도전(Fast Retry) 가능.

### 캐릭터 & 역할 상성

- **A — Assault (탱커/어그로)**: 시카고 타자기(기관단총) + 리피터 피스톨 + 강철 검. 높은 HP/SP. 보스 전면 어그로 유지. 근접 처치 시 자원 드랍.
- **B — Demolition (광역 딜)**: 포자피개(샷건) + 메리디안(유탄발사기) + 고철 해머. 중간 HP/낮은 SP. 다수 미니언 광역 소탕. 3마리 이상 동시 처치 시 자원 드랍.
- **C — Sniper (정밀 타격)**: 녹슨 레버액션 소총 + 더블 배럴 + 강철 검. 낮은 HP/중간 SP. 보스 약점·정예 미니언 저격. 치명타 처치 시 자원 드랍.

### 핵심 클래스 계층 (TDD 기준)

```
ABlackoutCharacterBase (최상위: IAbilitySystemInterface)
├── ABlackoutPlayerCharacter (TPS 카메라, UBlackoutCombatComponent)
├── ABlackoutEnemyCharacter (자체 ASC, 오브젝트 풀링 IBlackoutPoolableInterface)
│   └── ABlackoutBossCharacter (다단계 페이즈, UBlackoutAggroComponent, UBOBossData)
```

```
ABlackoutGameMode (서버 베이스)
├── ABlackoutLobbyGameMode (캐릭터 선택, ServerTravel)
└── ABlackoutBattleGameMode (보스전, 체크포인트, 전멸/부활 제어)
```

- **PlayerState** (`ABlackoutPlayerState`): ASC 소유, 어트리뷰트(HP/SP/탄약/유물), 리플리케이션
- **GameState** (`ABlackoutGameState`): 매치 타이머, 기둥 파괴 상태, 페이즈 전환 동기화
- **Data Assets**: `UBOCharacterData`, `DT_WeaponStats`, `UBOMinionData`, `UBOBossData`
- **Subsystem**: `UBlackoutPoolSubsystem` (미니언, 발사체, 드랍 아이템 풀링)

## Source & Content Structure

> 상세 디렉토리 트리: [`Docs/CONVENTIONS.md`](Docs/CONVENTIONS.md#3-폴더-구조-가이드-directory-structure)

- **모듈 루트** (`Source/ProjectBlackout/`): 공용 베이스 클래스 (GameMode, Character, PlayerController)
- **Variant 서브디렉토리**: 언리얼 엔진 템플릿으로 생성된 `Variant_Combat/`, `Variant_Platforming/`, `Variant_SideScrolling/`. 현재 프로젝트에서는 사용하지 않습니다.
- **Content**: 블루프린트는 `_BP/`에 집중, 서드파티 에셋은 `Assets/`에 격리

## Conventions (요약)

> 상세 규칙: [`Docs/CONVENTIONS.md`](Docs/CONVENTIONS.md)

- 클래스 접두어: 기본 `Blackout`, 35자 초과 시 `BO` 약어 허용
- UPROPERTY Category 대분류: `"Blackout|..."` 필수
- 에셋: `Prefix_Name` (PascalCase, 접두사 표 참조)
- Git: `feature/` 브랜치 → `develop` PR. 태그(`Feat`/`Fix`/`Design`/`Refactor`/`Docs`/`Chore`)

## Key Rules for AI Agents

1. **`main`, `develop` 브랜치에 직접 커밋 금지.** 항상 `feature/` 브랜치를 생성하세요.
2. **새 C++ 파일은 모듈 루트(`Source/ProjectBlackout/`) 아래 목적에 맞는 폴더를 생성하여 배치하세요.**
3. **`Variant_Combat/`, `Variant_Platforming/`, `Variant_SideScrolling/` 디렉토리는 건드리지 마세요.** 언리얼 엔진 템플릿 코드이며 이번 프로젝트에서 사용하지 않습니다.
4. **`Content/Assets/` 하위 파일을 수정하거나 구조를 변경하지 마세요.** 서드파티 에셋 원본입니다.
5. **커밋 메시지는 반드시 태그 형식**(`Feat`, `Fix`, `Design`, `Refactor`, `Docs`, `Chore`)을 사용하세요.
6. **코드 변경 전 위 "작업 유형별 참조 가이드" 표를 먼저 확인하고, 해당 섹션만 로드하세요.** 무조건 전체 문서를 읽지 마세요.
