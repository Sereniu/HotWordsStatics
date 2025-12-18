# Makefile for 热词统计系统

#定义变量
CXX=g++  #编译器
CXXFLAGS=-std=c++17 -Wall -O2 -pthread  #编译选项
# -std=c++17：使用 C++17 标准
# -Wall：显示所有警告
# -O2：优化级别2（0=无优化，1=基本，2=标准，3=激进）
# -pthread：支持多线程（等价于 -lpthread）
INCLUDES= -I./include  #头文件搜索路径
SRC_DIR=src#目录变量
BIN_DIR=bin

#源文件列表
SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/HotWordSystem.cpp \
          $(SRC_DIR)/InputThread.cpp \
          $(SRC_DIR)/InputHandler.cpp \
          $(SRC_DIR)/TextProcessor.cpp \
          $(SRC_DIR)/StatisticsThread.cpp \
          $(SRC_DIR)/SlidingWindow.cpp \
          $(SRC_DIR)/QueryHandler.cpp

#生成可执行文件的路径
TARGET=$(BIN_DIR)/hotword_system

#规则
#第一个目标：make or make all
all: dirs $(TARGET)

dirs:
	@mkdir -p $(BIN_DIR) #创建bin目录

#编译主程序
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SOURCES) -o $(TARGET)
	@echo "✓ 编译完成: $(TARGET)"
# g++ -std=c++17 -Wall -O2 -pthread -I./src src/main.cpp ... -o bin/hotword_system

#运行程序
run: $(TARGET)
	@echo "运行程序..."
	@cd $(BIN_DIR) && ./hotword_system input1.txt output1.txt

#批量处理文件
run_all: $(TARGET)
	@echo "批量处理所有输入文件..."
	@cd $(BIN_DIR) && \
	if [ -f ../data/input1.txt ]; then \
		./hotword_system ../data/input1.txt ../data/output1.txt; \
	fi && \
	if [ -f ../data/input2.txt ]; then \
		./hotword_system ../data/input2.txt ../data/output2.txt; \
	fi && \
	if [ -f ../data/input3.txt ]; then \
		./hotword_system ../data/input3.txt ../data/output3.txt; \
	fi

#清理编译文件
clean:
	rm -rf $(BIN_DIR)
	rm -f src/*.o
	@echo "✓ 清理完成"


### 帮助信息
help:
	@echo "热词统计系统 - Makefile 使用说明"
	@echo ""
	@echo "使用方法:"
	@echo "  make          - 编译程序"
	@echo "  make run      - 编译并运行 input1.txt"
	@echo "  make run_all  - 批量处理所有输入文件"
	@echo "  make clean    - 清理编译文件"
	@echo "  make help     - 显示帮助"

#伪目标（Phony Targets）
.PHONY: all dirs run run_all clean help
#这个目标不是真实文件,直接执行目标对应的命令。
