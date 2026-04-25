# Project Blackout — Claude Code Instructions

> Claude Code 전용 포인터 파일입니다. **모든 공통 지침은 [`AGENTS.md`](AGENTS.md)에 있습니다. 먼저 읽으세요.**
> 상세 컨벤션은 [`Docs/CONVENTIONS.md`](Docs/CONVENTIONS.md) 참조.

## Claude Code 고유 팁

- **작업 계획**: 다단계 작업은 `TodoWrite`로 진행 상황을 추적하세요. 작업 완료 즉시 체크 처리합니다.
- **광역 탐색**: 3회 이상 Grep/Glob이 필요한 코드베이스 조사는 `Agent` 도구의 `Explore` subagent로 위임해 메인 컨텍스트를 보호하세요.
- **슬래시 커맨드**: `/review` (PR 리뷰), `/security-review` (보안 검토) 활용 가능.
- **Skill 활용**: `simplify` (변경 코드 정리), `less-permission-prompts` (권한 프롬프트 감소) 등.

## 위험 작업 시 확인

되돌리기 어려운 작업(브랜치 삭제, force push, `.uasset`/`.umap` 덮어쓰기 등)은 실행 전 사용자에게 확인하세요. 1회 승인은 **해당 작업 한정**이며 유사 작업에 재사용하지 마세요.
