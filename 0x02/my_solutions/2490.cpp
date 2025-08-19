#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int arr[3][4];
    int state=0;
    
    for (int i =0 ; i <3; i++)
    {
        for(int j=0; j<4;j++)
        {
            std::cin >> arr[i][j];
        }
    }

    for (int i =0 ; i <3; i++)
    {
        for(int j=0; j<4;j++)
        {
            state += arr[i][j];
        }
        if(state == 0 ) std::cout << "D" << "\n";
        if(state == 3 ) std::cout << "A" << "\n";
        if(state == 2 ) std::cout << "B" << "\n";
        if(state == 1 ) std::cout << "C" << "\n";
        if(state == 4 ) std::cout << "E" << "\n";
        state =0;
    }

}