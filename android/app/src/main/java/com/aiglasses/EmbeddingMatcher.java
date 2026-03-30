package com.aiglasses;

import android.util.Log;

public class EmbeddingMatcher {
    private static final String TAG = "EmbeddingMatcher";
    
    static {
        System.loadLibrary("ai_glasses");
    }

    private long nativeHandle;

    public EmbeddingMatcher() {
        nativeHandle = nativeCreate();
    }

    public void destroy() {
        if (nativeHandle != 0) {
            nativeDestroy(nativeHandle);
            nativeHandle = 0;
        }
    }

    public boolean initialize(String jiebaDictPath, String embeddingModelPath) {
        if (nativeHandle != 0) {
            return nativeInitialize(nativeHandle, jiebaDictPath, embeddingModelPath);
        }
        return false;
    }

    public boolean initialize() {
        return initialize("", "");
    }

    public void addEnumItem(String id, String text, String[] keywords) {
        if (nativeHandle != 0) {
            nativeAddEnumItem(nativeHandle, id, text, keywords);
        }
    }

    public void addEnumItem(String id, String text) {
        addEnumItem(id, text, null);
    }

    public EmbedMatchResult findBestMatch(String inputText) {
        if (nativeHandle != 0) {
            return nativeFindBestMatch(nativeHandle, inputText);
        }
        return null;
    }

    public EmbedMatchResult[] findAllMatches(String inputText, int topK) {
        if (nativeHandle != 0) {
            return nativeFindAllMatches(nativeHandle, inputText, topK);
        }
        return null;
    }

    public void setSimilarityThreshold(float threshold) {
        if (nativeHandle != 0) {
            nativeSetSimilarityThreshold(nativeHandle, threshold);
        }
    }

    public float getSimilarityThreshold() {
        if (nativeHandle != 0) {
            return nativeGetSimilarityThreshold(nativeHandle);
        }
        return 0.0f;
    }

    public long getEnumItemCount() {
        if (nativeHandle != 0) {
            return nativeGetEnumItemCount(nativeHandle);
        }
        return 0;
    }

    public boolean precomputeAndSaveEnumVectors(String cachePath) {
        if (nativeHandle != 0) {
            return nativePrecomputeAndSave(nativeHandle, cachePath);
        }
        return false;
    }

    public boolean loadPrecomputedEnumVectors(String cachePath) {
        if (nativeHandle != 0) {
            return nativeLoadPrecomputedVectors(nativeHandle, cachePath);
        }
        return false;
    }

    private native long nativeCreate();
    private native void nativeDestroy(long handle);
    private native boolean nativeInitialize(long handle, String jiebaDictPath, String embeddingModelPath);
    private native void nativeAddEnumItem(long handle, String id, String text, String[] keywords);
    private native EmbedMatchResult nativeFindBestMatch(long handle, String inputText);
    private native EmbedMatchResult[] nativeFindAllMatches(long handle, String inputText, int topK);
    private native void nativeSetSimilarityThreshold(long handle, float threshold);
    private native float nativeGetSimilarityThreshold(long handle);
    private native long nativeGetEnumItemCount(long handle);
    private native boolean nativePrecomputeAndSave(long handle, String cachePath);
    private native boolean nativeLoadPrecomputedVectors(long handle, String cachePath);

    @Override
    protected void finalize() throws Throwable {
        destroy();
        super.finalize();
    }
}
