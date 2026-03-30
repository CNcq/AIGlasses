package com.aiglasses;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.speech.RecognitionListener;
import android.speech.RecognizerIntent;
import android.speech.SpeechRecognizer;
import android.util.Log;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.util.ArrayList;
import java.util.Locale;

public class EmbeddingMatcherActivity extends AppCompatActivity {
    private static final String TAG = "EmbeddingMatcherActivity";
    private static final int REQUEST_RECORD_AUDIO_PERMISSION = 1001;

    private EmbeddingMatcher matcher;
    private SpeechRecognizer speechRecognizer;
    
    private EditText inputText;
    private TextView resultText;
    private ListView matchListView;
    private Button voiceButton;
    private Button matchButton;
    
    private ArrayAdapter<String> listAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_embedding_matcher);

        initViews();
        initEmbeddingMatcher();
        initSpeechRecognizer();
        checkPermissions();
    }

    private void initViews() {
        inputText = findViewById(R.id.inputText);
        resultText = findViewById(R.id.resultText);
        matchListView = findViewById(R.id.matchListView);
        voiceButton = findViewById(R.id.voiceButton);
        matchButton = findViewById(R.id.matchButton);

        listAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, new ArrayList<>());
        matchListView.setAdapter(listAdapter);

        voiceButton.setOnClickListener(v -> startVoiceRecognition());
        matchButton.setOnClickListener(v -> performMatch());
    }

    private void initEmbeddingMatcher() {
        matcher = new EmbeddingMatcher();
        
        boolean initialized = matcher.initialize();
        if (!initialized) {
            Log.e(TAG, "Failed to initialize EmbeddingMatcher");
            Toast.makeText(this, "初始化失败", Toast.LENGTH_SHORT).show();
            return;
        }
        
        matcher.setSimilarityThreshold(0.5f);

        loadDefectEnums();
        
        Log.d(TAG, "EmbeddingMatcher initialized with " + matcher.getEnumItemCount() + " items");
    }

    private void loadDefectEnums() {
        matcher.addEnumItem("DEFECT_001", "墙皮脱落", new String[]{
            "墙皮脱落", "墙面脱落", "墙皮剥落", "墙壁掉皮", "墙体脱落",
            "墙面掉皮", "墙皮损坏", "墙皮缺陷", "墙留坠", "墙皮掉了"
        });

        matcher.addEnumItem("DEFECT_002", "裂缝", new String[]{
            "裂缝", "裂痕", "开裂", "缝隙", "裂纹", "开裂缝", "墙面裂缝",
            "墙体开裂", "墙面裂纹", "裂开"
        });

        matcher.addEnumItem("DEFECT_003", "空鼓", new String[]{
            "空鼓", "空洞", "墙面空鼓", "墙壁空鼓", "墙体空鼓", "敲起来空空的"
        });

        matcher.addEnumItem("DEFECT_004", "渗水", new String[]{
            "渗水", "漏水", "洇水", "潮湿", "水渍", "浸水", "渗水",
            "漏水了", "有水渍"
        });

        matcher.addEnumItem("DEFECT_005", "发霉", new String[]{
            "发霉", "霉菌", "霉斑", "长霉", "霉变", "发黑", "黑斑",
            "长毛了", "有霉点"
        });

        matcher.addEnumItem("DEFECT_006", "脱落", new String[]{
            "脱落", "掉下来", "掉落", "剥离", "脱离", "分离"
        });

        Log.d(TAG, "Loaded " + matcher.getEnumItemCount() + " defect types");
    }

    private void initSpeechRecognizer() {
        speechRecognizer = SpeechRecognizer.createSpeechRecognizer(this);
        speechRecognizer.setRecognitionListener(new RecognitionListener() {
            @Override
            public void onReadyForSpeech(Bundle params) {
                Log.d(TAG, "Ready for speech");
                runOnUiThread(() -> Toast.makeText(EmbeddingMatcherActivity.this, 
                    "请开始说话", Toast.LENGTH_SHORT).show());
            }

            @Override
            public void onBeginningOfSpeech() {
                Log.d(TAG, "Beginning of speech");
            }

            @Override
            public void onRmsChanged(float rmsdB) {
            }

            @Override
            public void onBufferReceived(byte[] buffer) {
            }

            @Override
            public void onEndOfSpeech() {
                Log.d(TAG, "End of speech");
            }

            @Override
            public void onError(int error) {
                Log.e(TAG, "Speech recognition error: " + error);
                runOnUiThread(() -> Toast.makeText(EmbeddingMatcherActivity.this, 
                    "语音识别失败：" + error, Toast.LENGTH_SHORT).show());
            }

            @Override
            public void onResults(Bundle results) {
                ArrayList<String> matches = results.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION);
                if (matches != null && !matches.isEmpty()) {
                    String text = matches.get(0);
                    inputText.setText(text);
                    Log.d(TAG, "Recognized text: " + text);
                    
                    performMatch();
                }
            }

            @Override
            public void onPartialResults(Bundle partialResults) {
            }

            @Override
            public void onEvent(int eventType, Bundle params) {
            }
        });
    }

    private void checkPermissions() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.RECORD_AUDIO},
                    REQUEST_RECORD_AUDIO_PERMISSION);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_RECORD_AUDIO_PERMISSION) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                Log.d(TAG, "Record audio permission granted");
            } else {
                Toast.makeText(this, "需要录音权限", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void startVoiceRecognition() {
        Intent intent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
        intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL, RecognizerIntent.LANGUAGE_MODEL_FREE_FORM);
        intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE, Locale.CHINA);
        intent.putExtra(RecognizerIntent.EXTRA_PROMPT, "请描述缺陷...");
        speechRecognizer.startListening(intent);
    }

    private void performMatch() {
        String text = inputText.getText().toString().trim();
        if (text.isEmpty()) {
            Toast.makeText(this, "请输入或录制文本", Toast.LENGTH_SHORT).show();
            return;
        }

        EmbedMatchResult bestMatch = matcher.findBestMatch(text);
        if (bestMatch != null) {
            StringBuilder sb = new StringBuilder();
            sb.append("输入文本：").append(bestMatch.inputText).append("\n\n");
            sb.append("最佳匹配结果:\n");
            sb.append("  缺陷类型：").append(bestMatch.matchedText).append("\n");
            sb.append("  缺陷 ID: ").append(bestMatch.matchedId).append("\n");
            sb.append("  相似度：").append(String.format("%.4f", bestMatch.similarity)).append("\n");
            sb.append("  匹配成功：").append(bestMatch.isMatch ? "是 ✓" : "否 ✗");

            resultText.setText(sb.toString());
            Log.d(TAG, "Best match: " + bestMatch);
        } else {
            resultText.setText("匹配失败");
        }

        EmbedMatchResult[] allMatches = matcher.findAllMatches(text, 5);
        if (allMatches != null) {
            listAdapter.clear();
            for (int i = 0; i < allMatches.length; i++) {
                EmbedMatchResult match = allMatches[i];
                String item = String.format("%d. [%s] %s (相似度：%.4f) %s",
                        i + 1,
                        match.matchedId,
                        match.matchedText,
                        match.similarity,
                        match.isMatch ? "✓" : "✗");
                listAdapter.add(item);
            }
            listAdapter.notifyDataSetChanged();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (speechRecognizer != null) {
            speechRecognizer.destroy();
        }
        if (matcher != null) {
            matcher.destroy();
        }
    }
}
