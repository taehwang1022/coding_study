#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int num;
    int Y=0, M=0;

    std::cin >> num;

    int arr[num];

    for(int i =0 ; i< num; i++)
    {
        std::cin>>arr[i];
        Y += 10*(arr[i]/30 +1);
        M += 15*(arr[i]/60 +1);
    }

    if(Y > M) 
        std::cout << "M" << " " << M;
    else if(Y == M) std::cout << "Y" << " " << "M" << " "<< Y; 
    else std::cout << "Y" << " "<< Y;
    

}