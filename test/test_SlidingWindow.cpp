#include "SlidingWindow.h"
#include <cassert>
#include <iostream>

using namespace std;

void test_cnt(){
    SlidingWindow w(600);

    TimeSlot t1(0);
    t1.words={"人工智能", "中山大学"};

    TimeSlot t2(10);
    t2.words={"计算机科学与技术","人工智能"};

    w.addData(t1);
    w.addData(t2);
    
    assert(w.getWordCount("人工智能")==2);
    assert(w.getWordCount("计算机科学与技术")==1);
    assert(w.getWordCount("中山大学")==1);

    cout << "test_cnt passed"<<endl;
}

void test_eviction(){
    SlidingWindow w(600);

    TimeSlot t1(0);
    t1.words={"人工智能", "中山大学"};

    TimeSlot t2(601);
    t2.words={"计算机科学与技术","人工智能"};

    w.addData(t1);
    w.addData(t2);
    
    assert(w.getWordCount("人工智能")==1);
    assert(w.getWordCount("计算机科学与技术")==1);

    cout << "test_eviction passed"<<endl;
}

void test_topk(){
    SlidingWindow w(600);

    TimeSlot t1(0);
    t1.words={"人工智能", "中山大学","中山大学","人工智能","人工智能","人工智能","计算机科学与技术"};

    w.addData(t1);
    auto top2=w.getTopK(2);
    
    assert(top2.size()==2);
    assert(top2[0].first=="人工智能");
    assert(top2[0].second==4);
    assert(top2[1].first=="中山大学");

    cout << "test_topk passed"<<endl;
}

int main() {
    test_cnt();
    test_eviction();
    test_topk();
    std::cout << "All SlidingWindow tests passed!\n";
    return 0;
}

/**
 * cd HotWordsStatics/src
 * g++ -std=c++17 test_SlidingWindow.cpp SlidingWindow.cpp -pthread -o test_SlidingWindow
 * ./test_SlidingWindow
 */
