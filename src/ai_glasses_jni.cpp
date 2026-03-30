#include "ai_glasses_jni.h"
#include "semantic_matcher.h"
#include <android/log.h>
#include <string>
#include <vector>

#define LOG_TAG "AIGlassesJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using ai_glasses::SemanticMatcher;
using ai_glasses::MatchResult;

JNIEXPORT jlong JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeCreate(JNIEnv* env, jobject thiz) {
    try {
        SemanticMatcher* matcher = new SemanticMatcher();
        return reinterpret_cast<jlong>(matcher);
    } catch (const std::exception& e) {
        LOGE("Exception in nativeCreate: %s", e.what());
        return 0;
    }
}

JNIEXPORT void JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeDestroy(JNIEnv* env, jobject thiz, jlong handle) {
    try {
        if (handle != 0) {
            SemanticMatcher* matcher = reinterpret_cast<SemanticMatcher*>(handle);
            delete matcher;
        }
    } catch (const std::exception& e) {
        LOGE("Exception in nativeDestroy: %s", e.what());
    }
}

JNIEXPORT void JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeAddTermDictionary(JNIEnv* env, jobject thiz,
                                                              jlong handle,
                                                              jstring standard_term,
                                                              jobjectArray synonyms) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeAddTermDictionary");
            return;
        }
        
        SemanticMatcher* matcher = reinterpret_cast<SemanticMatcher*>(handle);
        
        const char* term_chars = env->GetStringUTFChars(standard_term, nullptr);
        std::string std_term(term_chars);
        env->ReleaseStringUTFChars(standard_term, term_chars);
        
        std::vector<std::string> syn_list;
        jsize syn_count = env->GetArrayLength(synonyms);
        
        for (jsize i = 0; i < syn_count; ++i) {
            jstring syn_str = static_cast<jstring>(env->GetObjectArrayElement(synonyms, i));
            const char* syn_chars = env->GetStringUTFChars(syn_str, nullptr);
            syn_list.push_back(std::string(syn_chars));
            env->ReleaseStringUTFChars(syn_str, syn_chars);
            env->DeleteLocalRef(syn_str);
        }
        
        matcher->addTermDictionary(std_term, syn_list);
        LOGI("Added term dictionary: %s with %zu synonyms", std_term.c_str(), syn_list.size());
    } catch (const std::exception& e) {
        LOGE("Exception in nativeAddTermDictionary: %s", e.what());
    }
}

JNIEXPORT jobject JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeFindBestMatch(JNIEnv* env, jobject thiz,
                                                         jlong handle, jstring input_text) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeFindBestMatch");
            return nullptr;
        }
        
        SemanticMatcher* matcher = reinterpret_cast<SemanticMatcher*>(handle);
        
        const char* input_chars = env->GetStringUTFChars(input_text, nullptr);
        std::string input_str(input_chars);
        env->ReleaseStringUTFChars(input_text, input_chars);
        
        MatchResult result = matcher->findBestMatch(input_str);
        
        jclass result_class = env->FindClass("com/aiglasses/MatchResult");
        jmethodID constructor = env->GetMethodID(result_class, "<init>", 
                                                  "(Ljava/lang/String;Ljava/lang/String;FZ)V");
        
        jstring original_str = env->NewStringUTF(result.original_text.c_str());
        jstring matched_str = env->NewStringUTF(result.matched_term.c_str());
        
        jobject result_obj = env->NewObject(result_class, constructor, 
                                             original_str, matched_str, 
                                             result.similarity, result.is_match);
        
        env->DeleteLocalRef(original_str);
        env->DeleteLocalRef(matched_str);
        
        LOGI("Found best match for '%s': '%s' (similarity: %.2f)", 
             input_str.c_str(), result.matched_term.c_str(), result.similarity);
        
        return result_obj;
    } catch (const std::exception& e) {
        LOGE("Exception in nativeFindBestMatch: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jobjectArray JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeFindAllMatches(JNIEnv* env, jobject thiz,
                                                          jlong handle, jstring input_text,
                                                          jint top_k) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeFindAllMatches");
            return nullptr;
        }
        
        SemanticMatcher* matcher = reinterpret_cast<SemanticMatcher*>(handle);
        
        const char* input_chars = env->GetStringUTFChars(input_text, nullptr);
        std::string input_str(input_chars);
        env->ReleaseStringUTFChars(input_text, input_chars);
        
        std::vector<MatchResult> results = matcher->findAllMatches(input_str, top_k);
        
        jclass result_class = env->FindClass("com/aiglasses/MatchResult");
        jmethodID constructor = env->GetMethodID(result_class, "<init>", 
                                                  "(Ljava/lang/String;Ljava/lang/String;FZ)V");
        
        jobjectArray result_array = env->NewObjectArray(results.size(), result_class, nullptr);
        
        for (size_t i = 0; i < results.size(); ++i) {
            jstring original_str = env->NewStringUTF(results[i].original_text.c_str());
            jstring matched_str = env->NewStringUTF(results[i].matched_term.c_str());
            
            jobject result_obj = env->NewObject(result_class, constructor, 
                                                 original_str, matched_str, 
                                                 results[i].similarity, results[i].is_match);
            
            env->SetObjectArrayElement(result_array, i, result_obj);
            
            env->DeleteLocalRef(original_str);
            env->DeleteLocalRef(matched_str);
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
Java_com_aiglasses_SemanticMatcher_nativeSetSimilarityThreshold(JNIEnv* env, jobject thiz,
                                                                   jlong handle, jfloat threshold) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeSetSimilarityThreshold");
            return;
        }
        
        SemanticMatcher* matcher = reinterpret_cast<SemanticMatcher*>(handle);
        matcher->setSimilarityThreshold(threshold);
        LOGI("Set similarity threshold to %.2f", threshold);
    } catch (const std::exception& e) {
        LOGE("Exception in nativeSetSimilarityThreshold: %s", e.what());
    }
}

JNIEXPORT jfloat JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeGetSimilarityThreshold(JNIEnv* env, jobject thiz,
                                                                   jlong handle) {
    try {
        if (handle == 0) {
            LOGE("Invalid handle in nativeGetSimilarityThreshold");
            return 0.0f;
        }
        
        SemanticMatcher* matcher = reinterpret_cast<SemanticMatcher*>(handle);
        return matcher->getSimilarityThreshold();
    } catch (const std::exception& e) {
        LOGE("Exception in nativeGetSimilarityThreshold: %s", e.what());
        return 0.0f;
    }
}
