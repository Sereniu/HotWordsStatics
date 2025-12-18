#include "TextProcessor.h"

TextProcessor::TextProcessor(const std::string &dict_path,bool enable_pos_filter):enable_pos_filter_(enable_pos_filter)
{
try {
        // 初始化 jieba 分词器（加载 5 个核心词典）
        jieba_ = std::make_unique<cppjieba::Jieba>(
            dict_path + "jieba.dict.utf8",       // 主词典
            dict_path + "hmm_model.utf8",        // HMM 模型
            dict_path + "user.dict.utf8",        // 用户自定义词典
            dict_path + "idf.utf8",              // IDF 权重文件（用于关键词提取）
            dict_path + "stop_words.utf8"        // 停用词表
        );

        if(enable_pos_filter_){
            initValidPOS();
        }
        
        // 加载停用词到内存哈希表
        loadStopWords(dict_path+"stop_words.utf8");
        loadSensitiveWords(dict_path+"sensitive_words.utf8");
        
        std::cout << "[TextProcessor] [信息] TextProcessor 初始化成功" << std::endl;
        std::cout << "[TextProcessor] [信息] 停用词数量: " << stop_words_.size() << std::endl;
        std::cout << "[TextProcessor] [信息] 敏感词数量: " << sensitive_words_.size() << std::endl;
        std::cout << "[TextProcessor] [信息] 词性过滤: " << (enable_pos_filter_ ? "启用" : "禁用") << std::endl;
    
    } catch (const std::exception& e) {
        std::cerr << "[TextProcessor] [错误] TextProcessor 初始化失败: " << e.what() << std::endl;
        throw;
    }
}

TextProcessor::~TextProcessor()=default;

std::vector<std::string> TextProcessor::process(const std::string &text)
{
    if(text.empty()){
        return {};
    }

    //分词
    std::vector<std::string> raw_words;
    jieba_->Cut(text, raw_words, true);  // true = 使用 HMM

    //过滤和清洗
    std::vector<std::string> result;
    result.reserve(raw_words.size());  // 预分配空间
    
    for (const auto& word : raw_words) {
        if (isValidWord(word)) {
            result.push_back(word);
        }
    }
    
    return result;
}

std::vector<std::string> TextProcessor::processWithPOS(const std::string &text)
{
    if(text.empty()){
        return {};
    }

    //标注词性
    std::vector<std::pair<std::string, std::string>> tagged_words;
    jieba_->Tag(text, tagged_words);

    //过滤
     std::vector<std::string> result;
    result.reserve(tagged_words.size());
    
    for (const auto& pair : tagged_words) {
        const std::string& word = pair.first;
        const std::string& pos = pair.second;

        if(isValidWord(word) && isValidPOS(pos)){
            result.push_back(word);
        }
    }
    return result;
}

void TextProcessor::loadStopWords(const std::string &file_path)
{   
    //以二进制打开，避免编码问题
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[TextProcessor] [警告] 无法打开停用词文件: " << file_path << std::endl;
        std::cerr << "[TextProcessor] [警告] 将不进行停用词过滤" << std::endl;
        return;
    }
    
    //读取所有的停用词
    std::string line;
    while (std::getline(file, line)) {
        // 移除 Windows 换行符 \r
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // 去除首尾空格
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);
        
        if (!line.empty()) {
            stop_words_.insert(line);
        }
    }
    
    file.close();
}

bool TextProcessor::isStopWord(const std::string &word) const
{
    return stop_words_.find(word)!=stop_words_.end();
}

bool TextProcessor::isValidWord(const std::string &word) const
{   
    // 1. 过滤空字符串
    if (word.empty()) {
        return false;
    }
    
    // 2. 过滤纯空白字符（空格、制表符、换行符等）
    if (word.find_first_not_of(" \t\n\r") == std::string::npos) {
        return false;
    }
    
    // 3. 过滤单个空格
    if (word == " ") {
        return false;
    }
    
    // 4. 停用词过滤
    if (isStopWord(word)) {
        return false;
    }

    // 5. 敏感词过滤
    if (isSensitiveWord(word)) {
        return false;
    }
    
    return true;
}

bool TextProcessor::isSensitiveWord(const std::string &word) const
{
    return sensitive_words_.find(word)!=sensitive_words_.end();
}

bool TextProcessor::isValidPOS(const std::string &pos) const
{
    return valid_pos_.find(pos) != valid_pos_.end();
}

void TextProcessor::initValidPOS()
{
    // 保留的词性（实词为主）
    valid_pos_ = {
        // 名词类
        "n",   // 普通名词（如：学生、电脑）
        "nr",  // 人名（如：张三、李四）
        "ns",  // 地名（如：北京、上海）
        "nt",  // 机构名（如：清华大学）
        "nz",  // 其他专名
        
        // 动词类
        "v",   // 动词（如：学习、研究）
        "vn",  // 名动词（如：调查、研究）
        
        // 形容词
        "a",   // 形容词（如：美丽、快速）
        "ad",  // 副形词（如：很、非常）
        "an",  // 名形词（如：经济、政治）
        
        // 其他有意义词性
        "i",   // 成语（如：一帆风顺）
        "j",   // 简称（如：北大、清华）
        "l",   // 习用语（如：按照、根据）
        "eng", // 英文词
        "x"    // 非语素字（保留，可能是专有名词）

    };
}

void TextProcessor::loadSensitiveWords(const std::string &file_path)
{
    //以二进制打开，避免编码问题
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[TextProcessor] [警告] 无法打开停用词文件: " << file_path<<" 不需要敏感词过滤可忽略该错误" << std::endl;
        return;
    }
    
    //读取所有的停用词
    std::string line;
    while (std::getline(file, line)) {
        // 移除 Windows 换行符 \r
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // 去除首尾空格
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);
        
        if (!line.empty()) {
            sensitive_words_.insert(line);
        }
    }
    
    file.close();

}
