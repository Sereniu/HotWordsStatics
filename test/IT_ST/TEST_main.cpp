#include "test_TP_IH.h"
#include "test_SW_QH.h"
#include "test_Input_Buffer.h"
#include "test_Thread.h"
#include "TestRunner.h"

int main(){
    TestRunner runner;

    //runner.AddTest(make_unique<test_IP_IH>());
    //runner.AddTest(make_unique<test_SW_QH>());
    //runner.AddTest(make_unique<test_Input_Buffer>());
    runner.AddTest(make_unique<test_Thread>());
    runner.RunAll();
    return 0;
}

/**
 * g++ TEST_main.cpp ../../src/InputThread.cpp ../../src/InputHandler.cpp ../../src/TextProcessor.cpp ../../src/SlidingWindow.cpp ../../src/QueryHandle.cpp -o test_runner -I../../src -std=c++17
 * ./test_runner
 * 
 * g++ TEST_main.cpp     ../../src/InputThread.cpp     ../../src/InputHandler.cpp     ../../src/TextProcessor.cpp     ../../src/StatisticsThread.cpp     ../../src/SlidingWindow.cpp     ../../src/QueryHandler.cpp     -o test_runner     -I../../src     -std=c++17     -lpthread && ./test_runner
 */