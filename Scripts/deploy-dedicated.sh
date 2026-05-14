#!/bin/bash
set -e

# ──────────────────────────────────────
# 환경 변수# ──────────────────────────────────────
REMOTE_USER="ubuntu"
REMOTE_HOST="3.34.56.88"
REMOTE_PATH="/home/ubuntu/dedicated-server"
KEY_PATH="C:\Users\KGA\Downloads\NetKey.pem"
LOCAL_BUILD="C:\Users\KGA\Documents\Unreal Projects\ProjectBlackout\Build\LinuxServer"   # ⚠️ UE 빌드 출력 경로
# ──────────────────────────────────────


# 빌드 결과 검증
if [ ! -d "$LOCAL_BUILD" ]; then
    echo "❌ 빌드 결과 없음: $LOCAL_BUILD"
    echo "   UE Editor 에서 Platforms > Linux Server 패키지 먼저"
    exit 1
fi

if [ ! -f "$KEY_PATH" ]; then
    echo "❌ SSH 키 없음: $KEY_PATH"
    exit 1
fi

echo "🚀 Uploading $LOCAL_BUILD/ -> $REMOTE_USER@$REMOTE_HOST:$REMOTE_PATH/"
# 1) 원격 폴더 비우기 (rsync --delete 대체)
echo "🧹 Cleaning remote directory..."
ssh -i "$KEY_PATH" "$REMOTE_USER@$REMOTE_HOST" "rm -rf $REMOTE_PATH/*"

# 2) scp 로 전체 업로드 (-r 재귀)
echo "📤 Uploading via scp (전체 업로드, 시간 소요)..."
scp -i "$KEY_PATH" -r "$LOCAL_BUILD"/* \
    "$REMOTE_USER@$REMOTE_HOST:$REMOTE_PATH/"

# 2. 권한 + systemd 재시작
echo "🔁 Restarting service..."
ssh -i "$KEY_PATH" "$REMOTE_USER@$REMOTE_HOST" << 'EOF'
    chmod +x /home/ubuntu/dedicated-server/ProjectBlackoutServer.sh
    sudo systemctl restart blackout-dedicated
    sleep 2
    sudo systemctl status blackout-dedicated --no-pager | head -20
EOF

echo "✅ Deploy complete"
