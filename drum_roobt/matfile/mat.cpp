#include <iostream>   // 표준 입출력 라이브러리
#include <matio.h>    // Matio 라이브러리 (MATLAB 파일을 읽기 위한 라이브러리)
#include <vector>     // std::vector 사용

int main() {
    const char *filename = "matFileTest.mat";

    // .mat 파일 열기
    mat_t *matfp = Mat_Open(filename, MAT_ACC_RDONLY);
    if (!matfp) {
        std::cerr << "파일을 열 수 없습니다: " << filename << "\n";
        return 1;
    }

    // "data" 변수 읽기
    matvar_t *matvar = Mat_VarRead(matfp, "matFileTest");  // 실제 변수명을 확인해서 변경하세요.
    if (!matvar) {
        std::cerr << "변수를 찾을 수 없습니다: data\n";
        Mat_Close(matfp);
        return 1;
    }

    // 데이터 차원 확인 (2차원 배열인지 확인)
    if (matvar->rank != 2 || matvar->dims[1] != 2) {
        std::cerr << "데이터 형식이 (N,2) 형태의 배열이 아닙니다.\n";
        Mat_VarFree(matvar);
        Mat_Close(matfp);
        return 1;
    }

    // 데이터 타입 확인
    if (matvar->class_type != MAT_C_DOUBLE) {
        std::cerr << "지원되지 않는 데이터 타입입니다. (double 배열 필요)\n";
        Mat_VarFree(matvar);
        Mat_Close(matfp);
        return 1;
    }

    // 데이터 포인터 가져오기
    double *data = static_cast<double *>(matvar->data);
    size_t rows = matvar->dims[0]; // 행 개수

    // 데이터를 (키, 값) 형태로 저장할 벡터
    std::vector<std::pair<int, int>> key_value_pairs;

    std::cout << "파일 내용:\n";
    for (size_t i = 0; i < rows; i++) {
        int key = static_cast<int>(data[i]);              // 첫 번째 열 (key)
        int value = static_cast<int>(data[i + rows]);     // 두 번째 열 (value)
        key_value_pairs.push_back({key, value});
        std::cout << key << " " << value << '\n';
    }

    // 사용자 입력을 받아 값 찾기 (선형 탐색 방식)
    int input;
    std::cout << "숫자를 입력하세요: ";
    std::cin >> input;

    bool found = false;
    for (const auto &pair : key_value_pairs) {
        if (pair.first == input) {
            std::cout << "대응 값: " << pair.second << '\n';
            found = true;
            break;
        }
    }

    if (!found) {
        std::cout << "해당 숫자에 대한 값이 없습니다.\n";
    }

    // 메모리 해제
    Mat_VarFree(matvar);
    Mat_Close(matfp);

    return 0;
}
