# Project Blackout

> 3인칭 슈팅(TPS) 기반 다이브 & 소울라이크 **4인 협동 PvE 보스 러시** 게임

| 항목 | 내용 |
| ---- | ---- |
| 장르 | TPS + 소울라이크, 4인 협동 PvE (Boss Rush) |
| 플랫폼 | PC (Steam) |
| 엔진 | Unreal Engine **5.7.4** (바이너리 빌드, 버전 고정) |
| 플레이어 수 | 1인 (혼자하기), 4인 (멀티플레이) |
| 네트워크 | Dedicated Server 전용 (Listen Server 미지원), 4인 자동 매치메이킹 |
| 언어 | C++ + Blueprints |
| 핵심 시스템 | GAS(Gameplay Ability System), 데이터 기반 설계, 오브젝트 풀링 |

## 게임 소개

4명의 플레이어가 팀을 이루어 중간 보스 **약삭빠름(Shrewd)** 를 거쳐 3페이즈로 구성된 메인 보스 **타락한 약탈자(Corrupted Ravager)** 에 도전하는 보스 러시 게임입니다.

```
메인 메뉴 → 자동 매치메이킹(4인) → 인게임 로비(캐릭터 선택) → Ready Check
→ 중간 보스(Shrewd) → 메인 보스(Corrupted Ravager, 3-Phase) → 승리 시 메인 메뉴 복귀
```

### 캐릭터 & 역할 상성

| 캐릭터 | 역할 | 주무장 | 특징 |
| ---- | ---- | ---- | ---- |
| **A — Assault** | 탱커 / 어그로 | 시카고 타자기(기관단총), 리피터 피스톨, 강철 검 | 높은 HP/SP, 보스 전면 어그로 유지, 근접 처치 시 자원 드랍 |
| **B — Demolition** | 광역 딜 | 포자피개(샷건), 메리디안(유탄발사기), 고철 해머 | 다수 미니언 광역 소탕, 3마리 이상 동시 처치 시 자원 드랍 |
| **C — Sniper** | 정밀 타격 | 녹슨 레버액션 소총, 더블 배럴, 강철 검 | 보스 약점·정예 미니언 저격, 치명타 처치 시 자원 드랍 |

## 프로젝트 구조

```
ProjectBlackout/
 ┣ Source/ProjectBlackout/   ← C++ 게임 모듈 (GAS, AI, 네트워크, 풀링 등)
 ┣ Content/
 ┃ ┣ _BP/                    ← 프로젝트의 모든 블루프린트
 ┃ ┗ Assets/                 ← 서드파티 외부 에셋 (Git 미추적)
 ┣ Config/                   ← 엔진·게임 설정
 ┣ Docs/                     ← 기획·설계 문서
 ┗ Scripts/                  ← 보조 스크립트
```

> `Source/ProjectBlackout/Variant_*` 디렉토리는 언리얼 템플릿이 생성한 샘플 코드로, 프로젝트에서 사용하지 않습니다.

## 문서

| 문서 | 설명 |
| ---- | ---- |
| [게임 기획서 (GDD)](Docs/게임_기획서_GDD.md) | 게임 플로우, 캐릭터 상성, 보스 설계, UI/UX 기획 |
| [기술 설계서 (TDD)](Docs/기술_설계서_TDD.md) | 클래스 계층, GAS, AI, 네트워크, 최적화 명세 |
| [개발 컨벤션](Docs/CONVENTIONS.md) | 에셋/코드 네이밍, Git 브랜치, 커밋, PR 규칙 |
| [클래스 다이어그램](Docs/Project_Blackout_클래스다이어그램.md) | 전체 클래스 관계 개요 (도메인별 상세는 `Docs/Combat/`, `Docs/Foundation/`, `Docs/AI_Boss/`, `Docs/NET/`, `Docs/UI/` 참조) |
| [시퀀스 다이어그램](Docs/Project_Blackout_시퀀스다이어그램.md) | 주요 플로우 시퀀스 |

## 라이선스

이 프로젝트의 코드는 [MIT License](LICENSE)를 따릅니다. 단, `Content/Assets/`의 외부 에셋은 각 에셋 제작자의 라이선스를 따르며 저장소에 포함되지 않습니다.
