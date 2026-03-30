#ifndef AI_GLASSES_JNI_H
#define AI_GLASSES_JNI_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeCreate(JNIEnv* env, jobject thiz);

JNIEXPORT void JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeDestroy(JNIEnv* env, jobject thiz, jlong handle);

JNIEXPORT void JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeAddTermDictionary(JNIEnv* env, jobject thiz, 
                                                              jlong handle, 
                                                              jstring standard_term, 
                                                              jobjectArray synonyms);

JNIEXPORT jobject JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeFindBestMatch(JNIEnv* env, jobject thiz, 
                                                         jlong handle, jstring input_text);

JNIEXPORT jobjectArray JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeFindAllMatches(JNIEnv* env, jobject thiz, 
                                                          jlong handle, jstring input_text, 
                                                          jint top_k);

JNIEXPORT void JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeSetSimilarityThreshold(JNIEnv* env, jobject thiz, 
                                                                   jlong handle, jfloat threshold);

JNIEXPORT jfloat JNICALL 
Java_com_aiglasses_SemanticMatcher_nativeGetSimilarityThreshold(JNIEnv* env, jobject thiz, 
                                                                   jlong handle);

#ifdef __cplusplus
}
#endif

#endif
