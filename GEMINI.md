# Project Blackout — Gemini CLI Instructions

> Gemini CLI 전용 포인터 파일입니다. **모든 공통 지침은 [`AGENTS.md`](AGENTS.md)에 있습니다. 먼저 읽으세요.**
> 상세 컨벤션은 [`Docs/CONVENTIONS.md`](Docs/CONVENTIONS.md) 참조.

## Gemini CLI 고유 팁

- **컨텍스트 계층**: Gemini CLI는 `GEMINI.md`를 자동 로드합니다. 이 파일은 얇게 유지하고, 상세 규칙은 `AGENTS.md` / `CONVENTIONS.md`를 JIT로 참조하세요.
- **섹션 로드**: 큰 문서는 `Grep`으로 `^##` 헤더 라인 번호를 얻은 뒤 `Read`의 `offset`·`limit`으로 해당 구간만 읽어 토큰을 아끼세요.
- **도구 승인**: 파일 쓰기·셸 실행은 사용자 승인을 전제로 하며, 1회 승인은 해당 작업 한정입니다.

## 위험 작업 시 확인

되돌리기 어려운 작업(브랜치 삭제, force push, `.uasset`/`.umap` 덮어쓰기 등)은 실행 전 사용자에게 확인하세요.
