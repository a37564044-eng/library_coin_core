#include <jni.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdio>
#include <string>
#include <fstream>
#include <memory>

#include "src/wallet/wallet.h"

extern "C" int larb_run_node(int argc, char* argv[]);
extern "C" void larb_stop_node();
extern "C" const char* larb_get_blockchain_info();
extern "C" const char* larb_mine_once(const larb::Wallet& wallet);

namespace {
std::thread node_thread;
std::mutex node_mutex;
std::atomic<bool> node_running{false};

std::unique_ptr<larb::Wallet> active_wallet;
std::mutex wallet_mutex;
std::string wallet_address;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_larb_node_MainActivity_startNativeNode(
        JNIEnv* env, jobject, jstring dataDir) {
    std::lock_guard<std::mutex> lock(node_mutex);

    if (node_running.load()) {
        return JNI_FALSE;
    }

    if (node_thread.joinable()) {
        node_thread.join();
    }

    if (!dataDir) {
        return JNI_FALSE;
    }

    const char* data_dir_utf = env->GetStringUTFChars(dataDir, nullptr);
    if (!data_dir_utf) {
        return JNI_FALSE;
    }

    std::string data_dir(data_dir_utf);
    env->ReleaseStringUTFChars(dataDir, data_dir_utf);

    node_running.store(true);

    node_thread = std::thread([data_dir]() {
        std::string chain_file = data_dir + "/larb_chain.dat";

        char arg0[] = "larb_node";
        char arg1[] = "8333";
        char arg2[] = "node";
        char arg3[] = "--chain";
        char arg4[4096];

        if (chain_file.size() >= sizeof(arg4)) {
            node_running.store(false);
            return;
        }

        std::snprintf(arg4, sizeof(arg4), "%s", chain_file.c_str());

        char* argv[] = {arg0, arg1, arg2, arg3, arg4};

        larb_run_node(5, argv);

        node_running.store(false);
    });

    return JNI_TRUE;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_larb_node_MainActivity_stopNativeNode(
        JNIEnv*, jobject) {
    larb_stop_node();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_larb_node_MainActivity_isNativeNodeRunning(
        JNIEnv*, jobject) {
    return node_running.load() ? JNI_TRUE : JNI_FALSE;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_larb_node_MainActivity_getBlockchainInfo(
        JNIEnv* env, jobject) {
    const char* info = larb_get_blockchain_info();
    return env->NewStringUTF(
        info ? info : "STATUS: UNKNOWN\n"
    );
}

/*
 * Wallet:
 * Creates a fresh ML-DSA-44 wallet in native memory.
 * The private/secret key is never returned to Java.
 */
extern "C"
JNIEXPORT jstring JNICALL
Java_com_larb_node_MainActivity_createNativeWallet(
        JNIEnv* env, jobject) {

    try {
        std::lock_guard<std::mutex> lock(wallet_mutex);

        active_wallet =
            std::make_unique<larb::Wallet>();

        wallet_address =
            active_wallet->address();

        return env->NewStringUTF(
            wallet_address.c_str()
        );

    } catch (const std::exception& e) {
        return env->NewStringUTF(
            "ERROR: WALLET CREATION FAILED"
        );
    }
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_larb_node_MainActivity_getNativeWalletAddress(
        JNIEnv* env, jobject) {

    std::lock_guard<std::mutex> lock(wallet_mutex);

    if (!active_wallet) {
        return env->NewStringUTF(
            "NO WALLET"
        );
    }

    return env->NewStringUTF(
        wallet_address.c_str()
    );
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_larb_node_MainActivity_mineOnce(
        JNIEnv* env, jobject) {

    try {
        std::lock_guard<std::mutex> lock(wallet_mutex);

        if (!active_wallet) {
            return env->NewStringUTF(
                "ERROR: NO WALLET"
            );
        }

        const char* result =
            larb_mine_once(*active_wallet);

        return env->NewStringUTF(
            result ? result : "ERROR: MINING FAILED"
        );

    } catch (const std::exception&) {
        return env->NewStringUTF(
            "ERROR: MINING FAILED"
        );
    }
}
