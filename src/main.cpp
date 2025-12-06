#include<iostream>
#include "PreProcessor.h"

using namespace std;

int main(){
    PreProcessor processor("dict");

    processor.loadStopWords("data/stopwords.txt");
    
    processor.addUserWord("人工智能");
    processor.addUserWord("深度学习");

    vector<string> words=processor.processFile("data/text.txt");

    cout<< "Result: ";
    for (const string& w : words) {
        cout << w << " / ";
    }
    cout << endl;

    return 0;
}