CXX      = g++
CXXFLAGS = -std=c++17 -O2 -DLAB6_BUILD
# 注意：不含 slr1.cpp（它是离线分析表生成器，有自己的 main）；-DLAB6_BUILD 排除 scanner/semantic 的测试 main
SRC      = scanner.cpp semantic.cpp irgen.cpp memory.cpp codec.cpp lab7_main.cpp

lab7: $(SRC)
	$(CXX) $(CXXFLAGS) -o lab7 $(SRC)

clean:
	rm -f lab7 lab7.exe out.s prog *.o

.PHONY: clean
