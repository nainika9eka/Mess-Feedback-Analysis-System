
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <climits>
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

int main() {
    unordered_set<string> positiveWords = {"good", "great", "nice", "amazing", "awesome", "friendly", "clean", "tasty", "well", "peacefully"};
    unordered_set<string> negativeWords = {"bad", "slow", "late", "dirty", "cold", "hate", "rude", "problem", "worst", "horrible", "bland", "awful", "oily", "rubbery", "uncooked"};
    unordered_set<string> negations = {"not", "no", "isn't", "wasn't", "aren't", "doesn't", "don't", "didn't", "never", "without"};

    ifstream infile("food_reviews.txt");
    if (!infile) {
        cout << "Error: food_reviews.txt not found." << endl;
        return 1;
    }

    string line;
    vector<pair<string, int>> feedbacks;
    int pos = 0, neg = 0, neu = 0, total = 0, totalScore = 0;
    int maxScore = INT_MIN, minScore = INT_MAX;
    string best, worst;

    while (getline(infile, line)) {
        vector<string> words = tokenize(line);
        int score = 0;
        bool negate = false;

        for (const string& word : words) {
            if (negations.count(word)) {
                negate = true;
                continue;
            }
            if (positiveWords.count(word)) {
                score += (negate ? -1 : 1);
                negate = false;
            } else if (negativeWords.count(word)) {
                score += (negate ? 1 : -1);
                negate = false;
            } else {
                negate = false;
            }
        }

        totalScore += score;
        if (score > 0) pos++;
        else if (score < 0) neg++;
        else neu++;

        if (score > maxScore) { maxScore = score; best = line; }
        if (score < minScore) { minScore = score; worst = line; }

        feedbacks.push_back({line, score});
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

    cout << "🌟 Top 3 Positive Feedbacks:
";
    sort(feedbacks.begin(), feedbacks.end(), [](auto& a, auto& b) { return a.second > b.second; });
    for (int i = 0; i < min(3, (int)feedbacks.size()); ++i)
        if (feedbacks[i].second > 0)
            cout << i+1 << ". " << feedbacks[i].first << " (" << feedbacks[i].second << ")" << endl;

    cout << "\n💢 Top 3 Negative Feedbacks:
";
    sort(feedbacks.begin(), feedbacks.end(), [](auto& a, auto& b) { return a.second < b.second; });
    for (int i = 0; i < min(3, (int)feedbacks.size()); ++i)
        if (feedbacks[i].second < 0)
            cout << i+1 << ". " << feedbacks[i].first << " (" << feedbacks[i].second << ")" << endl;

    return 0;
}
