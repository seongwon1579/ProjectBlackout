# NET — 06. 네트워크 설정 외부화

> `UBlackoutNetworkSettings` — 매칭 API URL · 로비 WS URL · 매칭 동작 플래그를 `DefaultGame.ini` 로 분리. 환경별 ini 오버라이드 가능.

## 클래스

```cpp
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Blackout Network"))
class PROJECTBLACKOUT_API UBlackoutNetworkSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, Category = "Endpoints")
    FString ApiBaseUrl = TEXT("http://localhost:3000");

    UPROPERTY(Config, EditAnywhere, Category = "Endpoints")
    FString LobbyWsUrl = TEXT("ws://localhost:3001");

    UPROPERTY(Config, EditAnywhere, Category = "Behavior")
    bool bAutoTravelOnGameStart = true;
};
```

## 저장 위치

- `Config = Game` → `Config/DefaultGame.ini` 에 저장
- Project Settings → Engine - Blackout Network 에서 에디터로 편집 가능

## 환경별 오버라이드 전략

| 환경 | ini 파일 | 적용 방식 |
|---|---|---|
| Dev (로컬) | `DefaultGame.ini` | 기본값 (localhost) |
| Staging | `Config/Staging/Game.ini` | 커맨드라인 `-ConfigPath=Staging` |
| Prod | `Config/Prod/Game.ini` | 빌드 스크립트에서 파일 교체 |

AWS 데디 배포 시 `-ApiBaseUrl=https://...` 같은 커맨드라인 오버라이드도 가능.

## 필수 모듈 의존성

`Build.cs` 에 `DeveloperSettings` 추가 필수. Engine 모듈에 포함 안됨 — 누락 시 `LNK2019 UDeveloperSettings::StaticClass` 등 link error.

```cs
PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine", "InputCore",
    "DeveloperSettings",  // UBlackoutNetworkSettings
    "HTTP", "Json", "JsonUtilities", "WebSockets",
});
```

## 사용 패턴

```cpp
const UBlackoutNetworkSettings* Settings = GetDefault<UBlackoutNetworkSettings>();
const FString Url = Settings->ApiBaseUrl + TEXT("/auth/login");
```

런타임 수정은 권장 안함. 설정은 시작 시점에 고정.

## 후속 확장 여지

- JWT 만료 시간, 재시도 횟수, WS 재연결 간격 같은 튜닝 값 추가 가능
- 프로덕션 이후 HTTPS / WSS 엔드포인트 전환도 URL 교체만으로 완료
