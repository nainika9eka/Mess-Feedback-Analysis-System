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

    string category;
    vector<string> detectedAspects;
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
    unordered_map<string, vector<string>> aspects = {

    {"Taste", {
        "taste","tasty","tasteless","sweet",
        "salty","spicy","bland","flavor"
    }},

    {"Cooking", {
        "raw","undercooked","overcooked",
        "cooked","stale","soggy","burnt"
    }},

    {"Hygiene", {
        "cat","cats","dog","dogs",
        "insect","insects","hair",
        "dirty","hygiene"
    }},

    {"Menu", {
        "menu","dish","variety",
        "change","removed"
    }},

    {"Service", {
        "staff","closed","late",
        "timing","timings"
    }},

    {"Quality", {
        "quality","poor","good",
        "bad","excellent"
    }}
};
vector<string> suggestionKeywords = {

    "please add",
    "bring back",
    "should be replaced",
    "change menu",
    "add to menu",
    "kindly",
    "request"
};
unordered_map<string,int> phrases = {

    {"not good",-3},
    {"not tasty",-3},

    {"too oily",-3},
    {"very oily",-3},

    {"raw chicken",-5},
    {"undercooked",-4},
    {"not cooked",-4},
    {"not cooked properly",-5},

    {"poor quality",-4},
    {"very bad",-4},
    {"horrible",-4},
    {"tasteless",-3},

    {"stale",-3},
    {"watery curd",-3},

    {"cats lick",-5},
    {"dogs roam",-4},

    {"insects",-5},
    {"hair",-5},

    {"menu different",-3},
    {"same taste",-2},

    {"excellent",3},
    {"very good",3},
    {"really tasty",3}
};
    unordered_set<string> negations = {"not", "no", "isn't", "wasn't", "aren't", "doesn't", "don't", "didn't", "never", "without", "neither", "none", "nobody", "nothing", "nowhere", "hardly", "barely", "scarcely"};
    
    unordered_set<string> intensifiers = {"very", "really", "extremely", "absolutely", "completely", "totally", "utterly"};
    unordered_set<string> diminishers = {"somewhat", "slightly", "kind", "sort", "bit", "little"};
    
    // Phrase-level sentiment detection (phrase -> score)
   

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
    unordered_map<string,int> aspectCount;
    unordered_map<string,int> likedItems;
unordered_map<string,int> dislikedItems;

vector<string> foodItems = {
    "puri",
    "dalia",
    "cornflakes",
    "boiled egg",
    "banana",
    "vada",
    "ghuguni",
    "uttapam",
    "methi puri",
    "plain dosa",
    "chole bhature",
    "masala dosa",
    "plain paratha",

    "rice",
    "dal",
    "sambar",
    "roti",
    "rajma",
    "aloo chokha",
    "seasonal khatta",
    "dahi curry",
    "saag",
    "salad",
    "tomato rice",
    "mix vegetable",
    "palak paneer",
    "ghanta",
    "aloo masala",
    "khechdi",
    "kerala chips",
    "fish masala",
    "pumpkin curry",
    "drumsticks",
    "bhindi masala",
    "chicken",
    "paneer",
    "mushroom chilli",
    "chhole",
    "bundi raita",

    "samosa",
    "jalebi",
    "watermelon",
    "tea",
    "coffee",
    "summer drink",
    "pani bhaji",
    "dahi vada",
    "veg sandwich",
    "chat",
    "noodles",
    "pasta",
    "chatpata sprouts",

    "paneer chilli",
    "veg manchurian",
    "chana masala",
    "egg masala curry",
    "sweet",
    "fruit custard",
    "raita",
    "potato fingers"
};
    while (getline(infile, line)) {
        if (line.empty() || line.find_first_not_of(" \t\n\r") == string::npos) {
            continue; // Skip empty lines
        }

        string lowerLine = toLower(line);
        vector<string> words = tokenize(line);
        int score = 0;
        int sentimentWordCount = 0;
        vector<bool> used(words.size(), false);
        // Step 1: Check for phrase-level sentiment first
        bool phraseMatched = false;
        
        
        // Step 2: Word-level analysis with negation window
        // Only do word-level analysis if no phrase was matched OR if review is very long (need additional context)
        if (!phraseMatched) {
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
        for(size_t i=0;i+1<words.size();i++)
{
      if(used[i])
        continue;
    string pairPhrase = words[i] + " " + words[i+1];

    if(phrases.count(pairPhrase))
    {
        score += phrases[pairPhrase];

        used[i] = true;
        used[i+1] = true;

        sentimentWordCount += 2;
    }
}
        // Step 3: Calculate confidence score
        double confidence = 0.0;
        if (words.size() > 0) {
            double sentimentRatio = static_cast<double>(sentimentWordCount) / words.size();
            double scoreMagnitude = min(1.0, static_cast<double>(abs(score)) / 10.0);
            confidence = min(1.0, (sentimentRatio * 2.0) + (scoreMagnitude * 0.5));
        }
        
SentimentResult result;
result.score = score;
result.confidence = confidence;
result.sentimentWordCount = sentimentWordCount;
result.category = "";
result.detectedAspects.clear();
     for(auto& aspect : aspects)
{
    bool found = false;

    for(auto& keyword : aspect.second)
    {
        if(lowerLine.find(keyword) != string::npos)
        {
            found = true;
            break;
        }
    }

    if(found)
result.detectedAspects.push_back(aspect.first);
aspectCount[aspect.first]++;
}
bool isSuggestion = false;

for(const auto& keyword : suggestionKeywords)
{
    if(lowerLine.find(keyword) != string::npos)
    {
        isSuggestion = true;
        break;
    }
}

if(isSuggestion)
{
    result.category = "Suggestion";
}
else if(score > 0)
{
    result.category = "Positive";
}
else if(score < 0)
{
    result.category = "Negative";
}
else
{
    result.category = "Neutral";
}
for(const auto& item : foodItems)
{
    if(lowerLine.find(item) != string::npos)
    {
        if(score > 0)
        {
            likedItems[item]++;
        }
        else if(score < 0)
        {
            dislikedItems[item]++;
        }
    }
}
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
    cout << "\n===== Most Appreciated Items =====\n";

vector<pair<string,int>> goodList(
    likedItems.begin(),
    likedItems.end()
);

sort(goodList.begin(), goodList.end(),
[](auto &a, auto &b)
{
    return a.second > b.second;
});

for(size_t i=0; i<min((size_t)5, goodList.size()); i++)
{
    cout << goodList[i].first
         << " : "
         << goodList[i].second
         << " positive mentions\n";
}

cout << "\n===== Most Complained About =====\n";

vector<pair<string,int>> badList(
    dislikedItems.begin(),
    dislikedItems.end()
);

sort(badList.begin(), badList.end(),
[](auto &a, auto &b)
{
    return a.second > b.second;
});

for(size_t i=0; i<min((size_t)5, badList.size()); i++)
{
    cout << badList[i].first
         << " : "
         << badList[i].second
         << " negative mentions\n";
}
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

    cout << "\n===== Aspect Summary =====\n";

for(const auto& item : aspectCount)
{
    cout << item.first
         << " : "
         << item.second
         << " mentions"
         << endl;
}

                 return 0;
}
