package com.aiglasses;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;

public class ResourceManager {
    private static final String TAG = "ResourceManager";
    private static final String ASSETS_DIR = "aiglasses";
    private static final String MODELS_DIR = "models";
    private static final String CONFIG_DIR = "config";
    private static final String DICT_DIR = "dict";
    private static final String CACHE_DIR = "cache";
    
    private Context context;
    private String basePath;
    private boolean initialized;
    
    public ResourceManager(Context context) {
        this.context = context;
        this.basePath = context.getFilesDir().getAbsolutePath();
        this.initialized = false;
    }
    
    public boolean initialize() {
        return initialize(basePath);
    }
    
    public boolean initialize(String basePath) {
        try {
            this.basePath = basePath;
            
            createDirectory(basePath + "/" + MODELS_DIR);
            createDirectory(basePath + "/" + CONFIG_DIR);
            createDirectory(basePath + "/" + DICT_DIR);
            createDirectory(basePath + "/" + CACHE_DIR);
            
            copyAssetsFromAssets();
            
            initialized = true;
            Log.d(TAG, "ResourceManager initialized at: " + basePath);
            return true;
        } catch (Exception e) {
            Log.e(TAG, "Failed to initialize ResourceManager", e);
            return false;
        }
    }
    
    private void createDirectory(String path) {
        File dir = new File(path);
        if (!dir.exists()) {
            dir.mkdirs();
        }
    }
    
    private void copyAssetsFromAssets() throws IOException {
        AssetManager assetManager = context.getAssets();
        
        copyAssetFile(assetManager, MODELS_DIR + "/embedding.txt", MODELS_DIR + "/embedding.txt");
        copyAssetFile(assetManager, CONFIG_DIR + "/defects.json", CONFIG_DIR + "/defects.json");
        copyAssetFile(assetManager, DICT_DIR + "/jieba.dict", DICT_DIR + "/jieba.dict");
    }
    
    private void copyAssetFile(AssetManager assetManager, String sourcePath, String destPath) throws IOException {
        File destFile = new File(basePath + "/" + destPath);
        
        if (destFile.exists()) {
            return;
        }
        
        try (InputStream in = assetManager.open(ASSETS_DIR + "/" + sourcePath);
             OutputStream out = new FileOutputStream(destFile)) {
            
            byte[] buffer = new byte[4096];
            int read;
            while ((read = in.read(buffer)) != -1) {
                out.write(buffer, 0, read);
            }
            out.flush();
            
            Log.d(TAG, "Copied asset: " + sourcePath + " -> " + destPath);
        } catch (IOException e) {
            Log.w(TAG, "Asset not found: " + ASSETS_DIR + "/" + sourcePath);
        }
    }
    
    public boolean copyModelFromAsset(String assetName, String destName) {
        try {
            AssetManager assetManager = context.getAssets();
            String destPath = basePath + "/" + MODELS_DIR + "/" + destName;
            
            File destFile = new File(destPath);
            if (destFile.exists()) {
                Log.d(TAG, "Model already exists: " + destName);
                return true;
            }
            
            try (InputStream in = assetManager.open(ASSETS_DIR + "/" + MODELS_DIR + "/" + assetName);
                 OutputStream out = new FileOutputStream(destFile)) {
                
                byte[] buffer = new byte[4096];
                int read;
                while ((read = in.read(buffer)) != -1) {
                    out.write(buffer, 0, read);
                }
                out.flush();
                
                Log.d(TAG, "Copied model: " + assetName + " -> " + destPath);
                return true;
            }
        } catch (IOException e) {
            Log.e(TAG, "Failed to copy model: " + assetName, e);
            return false;
        }
    }
    
    public boolean loadConfigFromFile(String configPath) {
        File configFile = new File(configPath);
        if (!configFile.exists()) {
            Log.e(TAG, "Config file not found: " + configPath);
            return false;
        }
        
        try {
            byte[] buffer = new byte[(int) configFile.length()];
            try (InputStream in = new java.io.FileInputStream(configFile)) {
                in.read(buffer);
            }
            
            String jsonContent = new String(buffer, "UTF-8");
            Log.d(TAG, "Loaded config from: " + configPath);
            return true;
        } catch (IOException e) {
            Log.e(TAG, "Failed to load config", e);
            return false;
        }
    }
    
    public String getModelPath(String modelName) {
        return basePath + "/" + MODELS_DIR + "/" + modelName;
    }
    
    public String getConfigPath(String configName) {
        return basePath + "/" + CONFIG_DIR + "/" + configName;
    }
    
    public String getDictPath(String dictName) {
        return basePath + "/" + DICT_DIR + "/" + dictName;
    }
    
    public String getCachePath(String cacheName) {
        return basePath + "/" + CACHE_DIR + "/" + cacheName;
    }
    
    public String getBasePath() {
        return basePath;
    }
    
    public boolean isInitialized() {
        return initialized;
    }
    
    public void clearCache() {
        File cacheDir = new File(basePath + "/" + CACHE_DIR);
        if (cacheDir.exists()) {
            deleteDirectory(cacheDir);
            Log.d(TAG, "Cache cleared");
        }
    }
    
    private void deleteDirectory(File dir) {
        if (dir.isDirectory()) {
            File[] files = dir.listFiles();
            if (files != null) {
                for (File file : files) {
                    deleteDirectory(file);
                }
            }
            dir.delete();
        } else {
            dir.delete();
        }
    }
}
