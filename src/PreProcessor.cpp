#include "PreProcessor.h"
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

PreProcessor::PreProcessor(const string& dictPath)
    :jieba(
        dictPath + "/jieba.dict.utf8",      // 主词典
        dictPath + "/hmm_model.utf8",       // HMM模型
        dictPath + "/user.dict.utf8",       // 用户词典
        dictPath + "/idf.utf8",             // IDF文件
        dictPath + "/stop_words.utf8"       // 停用词"
    ){
    cout<<"[PreProcessor] Jieba initialized with dict:"<<dictPath<<endl;
}

bool PreProcessor::loadStopWords(const string& filepath){
    //以二进制打开，避免编码问题
    ifstream file(filepath,ios::binary);
    if(!file.is_open()){
        cerr<<"[ERROR] cannot open stopwords file: "<<filepath<<endl;
        return false;
    }

    string word;
    int count=0;
    while(getline(file,word)){
        //去除windows的\r
        if(!word.empty() && word.back()=='\r'){
            word.pop_back();
        }
        
        //去除其他空白字符
        word.erase(remove_if(word.begin(),word.end(),::isspace),word.end());

        if(!word.empty()){
            stopWords.insert(word);
            count++;
        }
    }

    file.close();
    cout<< "[PreProcessor] Loaded " << count << " stopwords" << endl;
    return true;
}

string PreProcessor::cleanWord(const string& word){
    string cleaned;
    for(char c:word){
        if(!isspace(c)){
            cleaned+=c;
        }
    }
    return cleaned;
}

vector<string> PreProcessor::process(const string& text){
    vector<string> result;
    if(text.empty()){
        return result;
    }

    //使用jieba分词
    vector<string> words;
    jieba.Cut(text,words,true);

    //清洗和过滤
    for(const string& word:words){
        string cleaned=cleanWord(word);

        if(!cleaned.empty() && stopWords.find(cleaned) == stopWords.end()){
            result.push_back(cleaned);
        }
    }
    
    return result;
}

vector<string> PreProcessor::processFile(const string& filepath){
    vector<string> allWords;
    
    ifstream file(filepath,ios::binary);
    if(!file.is_open()){
        cerr << "[ERROR] Cannot open file: " << filepath << endl;
        return allWords;
    }

    string line;
    int lineCount=0;
    while(getline(file,line)){
        //去除‘\r'
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (!line.empty()) {
            // 处理每一行
            vector<string> words = process(line);
            allWords.insert(allWords.end(), words.begin(), words.end());
            lineCount++;
        }
    }

    file.close();
    cout << "[PreProcessor] Processed " << lineCount << " lines, "
         << allWords.size() << " words extracted" << endl;
    
    return allWords;
}

void PreProcessor::addUserWord(const string& word) {
    jieba.InsertUserWord(word);
    cout << "[PreProcessor] User word added: " << word << endl;
}

size_t PreProcessor::getStopWordsCount() const {
    return stopWords.size();
}
