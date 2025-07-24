#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <sstream>

using namespace std;

struct TrieNode {
    map<string, TrieNode*> children;
    int count = 0;
};

class Trie {
private:
    TrieNode* root;
    int n_gram_size;

public:
    Trie(int n) : n_gram_size(n) {
        root = new TrieNode();
    }

    ~Trie() {
        clear(root);
    }

    void clear(TrieNode* node) {
        for (auto& pair : node->children) {
            clear(pair.second);
        }
        delete node;
    }

    void insert(const vector<string>& tokens) {
        if (tokens.size() < n_gram_size) return;
        for (size_t i = 0; i <= tokens.size() - n_gram_size; ++i) {
            TrieNode* current = root;
            for (int j = 0; j < n_gram_size; ++j) {
                string word = tokens[i + j];
                if (current->children.find(word) == current->children.end()) {
                    current->children[word] = new TrieNode();
                }
                current = current->children[word];
            }
            current->count++;
        }
    }

    map<vector<string>, int> get_ngram_frequencies() {
        map<vector<string>, int> freq;
        vector<string> current_ngram;
        collect_ngrams(root, current_ngram, freq);
        return freq;
    }

private:
    void collect_ngrams(TrieNode* node, vector<string>& current_ngram, map<vector<string>, int>& freq) {
        if (current_ngram.size() == n_gram_size && node->count > 0) {
            freq[current_ngram] = node->count;
        }
        for (const auto& pair : node->children) {
            current_ngram.push_back(pair.first);
            collect_ngrams(pair.second, current_ngram, freq);
            current_ngram.pop_back();
        }
    }
};

vector<string> tokenize(const string& text) {
    vector<string> tokens;
    stringstream ss(text);
    string word;
    while (ss >> word) {
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        if (!word.empty()) {
            tokens.push_back(word);
        }
    }
    return tokens;
}

vector<vector<string>> get_all_ngrams(const map<vector<string>, int>& freq) {
    vector<vector<string>> all_ngrams;
    for (const auto& pair : freq) {
        all_ngrams.push_back(pair.first);
    }
    return all_ngrams;
}

vector<double> create_vector(const map<vector<string>, int>& freq, const vector<vector<string>>& all_ngrams) {
    vector<double> vec(all_ngrams.size(), 0.0);
    for (size_t i = 0; i < all_ngrams.size(); ++i) {
        if (freq.find(all_ngrams[i]) != freq.end()) {
            vec[i] = freq.at(all_ngrams[i]);
        }
    }
    return vec;
}

double cosine_similarity(const vector<double>& vec1, const vector<double>& vec2) {
    double dot_product = 0.0, norm1 = 0.0, norm2 = 0.0;
    for (size_t i = 0; i < vec1.size(); ++i) {
        dot_product += vec1[i] * vec2[i];
        norm1 += vec1[i] * vec1[i];
        norm2 += vec2[i] * vec2[i];
    }
    norm1 = sqrt(norm1);
    norm2 = sqrt(norm2);
    if (norm1 == 0 || norm2 == 0) return 0.0;
    return dot_product / (norm1 * norm2);
}

string read_file(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return content;
}

double compare_documents(const string& file1, const string& file2, int n_gram_size = 3) {
    string text1 = read_file(file1);
    string text2 = read_file(file2);
    if (text1.empty() || text2.empty()) {
        return 0.0;
    }

    vector<string> tokens1 = tokenize(text1);
    vector<string> tokens2 = tokenize(text2);

    Trie trie1(n_gram_size);
    Trie trie2(n_gram_size);
    trie1.insert(tokens1);
    trie2.insert(tokens2);

    map<vector<string>, int> freq1 = trie1.get_ngram_frequencies();
    map<vector<string>, int> freq2 = trie2.get_ngram_frequencies();

    vector<vector<string>> all_ngrams = get_all_ngrams(freq1);
    for (const auto& ngram : get_all_ngrams(freq2)) {
        if (find(all_ngrams.begin(), all_ngrams.end(), ngram) == all_ngrams.end()) {
            all_ngrams.push_back(ngram);
        }
    }

    vector<double> vec1 = create_vector(freq1, all_ngrams);
    vector<double> vec2 = create_vector(freq2, all_ngrams);

    return cosine_similarity(vec1, vec2);
}

int main() {
    string file1, file2;
    cout << "Enter first file path: ";
    getline(cin, file1);
    cout << "Enter second file path: ";
    getline(cin, file2);

    double similarity = compare_documents(file1, file2);
    cout << "Similarity score: " << similarity * 100 << "%" << endl;

    return 0;
}
