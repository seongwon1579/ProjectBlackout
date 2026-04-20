# Project Blackout — Claude Code Instructions

> Claude Code 전용 지침입니다. 공통 규칙은 반드시 아래 파일들을 먼저 읽으세요.

## 필수 참조 문서

- **공통 AI Agent 지침**: [`AGENTS.md`](AGENTS.md)
- **상세 개발 컨벤션**: [`Docs/CONVENTIONS.md`](Docs/CONVENTIONS.md)

## Claude Code 전용 설정

### 작업 시작 전 체크리스트

1. `AGENTS.md`를 읽어 프로젝트 개요와 **"작업 유형별 참조 가이드"** 표를 확인하세요.
2. 본인 작업에 해당하는 행의 **"반드시 읽기"** 문서/섹션만 로드하세요. 전체 문서를 통째로 읽지 마세요.
3. 섹션 단위로 읽을 때는 `Grep`으로 헤더 라인 번호를 얻어 `Read`의 `offset`·`limit`으로 해당 범위만 로드하세요.
4. 현재 브랜치가 `feature/` 브랜치인지 확인하세요. 아니라면 새로 생성하세요.

### 코드 생성 시 주의사항

- Unreal Engine 5 C++ 코드를 생성할 때는 `#include` 경로가 모듈 구조(`ProjectBlackout/...`)와 일치하는지 확인하세요.
- C++ 구현부(.cpp)에서 헤더에 전방 선언(Forward Declaration)된 클래스를 사용하거나 `Cast<>`를 수행할 때, 반드시 해당 클래스의 헤더 파일을 `#include` 하여 `incomplete types` 컴파일 에러가 발생하지 않도록 각별히 주의하세요.
- `.generated.h` 인클루드를 절대 빠뜨리지 마세요.
- `UCLASS()`, `USTRUCT()`, `UENUM()` 매크로 사용 시 적절한 Specifier를 포함하세요.
- 헤더 파일에는 `#pragma once`를 사용하세요.
- 모든 코드 내 주석은 반드시 한글로 작성하세요.

### 커밋 워크플로우

- 커밋 메시지는 `Docs/CONVENTIONS.md`의 태그 형식을 따르세요.
- 커밋 전 변경 파일 목록을 확인하고, `.uasset` / `.umap` 바이너리 파일이 포함되어 있다면 주의하세요.

### PR 워크플로우

- PR 메시지는 `Docs/CONVENTIONS.md`의 형식을 따르세요.
- PR 전 base가 develop가 맞는지 확인하고 절대로 main에 직접 PR를 생성하지 마세요.

### 금지 사항

- `main` 또는 `develop` 브랜치에 직접 커밋하지 마세요.
- `Content/Assets/` 하위 서드파티 에셋을 수정하지 마세요.
- `.uproject`, `.uplugin` 파일을 임의로 수정하지 마세요.
