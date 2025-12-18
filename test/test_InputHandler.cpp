#include "InputHandler.h"
#include <cassert>
#include <iostream>

int main(){
    InputHandler handler("../data/input_handler.txt");

    handler.open();
    
    unsigned int timestamp[3];
    string text[3];
    int k;
    bool is_query;

    
    handler.readLine(timestamp[0],text[0],is_query,k);
    handler.readLine(timestamp[1],text[1],is_query,k);

    assert(!is_query);
    assert(k==-1);

    handler.readLine(timestamp[2],text[2],is_query,k);
    

    assert(is_query);
    assert(k==3);
    assert(timestamp[0]==1201 && text[0]=="都是门糜芳傅士仁的锅");
    assert(timestamp[1]==3602 && text[1]=="徐庶不走就好了");

    std::cout<<"InputHandler Pass"<<endl;
    
    return 0;
}

/**
 * cd HotWordsStatics/src
 * g++ -std=c++17 test_InputHandler.cpp InputHandler.cpp -pthread -o test_InputHandler
 * ./test_InputHandler
 */