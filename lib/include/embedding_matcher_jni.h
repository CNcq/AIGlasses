#ifndef EMBEDDING_MATCHER_JNI_H
#define EMBEDDING_MATCHER_JNI_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeCreate(JNIEnv* env, jobject thiz);

JNIEXPORT void JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeDestroy(JNIEnv* env, jobject thiz, jlong handle);

JNIEXPORT jboolean JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeInitialize(JNIEnv* env, jobject thiz, 
                                                       jlong handle, 
                                                       jstring jieba_dict_path,
                                                       jstring embedding_model_path);

JNIEXPORT void JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeAddEnumItem(JNIEnv* env, jobject thiz, 
                                                        jlong handle, 
                                                        jstring id,
                                                        jstring text,
                                                        jobjectArray keywords);

JNIEXPORT jobject JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeFindBestMatch(JNIEnv* env, jobject thiz, 
                                                          jlong handle, jstring input_text);

JNIEXPORT jobjectArray JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeFindAllMatches(JNIEnv* env, jobject thiz, 
                                                           jlong handle, jstring input_text, 
                                                           jint top_k);

JNIEXPORT void JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeSetSimilarityThreshold(JNIEnv* env, jobject thiz, 
                                                                   jlong handle, jfloat threshold);

JNIEXPORT jfloat JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeGetSimilarityThreshold(JNIEnv* env, jobject thiz, 
                                                                   jlong handle);

JNIEXPORT jlong JNICALL 
Java_com_aiglasses_EmbeddingMatcher_nativeGetEnumItemCount(JNIEnv* env, jobject thiz, 
                                                              jlong handle);

#ifdef __cplusplus
}
#endif

#endif
