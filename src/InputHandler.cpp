#include "InputHandler.h"
#include <iostream>
#include <sstream>
#include <string>
#include <regex>
#include <iostream>
#include "spdlog/spdlog.h"
using namespace std;

InputHandler::InputHandler(const std::string &input_file):input_file_(input_file)
{
    spdlog::info("InputHandler initialized: input_file={}", input_file_);
}

InputHandler::~InputHandler()
{
    close();
}

bool InputHandler::open()
{   
    file_stream_.open(input_file_);
    
    if(!file_stream_.is_open()){
        spdlog::critical("Failed to open input file: {}", input_file_);
        return false;
    }
    spdlog::info("Input file opened successfully: {}", input_file_);
    return true;
}

void InputHandler::close()
{   
    if(file_stream_.is_open()){
        file_stream_.close();
        spdlog::info("Input file closed");
    }
}

bool InputHandler::readLine(unsigned int &timestamp, std::string &text, bool &is_query, int &k)
{
    std::string line;
    if(!std::getline(file_stream_,line)){
        return false;
    }

    timestamp=parseTimestamp(line);

    // 【异常处理】时间戳乱序/迟到检测
    if (timestamp > ts) {
        ts = timestamp;
    } else {
        spdlog::debug("Out-of-order timestamp detected: current={}, previous={}", 
                     timestamp, ts);
        timestamp = ts;
    }

    text=extractText(line);
    k = parseQueryCommand(line);
    is_query=(k==-1)?false:true;
    
    return true;
    
}

bool InputHandler::eof() const
{
    return file_stream_.eof();
}

unsigned int InputHandler::parseTimestamp(const std::string &line)
{
    regex timePattern(R"(\[(\d+):(\d+):(\d+)\])");
    smatch match;
    
    if (regex_search(line, match, timePattern)) {
        int h = stoi(match[1]);
        int m = stoi(match[2]);
        int s = stoi(match[3]);
        return h * 3600 + m * 60 + s;
    }
    return 0;  
}

string InputHandler::extractText(const string &line)
{
    size_t pos=line.find(']');
    string text;

    if(pos!=string::npos && pos+1<line.length()){
        text=line.substr(pos+1);
    }

    //找第一个和最后一个非空字符
    size_t start=text.find_first_not_of(" \t\r\n");
    size_t end=text.find_last_not_of(" \t\r\n");

    if(start!=string::npos){
        return text.substr(start,end-start+1);
    }

    return "";
}

int InputHandler::parseQueryCommand(const std::string &line)
{
    size_t pos=line.find("[ACTION] QUERY");

    if(pos==std::string::npos){
        return -1;
    }
    else{
        size_t posk=line.find("K=");
        if(posk!=std::string::npos){
            return stoi(line.substr(posk+2));
        }
        else return -1;
    }
}
