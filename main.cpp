#include <algorithm>
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};


class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);

        for(const string& word: words)
        {
            map<int,double>& docIdTf = document_idx_[word];
            docIdTf[document_id] += 1./words.size();
        }

        document_count_ += 1;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query = ParseQuery(raw_query);

        vector<Document> matched_documents = FindAllDocuments(query);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    struct Query {
        set<string> query_words;
        set<string> minus_words;
    };

    int document_count_ = 0;
    map<string, map<int,double>> document_idx_; // word, map of {doc ID, word TF}

    set<string> stop_words_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if(word[0] == '-')
                query.minus_words.insert(word.substr(1));
            else
                query.query_words.insert(word);
        }
        return query;
    }

    vector<Document> FindAllDocuments(const Query& query) const {
        map<int,double> mapMatchedDocs;// id,relevance (TF*IDF)
        for(const string& query_word: query.query_words)
        {
            map<string, map<int,double>>::const_iterator cIt = document_idx_.find(query_word);
            if(cIt != document_idx_.cend())
            {
                const map<int,double>& docIdTf = cIt->second;

                size_t sz = docIdTf.size();
                if(sz == 0)
                    continue;

                const double idf = log((double)document_count_/sz);
                for(const auto& [id,tf] : docIdTf)
                {
                    mapMatchedDocs[id] += tf*idf;
                }
            }
        }
        for(const string& minus_word: query.minus_words)
        {
            map<string, map<int,double>>::const_iterator cIt = document_idx_.find(minus_word);
            if(cIt != document_idx_.cend())
            {
                const map<int,double>& docIdTf = cIt->second;
                for(const auto& [id,tf] : docIdTf)
                    mapMatchedDocs.erase(id);
            }
        }

        vector<Document> matched_documents;
        for(const auto& [id,relevance] : mapMatchedDocs)
            matched_documents.push_back({id, relevance});

        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}