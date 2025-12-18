#ifndef TESTRUNNER_H
#define TESTRUNNER_H

#include "IntegrationTestBase.h"
#include "test_TP_IH.h"
#include<iostream>
#include<chrono>
#include<vector>
#include<string>
#include<memory>

using namespace std;

class TestRunner {
private:
    vector<unique_ptr<IntegrationTestBase>> tests_;
    
public:
    void AddTest(unique_ptr<IntegrationTestBase> test) {
        tests_.push_back(move(test));
    }
    
    void RunAll() {
        cout << endl;
        cout << "         热词统计与分析系统 - 集成测试"<<endl;
        cout << "══════════════════════════════════════════════════════════════"<<endl;
        
        if(tests_.empty()){
            cout<<"[error] 未添加任何测试用例，跳过执行"<<endl;
            return;
        }

        auto start_time = chrono::high_resolution_clock::now();
        
        for (auto& test : tests_) {
            test->Run();
        }
        
        auto end_time = chrono::high_resolution_clock::now();
        auto total_duration = chrono::duration_cast<chrono::milliseconds>(
            end_time - start_time);
        
        IntegrationTestBase::PrintSummary();
        cout << "总测试时间: " << total_duration.count() << " ms" << endl;
        cout << endl;
    }
    
    ~TestRunner()=default;
};

#endif