package com.aiglasses;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.speech.RecognitionListener;
import android.speech.RecognizerIntent;
import android.speech.SpeechRecognizer;
import android.util.Log;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.util.ArrayList;
import java.util.Locale;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private static final int REQUEST_RECORD_AUDIO_PERMISSION = 1001;

    private SemanticMatcher matcher;
    private SpeechRecognizer speechRecognizer;
    private EditText inputText;
    private TextView resultText;
    private Button voiceButton;
    private Button matchButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initViews();
        initSemanticMatcher();
        initSpeechRecognizer();
        checkPermissions();
    }

    private void initViews() {
        inputText = findViewById(R.id.inputText);
        resultText = findViewById(R.id.resultText);
        voiceButton = findViewById(R.id.voiceButton);
        matchButton = findViewById(R.id.matchButton);

        voiceButton.setOnClickListener(v -> startVoiceRecognition());
        matchButton.setOnClickListener(v -> performMatch());
    }

    private void initSemanticMatcher() {
        matcher = new SemanticMatcher();
        matcher.setSimilarityThreshold(0.5f);

        matcher.addTermDictionary("墙皮脱落", new String[]{
                "墙留坠", "墙皮掉了", "墙面脱落", "墙皮剥落", "墙壁掉皮",
                "墙体脱落", "墙面掉皮", "墙皮损坏", "墙皮缺陷"
        });

        matcher.addTermDictionary("裂缝", new String[]{
                "裂痕", "开裂", "缝隙", "裂纹", "开裂缝", "墙面裂缝"
        });

        matcher.addTermDictionary("空鼓", new String[]{
                "空洞", "墙面空鼓", "墙壁空鼓", "墙体空鼓"
        });

        Log.d(TAG, "SemanticMatcher initialized");
    }

    private void initSpeechRecognizer() {
        speechRecognizer = SpeechRecognizer.createSpeechRecognizer(this);
        speechRecognizer.setRecognitionListener(new RecognitionListener() {
            @Override
            public void onReadyForSpeech(Bundle params) {
                Log.d(TAG, "Ready for speech");
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
                Toast.makeText(MainActivity.this, "语音识别失败", Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onResults(Bundle results) {
                ArrayList<String> matches = results.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION);
                if (matches != null && !matches.isEmpty()) {
                    String text = matches.get(0);
                    inputText.setText(text);
                    Log.d(TAG, "Recognized text: " + text);
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
        intent.putExtra(RecognizerIntent.EXTRA_PROMPT, "请说话...");
        speechRecognizer.startListening(intent);
        Toast.makeText(this, "请开始说话", Toast.LENGTH_SHORT).show();
    }

    private void performMatch() {
        String text = inputText.getText().toString().trim();
        if (text.isEmpty()) {
            Toast.makeText(this, "请输入或录制文本", Toast.LENGTH_SHORT).show();
            return;
        }

        MatchResult bestMatch = matcher.findBestMatch(text);
        if (bestMatch != null) {
            StringBuilder sb = new StringBuilder();
            sb.append("输入文本: ").append(bestMatch.originalText).append("\n\n");
            sb.append("最佳匹配: ").append(bestMatch.matchedTerm).append("\n");
            sb.append("相似度: ").append(String.format("%.2f", bestMatch.similarity)).append("\n");
            sb.append("匹配成功: ").append(bestMatch.isMatch ? "是" : "否");

            resultText.setText(sb.toString());
            Log.d(TAG, "Match result: " + bestMatch);
        } else {
            resultText.setText("匹配失败");
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
