#ifndef TEST_SW_QH_H
#define TEST_SW_QH_H

//集成测试：SlidingWindow+QueryHandler
#include "IntegrationTestBase.h"
#include "SlidingWindow.h"
#include "QueryHandler.h"
#include<iostream>
using namespace std;

class test_SW_QH:public IntegrationTestBase{
    private:
    string output_file_;
    protected:
    void SetUp() override{
        output_file_="../../data/test_output1.txt";
        LOG("输出文件: " + output_file_);
    }

    void TearDown() override{
        LOG("输出文件已保留: " + output_file_);
    }

    void RunTest() override{
        SlidingWindow window(600);
        QueryHandler handler(output_file_);

        ASSERT_TRUE(handler.open(), "输出文件打开失败");
        LOG("输出文件打开成功");

        //测试窗口1
        TimeSlot slot1(100);
        slot1.words = {"人工智能", "技术", "人工智能", "发展", "人工智能", "技术"};
        window.addData(slot1);
        LOG("添加第1个时间槽: timestamp=100, 词数=6");

        //测试能否追加（同一时间戳）
        TimeSlot slot4(100);
        slot4.words = {"中山大学", "孙中山", "中山大学", "发展", "中山大学", "孙中山"};
        window.addData(slot4);
        LOG("添加第2个时间槽: timestamp=100, 词数=6");

        window.addData(TimeSlot(100));
        auto topk3 = window.getTopK(3);
        handler.outputTopK(100,topk3);
        LOG("  查询1: timestamp=100, Top-3");

        TimeSlot slot2(200);
        slot2.words = {"技术", "创新", "人工智能", "技术"};
        window.addData(slot2);
        LOG("添加第3个时间槽: timestamp=200, 词数=4");

        window.addData(TimeSlot(300));
        auto topk1 = window.getTopK(3);
        handler.outputTopK(300,topk1);
        LOG("  查询1: timestamp=300, Top-3");

        //测试窗口二，淘汰机制
        TimeSlot slot3(500);
        slot3.words = {"创新", "技术", "创新", "创新", "创新", "发展"};
        window.addData(slot3);
        LOG("添加第4个时间槽: timestamp=500, 词数=6");
        
        window.addData(TimeSlot(701));
        auto topk2 = window.getTopK(3);
        handler.outputTopK(700,topk2);
        LOG("  查询2: timestamp=700, Top-3");
    }

    public:
    test_SW_QH():IntegrationTestBase("SlidingWindow + QueryHandler 集成测试") {}
};

#endif