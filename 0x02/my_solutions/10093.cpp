#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    long long A,B;
    long long min,max;
    std::cin >>A >>B;

    if(A > B)
    {
        max = A;
        min = B;
    }
    else
    {
        max = B;
        min = A;
    }
    if(max == min)
    {
        std::cout << "0" << "\n";
    }
    else
    {
        std::cout << max-min-1 << "\n";
    }


    for(long long i = min+1; i<max;i++)
    {
        std::cout << i << " ";
    }
}