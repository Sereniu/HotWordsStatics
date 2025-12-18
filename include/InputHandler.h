#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "Common.h"
#include <fstream>
#include <string>
#include <iostream>
using namespace std;


class InputHandler {
private:
    std::string input_file_;//输入的文件
    std::ifstream file_stream_;//文件读取
    unsigned int ts=0;
    
public:
    InputHandler(const std::string& input_file);
    ~InputHandler();
    
    //打开文件
    bool open();
    
    //关闭文件
    void close();

    /**
     * 读取函数
     * @return 是否成功读取并解析一行
     */
    bool readLine(unsigned int& timestamp, std::string& text, 
                  bool& is_query, int& k);

    //是否到达文件末尾
    bool eof() const;
    
private:
    unsigned int parseTimestamp(const std::string& line);
    string extractText(const string& line);
    int parseQueryCommand(const std::string& line);
};

#endif 