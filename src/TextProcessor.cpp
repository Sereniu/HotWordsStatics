#include "TextProcessor.h"
#include "spdlog/spdlog.h"
#include <chrono>

TextProcessor::TextProcessor(const std::string &dict_path,bool enable_pos_filter):enable_pos_filter_(enable_pos_filter)
{
try {

        spdlog::info("=== TextProcessor Initializing ===");
        spdlog::info("Dictionary path: {}", dict_path);
        spdlog::info("POS filter: {}", enable_pos_filter_ ? "Enabled" : "Disabled");
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
            spdlog::info("Valid POS tags loaded: {} types", valid_pos_.size());
        }
        
        // 加载停用词到内存哈希表
        loadStopWords(dict_path+"stop_words.utf8");
        loadSensitiveWords(dict_path+"sensitive_words.utf8");
        
        spdlog::info(">>> TextProcessor Initialized Successfully <<<");
        spdlog::info("Stop words count: {}", stop_words_.size());
        spdlog::info("Sensitive words count: {}", sensitive_words_.size());
    
    } catch (const std::exception& e) {
        spdlog::critical("TextProcessor initialization failed: {}", e.what());
        throw;
    }
}

TextProcessor::~TextProcessor()=default;

std::vector<std::string> TextProcessor::process(const std::string &text)
{   
    auto start_time = std::chrono::high_resolution_clock::now();

    if(text.empty()){
        spdlog::debug("Empty text input, skipping processing");
        return {};
    }

    size_t original_length = text.length();
    spdlog::trace("Processing text: length={}, preview='{}'", 
                  original_length, 
                  text.substr(0, std::min(size_t(50), original_length)));

    //分词
    std::vector<std::string> raw_words;
    auto segment_start = std::chrono::high_resolution_clock::now();

    try {
        jieba_->Cut(text, raw_words, true);  // true = 使用 HMM
    } catch (const std::exception& e) {
        //分词失败
        spdlog::error("Segmentation failed: {} | Text: '{}'", 
                     e.what(), text.substr(0, 100));
        return {};
    }

    auto segment_end = std::chrono::high_resolution_clock::now();
    auto segment_ms = std::chrono::duration<double, std::milli>(segment_end - segment_start).count();

    spdlog::debug("Segmentation result: {} words from {} chars", 
                  raw_words.size(), original_length);

    //过滤和清洗
    std::vector<std::string> result;
    result.reserve(raw_words.size());  // 预分配空间
    
    for (const auto& word : raw_words) {
        if (isValidWord(word)) {
            result.push_back(word);
        }
    }
   
    //记录耗时
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    auto filter_ms = total_ms - segment_ms;
    
    auto perf_logger = spdlog::get("perf");
    if (perf_logger) {
        perf_logger->info("{},preprocess_total_ms,{:.3f}", std::time(nullptr), total_ms);
        perf_logger->info("{},segment_ms,{:.3f}", std::time(nullptr), segment_ms);
        perf_logger->info("{},filter_ms,{:.3f}", std::time(nullptr), filter_ms);
        perf_logger->info("{},text_length,{}", std::time(nullptr), original_length);
        perf_logger->info("{},words_segmented,{}", std::time(nullptr), raw_words.size());
        perf_logger->info("{},words_valid,{}", std::time(nullptr), result.size());
    }
    
    spdlog::debug("Processing time: segment={:.2f}ms, filter={:.2f}ms, total={:.2f}ms",
                  segment_ms, filter_ms, total_ms);
    
    // 耗时过长
    if (total_ms > 100.0) {
        spdlog::warn("Slow text processing: {:.2f}ms (threshold: 100ms) for {} chars", 
                     total_ms, original_length);
    }
    return result;
}

std::vector<std::string> TextProcessor::processWithPOS(const std::string &text)
{
    //计时开始
    auto start_time = std::chrono::high_resolution_clock::now();
    
    //\空文本检测
    if (text.empty()) {
        spdlog::debug("Empty text input (POS mode), skipping processing");
        return {};
    }

    spdlog::debug("Processing text with POS tagging: length={}", text.length());

    // 标注词性
    std::vector<std::pair<std::string, std::string>> tagged_words;
    try {
        jieba_->Tag(text, tagged_words);
    } catch (const std::exception& e) {
        // 【异常处理】词性标注失败
        spdlog::error("POS tagging failed: {} | Text: '{}'", 
                     e.what(), text.substr(0, 100));
        return {};
    }

    // 过滤
    std::vector<std::string> result;
    result.reserve(tagged_words.size());
    
    for (const auto& pair : tagged_words) {
        const std::string& word = pair.first;
        const std::string& pos = pair.second;

        if(isValidWord(word) && isValidPOS(pos)){
            result.push_back(word);
        }
    }

    // 记录耗时
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    
    auto perf_logger = spdlog::get("perf");
    if (perf_logger) {
        perf_logger->info("{},preprocess_pos_ms,{:.3f}", std::time(nullptr), duration_ms);
    }
    
    return result;
}

void TextProcessor::loadStopWords(const std::string &file_path)
{   
    spdlog::info("Loading stop words from: {}", file_path);
    //以二进制打开，避免编码问题
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        spdlog::warn("Cannot open stop words file: {}", file_path);
        spdlog::warn("Stop words filtering will be disabled");
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
    spdlog::info("Stop words loaded successfully: {} words", stop_words_.size());
}

bool TextProcessor::isStopWord(const std::string &word) const
{
    return stop_words_.find(word)!=stop_words_.end();
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
    spdlog::debug("Valid POS initialized: {} types", valid_pos_.size());
}

void TextProcessor::loadSensitiveWords(const std::string &file_path)
{
    //以二进制打开，避免编码问题
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        spdlog::info("Sensitive words file not found: {} (optional feature)", file_path);
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
    spdlog::info("Sensitive words loaded successfully: {} words", sensitive_words_.size());
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