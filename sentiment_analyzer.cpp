
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <climits>
#include <cmath>
#include <iomanip>
using namespace std;

string toLower(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

vector<string> tokenize(const string& sentence) {
    vector<string> tokens;
    stringstream ss(sentence);
    string word;
    while (ss >> word) {
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        tokens.push_back(toLower(word));
    }
    return tokens;
}

struct SentimentResult {
    int score;
    double confidence;
    int sentimentWordCount;
};

int main() {
    // Weighted sentiment words with scores
    unordered_map<string, int> positiveWords = {
        // Strong positive (+3)
        {"amazing", 3}, {"awesome", 3}, {"excellent", 3}, {"perfect", 3}, 
        {"delicious", 3}, {"wonderful", 3}, {"outstanding", 3}, {"superb", 3},
        // Medium positive (+2)
        {"good", 2}, {"great", 2}, {"nice", 2}, {"tasty", 2}, {"well", 2}, 
        {"fresh", 2}, {"clean", 2}, {"friendly", 2}, {"best", 2}, {"better", 2},
        {"peacefully", 2}, {"love", 2}, {"fine", 2}, {"decent", 2},
        // Food-specific positive
        {"flavorful", 2}, {"spicy", 2}, {"savory", 2}, {"crispy", 2}, 
        {"tender", 2}, {"juicy", 2}, {"aromatic", 2}, {"seasoned", 2}, 
        {"authentic", 2}, {"homemade", 2}, {"hot", 2}, {"warm", 2}
    };
    
    unordered_map<string, int> negativeWords = {
        // Strong negative (-3)
        {"worst", -3}, {"unbearable", -3}, {"inedible", -3}, {"hazardous", -3}, 
        {"toxic", -3}, {"revolting", -3}, {"disgusting", -3},
        // Medium negative (-2)
        {"horrible", -2}, {"awful", -2}, {"terrible", -2}, {"rude", -2}, 
        {"dirty", -2}, {"cold", -2}, {"stale", -2}, {"hate", -2}, {"problem", -2},
        // Weak negative (-1)
        {"bad", -1}, {"slow", -1}, {"late", -1}, {"bland", -1}, {"tasteless", -1}, 
        {"dull", -1}, {"oily", -1}, {"rubbery", -1}, {"uncooked", -1},
        // Food-specific negative
        {"undercooked", -2}, {"overcooked", -2}, {"raw", -2}, {"burnt", -2}, 
        {"soggy", -2}, {"hard", -1}, {"chewy", -1}, {"dry", -1}, {"watery", -1}, 
        {"greasy", -2}, {"moldy", -3}, {"rotten", -3}, {"salty", -1}
    };
    
    unordered_set<string> negations = {"not", "no", "isn't", "wasn't", "aren't", "doesn't", "don't", "didn't", "never", "without", "neither", "none", "nobody", "nothing", "nowhere", "hardly", "barely", "scarcely"};
    
    unordered_set<string> intensifiers = {"very", "really", "extremely", "absolutely", "completely", "totally", "utterly"};
    unordered_set<string> diminishers = {"somewhat", "slightly", "kind", "sort", "bit", "little"};
    
    // Phrase-level sentiment detection (phrase -> score)
    unordered_map<string, int> phrases = {
        {"not good", -2}, {"not tasty", -2}, {"not cooked", -2}, {"no quality", -2}, 
        {"never fresh", -2}, {"very good", 3}, {"really tasty", 3}, {"well cooked", 2}, 
        {"perfectly seasoned", 3}, {"fresh and delicious", 3}, {"love the food", 3},
        {"very bad", -3}, {"horrible taste", -3}, {"awful quality", -3}, 
        {"undercooked chicken", -3}, {"stale food", -3}, {"oily food", -2},
        {"not at all good", -3}, {"not nice", -2}, {"not all tasty", -2}
    };

    ifstream infile("food_reviews.txt");
    if (!infile) {
        cout << "Error: food_reviews.txt not found." << endl;
        return 1;
    }

    string line;
    vector<pair<string, SentimentResult>> feedbacks;
    int pos = 0, neg = 0, neu = 0, total = 0, totalScore = 0;
    double maxScore = INT_MIN, minScore = INT_MAX;
    string best, worst;

    while (getline(infile, line)) {
        if (line.empty() || line.find_first_not_of(" \t\n\r") == string::npos) {
            continue; // Skip empty lines
        }

        string lowerLine = toLower(line);
        vector<string> words = tokenize(line);
        int score = 0;
        int sentimentWordCount = 0;
        
        // Step 1: Check for phrase-level sentiment first
        bool phraseMatched = false;
        for (const auto& phrase : phrases) {
            if (lowerLine.find(phrase.first) != string::npos) {
                score += phrase.second;
                sentimentWordCount += 2; // Count as 2 words for confidence
                phraseMatched = true;
            }
        }
        
        // Step 2: Word-level analysis with negation window
        if (!phraseMatched || words.size() > 5) { // Still analyze words if long review
            vector<bool> negationWindow(words.size(), false);
            
            // Mark negation window (next 3 words after negation)
            for (size_t i = 0; i < words.size(); i++) {
                if (negations.count(words[i])) {
                    for (size_t j = i + 1; j < min(i + 4, words.size()); j++) {
                        negationWindow[j] = true;
                    }
                }
            }
            
            // Calculate score with modifiers
            for (size_t i = 0; i < words.size(); i++) {
                const string& word = words[i];
                double modifier = 1.0;
                
                // Check for intensifier/diminisher before current word
                if (i > 0) {
                    if (intensifiers.count(words[i-1])) {
                        modifier = 1.5;
                    } else if (diminishers.count(words[i-1])) {
                        modifier = 0.5;
                    }
                }
                
                if (positiveWords.count(word)) {
                    int wordScore = positiveWords[word];
                    if (negationWindow[i]) {
                        wordScore = -wordScore; // Invert sentiment
                    }
                    score += static_cast<int>(wordScore * modifier);
                    sentimentWordCount++;
                } else if (negativeWords.count(word)) {
                    int wordScore = negativeWords[word];
                    if (negationWindow[i]) {
                        wordScore = -wordScore; // Invert sentiment
                    }
                    score += static_cast<int>(wordScore * modifier);
                    sentimentWordCount++;
                }
            }
        }
        
        // Step 3: Calculate confidence score
        double confidence = 0.0;
        if (words.size() > 0) {
            double sentimentRatio = static_cast<double>(sentimentWordCount) / words.size();
            double scoreMagnitude = min(1.0, static_cast<double>(abs(score)) / 10.0);
            confidence = min(1.0, (sentimentRatio * 2.0) + (scoreMagnitude * 0.5));
        }
        
        SentimentResult result = {score, confidence, sentimentWordCount};
        
        totalScore += score;
        if (score > 0) pos++;
        else if (score < 0) neg++;
        else neu++;

        // Improved selection: consider score, confidence, and length
        double weightedScore = score * confidence;
        if (weightedScore > maxScore && line.length() > 10) { 
            maxScore = weightedScore; 
            best = line; 
        }
        if (weightedScore < minScore && line.length() > 10) { 
            minScore = weightedScore; 
            worst = line; 
        }

        feedbacks.push_back({line, result});
        total++;
    }

    double avg = total > 0 ? (double)totalScore / total : 0;
    string mood;
    if (avg > 0.5) mood = "😄";
    else if (avg > 0.1) mood = "🙂";
    else if (avg > -0.1) mood = "😐";
    else if (avg > -0.5) mood = "😞";
    else mood = "😡";

    cout << "📊 Total Feedbacks: " << total << endl;
    cout << "✅ Positive: " << pos << ", ❌ Negative: " << neg << ", ⚪ Neutral: " << neu << endl;
    cout << "📈 Average Score: " << avg << " " << mood << endl << endl;

    cout << "🌟 Top 3 Positive Feedbacks:\n";
    sort(feedbacks.begin(), feedbacks.end(), [](auto& a, auto& b) { 
        return a.second.score * a.second.confidence > b.second.score * b.second.confidence; 
    });
    for (int i = 0; i < min(3, (int)feedbacks.size()); ++i)
        if (feedbacks[i].second.score > 0)
            cout << i+1 << ". " << feedbacks[i].first << " (Score: " << feedbacks[i].second.score 
                 << ", Confidence: " << fixed << setprecision(2) << feedbacks[i].second.confidence << ")" << endl;

    cout << "\n💢 Top 3 Negative Feedbacks:\n";
    sort(feedbacks.begin(), feedbacks.end(), [](auto& a, auto& b) { 
        return a.second.score * a.second.confidence < b.second.score * b.second.confidence; 
    });
    for (int i = 0; i < min(3, (int)feedbacks.size()); ++i)
        if (feedbacks[i].second.score < 0)
            cout << i+1 << ". " << feedbacks[i].first << " (Score: " << feedbacks[i].second.score 
                 << ", Confidence: " << fixed << setprecision(2) << feedbacks[i].second.confidence << ")" << endl;

    return 0;
}
