#include "HotWordSystem.h"
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

using namespace std;

int main(int argc, char* argv[]){
    string input_file="../data/";
    string output_file="../data/";

     // 检查参数数量
    if (argc < 3) {
        cerr << "[错误] 参数不足！" << endl;
        cerr << "需要至少2个参数: <input_file> <output_file>" << endl;
        cout << endl;
        return 1;
    }

    input_file += argv[1];
    output_file += argv[2];
    cout << "[Main] 输入文件: " << input_file << endl;
    cout << "[Main] 输出文件: " << output_file << endl;

    try{
        auto start_time=chrono::high_resolution_clock::now();

        //创建热词统计系统
         HotWordSystem system(input_file,output_file,300, 60,600,1);
                
        cout << endl;
        cout << "========================================" << endl;
        cout << "开始处理数据..." << endl;
        cout << "========================================" << endl;
        cout << endl;

        system.start();
        system.join();

        // 记录结束时间
        auto end_time = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
        
        cout << endl;
        cout << "========================================" << endl;
        cout << "处理完成！" << endl;
        cout << "========================================" << endl;
        cout << "  总耗时: " << duration.count() << " ms" << endl;

         if (duration.count() > 0) {
            cout << "  处理速度: " << fixed << setprecision(2) << (1000.0 / duration.count()) << " 次/秒" << endl;
        }
        
        cout << "  输出文件: " << output_file << endl;
        cout << endl;
        cout << "  系统运行成功！" << endl;
        cout << endl;
        
        return 0;
    }catch(const exception& e){
        cerr << endl;
        cerr << "========================================" << endl;
        cerr << "  系统运行失败" << endl;
        cerr << "========================================" << endl;
        cerr << "错误信息: " << e.what() << endl;
        cerr << endl;
        return 1;
    }
}