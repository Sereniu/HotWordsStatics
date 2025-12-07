#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include<string>
#include<vector>
#include<unordered_set>
#include "cppjieba/Jieba.hpp"

using namespace std;

/**
 * PreProcessor类
 * 使用cut简单分词后返回vector<string>
 */
class PreProcessor{
    private:
    cppjieba::Jieba jieba;

    //停用词集合
    unordered_set<string> stopWords;

    //辅助函数
    string cleanWord(const string& word);

    public:
    /**
     * 构造函数
     * dictPath是dict文件夹的位置，便于自动拼接5个词典文件
     */
    PreProcessor(const string& dictPath);

    /**
     * 加载停用词
     */
    bool loadStopWords(const string& filepath);

    /**
     * 核心处理方法-Cut
     * 返回处理后的干净的词列表
     */
    vector<string> process(const string& text);

    //文件处理
     vector<string> processFile(const string& filepath);

    //添加用户词
    void addUserWord(const string& word);
    
    //返回停用词数量
    size_t getStopWordsCount() const;
};

#endif
