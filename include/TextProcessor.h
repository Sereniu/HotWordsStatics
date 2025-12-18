#ifndef TEXTPROCESSOR_H
#define TEXTPROCESSOR_H

#include "Common.h"
#include "../cppjieba/Jieba.hpp"
#include <unordered_set>
#include <memory>
#include <string>
#include <vector>

namespace cppjieba {
    class Jieba;
}

/**
 * 中文分词 + 停用词过滤 + 词语清洗
 */
class TextProcessor {
private:
    std::unique_ptr<cppjieba::Jieba> jieba_;
    std::unordered_set<std::string> stop_words_;//基于哈希的停用词表，实现O(1)查找
    std::unordered_set<std::string> sensitive_words_;  // 敏感词表
    std::unordered_set<std::string> valid_pos_;        // 有效词性集合
    bool enable_pos_filter_;                            // 是否启用词性过滤

public:

    /**
     * 构造函数
     * @param dict_path 词典目录路径（默认 "dict/"）
     */
    TextProcessor(const std::string& dict_path = "../dict/", bool enable_pos_filter=false);

    ~TextProcessor();
    
    /**
     * 处理文本：分词 + 过滤 + 清洗
     * @param text 原始文本
     * @return 处理后的词语列表
     */
    std::vector<std::string> process(const std::string& text);
    
    /**
     * 带词性的处理函数
     * 处理文本：分词 + 过滤 + 清洗
     * @param text 原始文本
     * @return 处理后的词语列表
     */
    std::vector<std::string> processWithPOS(const std::string& text);

private:
   /**
    * 加载停用词表
    */
    void loadStopWords(const std::string& file_path);

    //加载有效词性
    void initValidPOS();

    //加载敏感词
    void loadSensitiveWords(const std::string& file_path);

    //判断是否为停用词
    bool isStopWord(const std::string& word) const;

    //判断词语是否有效（标点过滤符号，无意义符号等）
    bool isValidWord(const std::string& word) const;

    //判断是否为敏感词
    bool isSensitiveWord(const std::string& word) const;

    //判断词性是否有效
    bool isValidPOS(const std::string& pos) const;
};

#endif
