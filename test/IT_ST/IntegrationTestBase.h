/**
 * 热词统计系统--集成测试
 * TextProcessor+InputHandler
 * SlidingWindow+QueryHandler
 * StatisticsThread 全流程集成
 * HotWordSystem 系统测试
 */

#ifndef INTEGRATIONTESTBASE_H
#define INTEGRATIONTESTBASE_H

#include<iostream>
#include<string>
#include<iomanip>
using namespace std;

//测试基类
class IntegrationTestBase{
    protected:
    string test_name_;//测试名称
    bool passed_;//是否通过
    string error_msg_;//错误信息

    //静态变量：所有测试共享的统计数据
    static int total_tests_;//测试个数
    static int passed_tests_;//通过个数
    static int failed_tests_;//失败个数

    virtual void SetUp(){}//测试前
    virtual void TearDown(){}//测试后
    virtual void RunTest()=0; //纯虚函数，子类必须完成的测试中

    //断言
    //验证条件是否为真
    void ASSERT_TRUE(bool condition, const string& msg) {
        if (!condition) {
            throw runtime_error(msg);  // 条件不满足就抛异常
        }
    }
    //验证是否相等
    void ASSERT_EQ(int expected, int actual, const string& msg) {
        if (expected != actual) {
            ostringstream oss;
            oss << msg << " (期望: " << expected << ", 实际: " << actual << ")";
            throw runtime_error(oss.str());
        }
    }
    //是否大于
    void ASSERT_GT(int actual, int threshold, const string& msg) {
        if (actual <= threshold) {
            ostringstream oss;
            oss << msg << " (实际: " << actual << ", 阈值: " << threshold << ")";
            throw runtime_error(oss.str());
        }
    }
    //日志，打印测试过程信息
    void LOG(const string& msg) {
        cout << "    " << msg << endl;
    }

    public:

    IntegrationTestBase(const string& name):test_name_(name), passed_(false) {
        total_tests_++;
    }

    virtual ~IntegrationTestBase() = default;

    /** 
     * 核心函数
     * 调用SetUp()准备
     * 调用RunTest()执行测试
     * 调用TearDown()清理
     * 断言失败，捕获异常并抛出错误
    */
   void Run(){
        cout<<endl<<"[TEST "<<total_tests_<<"] "<<test_name_<<endl;
        cout<<string(50,'-')<<endl;

        try{
            SetUp();
            RunTest();
            TearDown();

            passed_=true;
            passed_tests_++;
            cout<<"  [PASS] 测试通过"<<endl;
        }catch(const exception& e){
            passed_=false;
            failed_tests_++;
            error_msg_=e.what();
            cout<<"  [FAIL] "<<error_msg_<<endl;
        }
   }

    //打印测试的总数据
    static void PrintSummary() {
        cout << endl << string(50, '=') << endl;
        cout << "测试总结" << endl;
        cout << string(50, '=') << endl;
        cout << "总测试数: " << total_tests_ << endl;
        cout << "通过: " << passed_tests_ << " ✓" << endl;
        cout << "失败: " << failed_tests_ << " ✗" << endl;
        cout << "通过率: " << fixed << setprecision(1) << (100.0 * passed_tests_ / total_tests_) << "%" << endl;
        cout << string(50, '=') << endl;
    }

};

int IntegrationTestBase::total_tests_=0;
int IntegrationTestBase::passed_tests_=0;
int IntegrationTestBase::failed_tests_=0;

#endif
