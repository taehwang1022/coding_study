# 컴파일러 설정
CXX = g++
CXXFLAGS = -Wall -I/usr/include
LDFLAGS = -L/usr/lib -lmatio

# 실행 파일 이름
TARGET = mat

# 소스 파일
SRC = mat.cpp

# 빌드 규칙
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# 실행
run: $(TARGET)
	./$(TARGET)

# 정리 (생성된 파일 삭제)
clean:
	rm -f $(TARGET)
