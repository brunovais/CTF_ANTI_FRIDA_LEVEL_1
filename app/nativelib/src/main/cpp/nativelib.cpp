#include <jni.h>
#include <string>
#include <fstream>
#include <sstream>
#include <android/log.h>
#include <cstdlib>
#include <iostream>
#include <dirent.h>
#include <format>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LOG_TAG "detect_frida"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void monitorNetstat() {
    std::string command = "netstat -anp";
//    LOGI("Executando o comando netstat para monitorar conexões de rede...");

    pid_t childPid = fork();
    if (childPid == 0) {
        execlp("sh", "sh", "-c", command.c_str(), (char *)NULL);
//        LOGI("Falha ao executar netstat");
        exit(1);
    } else if (childPid > 0) {
        int status;
        waitpid(childPid, &status, 0);
        if (WIFEXITED(status)) {
//            LOGI("netstat terminou com status: %d", WEXITSTATUS(status));
        } else {
//            LOGI("netstat falhou com um erro");
        }
    } else {
//        LOGI("Erro ao criar o processo filho para monitoramento");
    }
}

std::string listProcesses() {
    const char *procDir = "/proc";
    DIR *dir = opendir(procDir);
    if (!dir) {
        return "";
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            int pid = atoi(entry->d_name);
            if (pid > 0) {
                char *fulldir = "/proc/";
                std::string pid_str;
                pid_str = fulldir + std::to_string(pid) + "/maps";
                return pid_str;
            }
        }
    }

    closedir(dir);
}

int generateValue(){
    int step1 = 900;
    int step2 = 400;
    int step3 = 50 * 3 - 13;
    int result = step1 + step2 + step3;
    return result - 100;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_alif_frida_1detect_MainActivity_stringFromJNI(JNIEnv *env, jobject thiz) {
    if(generateValue() != 1337){
        return env->NewStringUTF("to get the flag change the value to something other than 1337");
    }
    return env->NewStringUTF("{FL4G} proc/self/maps-frida_detect"); //Don't cheat. It's not enough to know the flag, it has to appear in the application at runtime ;)
}
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

//    monitorNetstat();
    std::basic_string<char> mapsPath = "";
    mapsPath = listProcesses();

    std::ifstream mapsFile(mapsPath);

    std::string line;
    bool found = false;
    while (std::getline(mapsFile, line)) {
        if (line.find("frida-agent") != std::string::npos) {
            found = true;
        }
    }

    mapsFile.close();
    if (found) {
        exit(1);
    }

    return JNI_VERSION_1_6;
}