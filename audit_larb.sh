#!/bin/sh

echo "========================================"
echo "       LARB 5-POINT READ-ONLY AUDIT"
echo "========================================"

echo
echo "[1] ANDROID APK"
APK="$HOME/android-build/larb-android-app/LARB-Node-ARM64-fixed.apk"
SO="$HOME/android-build/larb-android-app/app/src/main/jniLibs/arm64-v8a/liblarb_android.so"

[ -f "$APK" ] && echo "APK       : EXISTS $(du -h "$APK" | cut -f1)" || echo "APK       : MISSING"
[ -f "$SO" ] && echo "Native SO : EXISTS $(du -h "$SO" | cut -f1)" || echo "Native SO : MISSING"

echo
echo "[2] WALLET"
if [ -f wallet_check ]; then
    echo "wallet_check : EXISTS"
    ./wallet_check 2>&1 | head -20
else
    echo "wallet_check : NOT BUILT"
fi

echo
echo "[3] CODE / GIT INTEGRITY"
echo "HEAD:"
git log -1 --oneline 2>/dev/null || echo "Git unavailable"

echo
echo "MODIFIED / UNTRACKED:"
git status --short 2>/dev/null || true

echo
echo "[4] BLOCK 1 / CHAIN DATA"
if [ -f larb_chain.dat ]; then
    echo "larb_chain.dat : EXISTS $(du -h larb_chain.dat | cut -f1)"
else
    echo "larb_chain.dat : MISSING"
fi

echo
echo "Genesis constant:"
grep -R "00032373b4b4b78d9f32723dc913bec6c6eb3a0e2a5db92a16bb0f3be4ed909a" \
    src 2>/dev/null | head -5 || echo "Genesis hash not found by grep"

echo
echo "Difficulty / halving:"
grep -R -E "INITIAL_POW_DIFFICULTY|HALVING|210000" \
    src 2>/dev/null | head -20

echo
echo "[5] P2P / NODE"
echo "P2P bind address:"
grep -n -E "INADDR_(ANY|LOOPBACK)" src/p2p.cpp 2>/dev/null || echo "Not found"

echo
echo "Node binary:"
[ -x larb_node ] && echo "larb_node : EXISTS" || echo "larb_node : MISSING"

echo
echo "========================================"
echo "AUDIT SELESAI — TIDAK ADA FILE DIUBAH"
echo "========================================"
