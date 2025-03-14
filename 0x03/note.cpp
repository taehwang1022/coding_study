#include <bits/stdc++.h>
using namespace std;

int main()
{
    int a[21];
    int b[21][21];

    // // 1. for문을 사용하여 배열을 초기화하는 방법
    // for(int i = 0; i < 21; i++)
    // {
    //     a[i] = 0; // a 배열 초기화
    // }

    // for(int i = 0; i < 21; i++)
    // {
    //     for(int j = 0; j < 21; j++)
    //     {
    //         b[i][j] = 0; // b 2차원 배열 초기화
    //     }
    // }

    // 2. fill 함수를 사용하여 배열을 초기화하는 방법
    fill(a, a + 21, 10); // a 배열 초기화
    for(int i =0 ; i< 21 ; i++)
    {
        fill(b[i], b[i]+21, 10);
    }
    // fill(&b[0][0], &b[0][0] + 21 * 21, 0); // b 2차원 배열 초기화

    // 배열이 잘 초기화되었는지 출력
    cout << "Array a: ";
    for(int i = 0; i < 21; i++)
    {
        cout << a[i] << " ";
    }
    cout << endl;

    cout << "2D Array b:" << endl;
    for(int i = 0; i < 21; i++)
    {
        for(int j = 0; j < 21; j++)
        {
            cout << b[i][j] << " ";
        }
        cout << endl;
    }

    return 0;
}
