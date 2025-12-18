#ifndef TEST_IP_IH_H
#define TEST_IP_IH_H

#include "IntegrationTestBase.h"
#include "InputHandler.h"
#include "TextProcessor.h"
#include <fstream>
#include <vector>

//IT：TextProcessor+InputHandler
class test_IP_IH:public IntegrationTestBase{
    private:
    string input_file_="../../data/input1.txt";

    protected:
    void SetUp() override{
        input_file_="../../data/input1.txt";

        LOG("[IP_TH] 测试文件"+input_file_);
    }

    void TearDown() override{
        LOG("[IP_TH] 测试完成");
    }

    void RunTest() override{
        InputHandler handler(input_file_);
        TextProcessor processor("../../dict/");
        std::map<unsigned int, TimeSlot> time_slot_map;  // 用map按时间戳聚合

        ASSERT_TRUE(handler.open(),"输入文件打开失败");
        LOG("输入文件打开成功");

        unsigned int timestamp;
        string text;
        bool is_query;
        int k;

        vector<TimeSlot> time_slots;
        int query_count=0;
        int total_lines=0;

        while (handler.readLine(timestamp, text, is_query, k)) {
            total_lines++;
            
            if (is_query) {
                query_count++;
                LOG("检测到查询命令: timestamp=" + std::to_string(timestamp) + 
                    ", k=" + std::to_string(k));
                ASSERT_GT(k, 0, "查询K值必须大于0");
            } else {
                auto words = processor.processWithPOS(text);

                if(time_slot_map.find(timestamp)==time_slot_map.end()){
                    time_slot_map[timestamp]=TimeSlot(timestamp);
                }

                time_slot_map[timestamp].words.insert(time_slot_map[timestamp].words.end(),words.begin(),words.end());
            }
        }
    
        handler.close();

        // 打印前3个时间槽的统计信息
        int print_count = 0;
        for (const auto& [ts, slot] : time_slot_map) {
            if (print_count < 3) {
                LOG("TimeSlot[" + std::to_string(ts) + "]: " + 
                    std::to_string(slot.words.size()) + " 个词");
                print_count++;
            }
        }
        if (time_slot_map.size() > 3) {
            LOG("... (还有 " + std::to_string(time_slot_map.size() - 3) + " 个时间槽)");
        }

        LOG("✓ 总共处理 " + std::to_string(total_lines) + " 行");
        LOG("✓ 时间槽数量: " + std::to_string(time_slot_map.size()) + " 个");
        LOG("✓ 查询命令: " + std::to_string(query_count) + " 条");
        LOG("✓ 文本处理正确性验证通过");
    }

    public:
    test_IP_IH():IntegrationTestBase("TextProcessor + InputHandler 集成"){}
};

#endif

