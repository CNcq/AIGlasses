package com.aiglasses;

public class EmbedMatchResult {
    public String inputText;
    public String matchedId;
    public String matchedText;
    public float similarity;
    public int rank;
    public boolean isMatch;

    public EmbedMatchResult(String inputText, String matchedId, String matchedText, 
                           float similarity, int rank, boolean isMatch) {
        this.inputText = inputText;
        this.matchedId = matchedId;
        this.matchedText = matchedText;
        this.similarity = similarity;
        this.rank = rank;
        this.isMatch = isMatch;
    }

    @Override
    public String toString() {
        return "EmbedMatchResult{" +
                "inputText='" + inputText + '\'' +
                ", matchedId='" + matchedId + '\'' +
                ", matchedText='" + matchedText + '\'' +
                ", similarity=" + similarity +
                ", rank=" + rank +
                ", isMatch=" + isMatch +
                '}';
    }
}
