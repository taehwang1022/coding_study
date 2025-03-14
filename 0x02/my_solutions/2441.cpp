#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);


    int N;

    std::cin >> N;  

    for(int i =0 ; i<N; i++)
    {
        for(int j=N-i; j<N;j++)
        {
            std::cout << " "; 
        }
        for(int j=0; j<N-i;j++)
        {
            std::cout << "*"; 
        }
        std::cout << "\n"; 
    }


}