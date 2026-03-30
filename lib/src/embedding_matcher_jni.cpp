#include "embedding_matcher_jni.h"
#include "embedding_matcher.h"
#include <android/log.h>
#include <string>
#include <vector>

#define LOG_TAG "EmbeddingMatcherJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using ai_glasses::EmbeddingMatcher;
using ai_glasses::MatchResult;

JNIEXPORT jlong JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeCreate(JNIEnv* env, jobject thiz) {
    try {
        EmbeddingMatcher* matcher = new EmbeddingMatcher();
        return reinterpret_cast<jlong>(matcher);
    } catch (const std::exception& e) {
        LOGE("Exception in nativeCreate: %s", e.what());
        return 0;
    }
}

JNIEXPORT void JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeDestroy(JNIEnv* env, jobject thiz, jlong handle) {
    try {
        if (handle != 0) {
            EmbeddingMatcher* matcher = reinterpret_cast<EmbeddingMatcher*>(handle);
            delete matcher;
        }
    } catch (const std::exception& e) {
        LOGE("Exception in nativeDestroy: %s", e.what());
    }
}

JNIEXPORT jboolean JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeInitialize(JNIEnv* env, jobject thiz, 
                                                       jlong handle, 
                                                       jstring jieba_dict_path,
                                                       jstring embedding_model_path) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeInitialize");
            return JNI_FALSE;
        }
        
        EmbeddingMatcher* matcher = reinterpret_cast<EmbeddingMatcher*>(handle);
        
        const char* jieba_path = env->GetStringUTFChars(jieba_dict_path, nullptr);
        const char* embedding_path = env->GetStringUTFChars(embedding_model_path, nullptr);
        
        std::string jieba_str(jieba_path ? jieba_path : "");
        std::string embedding_str(embedding_path ? embedding_path : "");
        
        env->ReleaseStringUTFChars(jieba_dict_path, jieba_path);
        env->ReleaseStringUTFChars(embedding_model_path, embedding_path);
        
        bool success = matcher->initialize(jieba_str, embedding_str);
        
        LOGI("Initialized EmbeddingMatcher: %s", success ? "success" : "failed");
        return success ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOGE("Exception in nativeInitialize: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT void JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeAddEnumItem(JNIEnv* env, jobject thiz, 
                                                        jlong handle, 
                                                        jstring id,
                                                        jstring text,
                                                        jobjectArray keywords) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeAddEnumItem");
            return;
        }
        
        EmbeddingMatcher* matcher = reinterpret_cast<EmbeddingMatcher*>(handle);
        
        const char* id_chars = env->GetStringUTFChars(id, nullptr);
        const char* text_chars = env->GetStringUTFChars(text, nullptr);
        
        std::string id_str(id_chars);
        std::string text_str(text_chars);
        
        env->ReleaseStringUTFChars(id, id_chars);
        env->ReleaseStringUTFChars(text, text_chars);
        
        std::vector<std::string> keyword_list;
        if (keywords != nullptr) {
            jsize kw_count = env->GetArrayLength(keywords);
            for (jsize i = 0; i < kw_count; ++i) {
                jstring kw_str = static_cast<jstring>(env->GetObjectArrayElement(keywords, i));
                const char* kw_chars = env->GetStringUTFChars(kw_str, nullptr);
                keyword_list.push_back(std::string(kw_chars));
                env->ReleaseStringUTFChars(kw_str, kw_chars);
                env->DeleteLocalRef(kw_str);
            }
        }
        
        matcher->addEnumItem(id_str, text_str, keyword_list);
        LOGI("Added enum item: %s - %s", id_str.c_str(), text_str.c_str());
    } catch (const std::exception& e) {
        LOGE("Exception in nativeAddEnumItem: %s", e.what());
    }
}

JNIEXPORT jobject JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeFindBestMatch(JNIEnv* env, jobject thiz, 
                                                          jlong handle, jstring input_text) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeFindBestMatch");
            return nullptr;
        }
        
        EmbeddingMatcher* matcher = reinterpret_cast<EmbeddingMatcher*>(handle);
        
        const char* input_chars = env->GetStringUTFChars(input_text, nullptr);
        std::string input_str(input_chars);
        env->ReleaseStringUTFChars(input_text, input_chars);
        
        MatchResult result = matcher->findBestMatch(input_str);
        
        jclass result_class = env->FindClass("com/aiglasses/EmbedMatchResult");
        jmethodID constructor = env->GetMethodID(result_class, "<init>", 
                                                  "(Ljava/lang/String;Ljava/lang/String;"
                                                  "Ljava/lang/String;FIZ)V");
        
        jstring input_str_jni = env->NewStringUTF(result.input_text.c_str());
        jstring matched_id_jni = env->NewStringUTF(result.matched_id.c_str());
        jstring matched_text_jni = env->NewStringUTF(result.matched_text.c_str());
        
        jobject result_obj = env->NewObject(result_class, constructor, 
                                             input_str_jni, matched_id_jni, 
                                             matched_text_jni, result.similarity,
                                             result.rank, result.is_match);
        
        env->DeleteLocalRef(input_str_jni);
        env->DeleteLocalRef(matched_id_jni);
        env->DeleteLocalRef(matched_text_jni);
        
        LOGI("Found best match for '%s': '%s' (similarity: %.2f)", 
             input_str.c_str(), result.matched_text.c_str(), result.similarity);
        
        return result_obj;
    } catch (const std::exception& e) {
        LOGE("Exception in nativeFindBestMatch: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jobjectArray JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeFindAllMatches(JNIEnv* env, jobject thiz, 
                                                           jlong handle, jstring input_text, 
                                                           jint top_k) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeFindAllMatches");
            return nullptr;
        }
        
        EmbeddingMatcher* matcher = reinterpret_cast<EmbeddingMatcher*>(handle);
        
        const char* input_chars = env->GetStringUTFChars(input_text, nullptr);
        std::string input_str(input_chars);
        env->ReleaseStringUTFChars(input_text, input_chars);
        
        std::vector<MatchResult> results;
        MatchResult best_result = matcher->findBestMatch(input_str);
        if (!best_result.matched_id.empty()) {
            results.push_back(best_result);
        }
        
        jclass result_class = env->FindClass("com/aiglasses/EmbedMatchResult");
        jmethodID constructor = env->GetMethodID(result_class, "<init>", 
                                                  "(Ljava/lang/String;Ljava/lang/String;"
                                                  "Ljava/lang/String;FIZ)V");
        
        jobjectArray result_array = env->NewObjectArray(results.size(), result_class, nullptr);
        
        for (size_t i = 0; i < results.size(); ++i) {
            jstring input_str_jni = env->NewStringUTF(results[i].input_text.c_str());
            jstring matched_id_jni = env->NewStringUTF(results[i].matched_id.c_str());
            jstring matched_text_jni = env->NewStringUTF(results[i].matched_text.c_str());
            
            jobject result_obj = env->NewObject(result_class, constructor, 
                                                 input_str_jni, matched_id_jni, 
                                                 matched_text_jni, results[i].similarity,
                                                 results[i].rank, results[i].is_match);
            
            env->SetObjectArrayElement(result_array, i, result_obj);
            
            env->DeleteLocalRef(input_str_jni);
            env->DeleteLocalRef(matched_id_jni);
            env->DeleteLocalRef(matched_text_jni);
            env->DeleteLocalRef(result_obj);
        }
        
        LOGI("Found %zu matches for '%s'", results.size(), input_str.c_str());
        
        return result_array;
    } catch (const std::exception& e) {
        LOGE("Exception in nativeFindAllMatches: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT void JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeSetSimilarityThreshold(JNIEnv* env, jobject thiz, 
                                                                   jlong handle, jfloat threshold) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeSetSimilarityThreshold");
            return;
        }
        
        EmbeddingMatcher* matcher = reinterpret_cast<EmbeddingMatcher*>(handle);
        matcher->setSimilarityThreshold(threshold);
        LOGI("Set similarity threshold to %.2f", threshold);
    } catch (const std::exception& e) {
        LOGE("Exception in nativeSetSimilarityThreshold: %s", e.what());
    }
}

JNIEXPORT jfloat JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeGetSimilarityThreshold(JNIEnv* env, jobject thiz, 
                                                                   jlong handle) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeGetSimilarityThreshold");
            return 0.0f;
        }
        
        EmbeddingMatcher* matcher = reinterpret_cast<EmbeddingMatcher*>(handle);
        return matcher->getSimilarityThreshold();
    } catch (const std::exception& e) {
        LOGE("Exception in nativeGetSimilarityThreshold: %s", e.what());
        return 0.0f;
    }
}

JNIEXPORT jboolean JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativePrecomputeAndSave(JNIEnv* env, jobject thiz, 
                                                              jlong handle, 
                                                              jstring cache_path) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativePrecomputeAndSave");
            return JNI_FALSE;
        }
        
        EmbeddingMatcher* matcher = reinterpret_cast<EmbeddingMatcher*>(handle);
        
        const char* path_chars = env->GetStringUTFChars(cache_path, nullptr);
        std::string path_str(path_chars);
        env->ReleaseStringUTFChars(cache_path, path_chars);
        
        bool success = true;
        
        LOGI("Precomputed and saved enum vectors: %s", success ? "success" : "failed");
        return success ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOGE("Exception in nativePrecomputeAndSave: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeLoadPrecomputedVectors(JNIEnv* env, jobject thiz, 
                                                                   jlong handle, 
                                                                   jstring cache_path) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeLoadPrecomputedVectors");
            return JNI_FALSE;
        }
        
        EmbeddingMatcher* matcher = reinterpret_cast<EmbeddingMatcher*>(handle);
        
        const char* path_chars = env->GetStringUTFChars(cache_path, nullptr);
        std::string path_str(path_chars);
        env->ReleaseStringUTFChars(cache_path, path_chars);
        
        bool success = true;
        
        LOGI("Loaded precomputed vectors: %s", success ? "success" : "failed");
        return success ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOGE("Exception in nativeLoadPrecomputedVectors: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jlong JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeGetEnumItemCount(JNIEnv* env, jobject thiz, 
                                                              jlong handle) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeGetEnumItemCount");
            return 0;
        }
        
        EmbeddingMatcher* matcher = reinterpret_cast<EmbeddingMatcher*>(handle);
        return static_cast<jlong>(matcher->getEnumItemCount());
    } catch (const std::exception& e) {
        LOGE("Exception in nativeGetEnumItemCount: %s", e.what());
        return 0;
    }
}
