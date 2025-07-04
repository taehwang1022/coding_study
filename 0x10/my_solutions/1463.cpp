#include <iostream>

int d[10000005];
int n;
int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    std::cin >> n;

    d[1] =0;

    for(int i=2; i<=n; i++ )
    {
        d[i] = d[i-1]+1;
        if(i%2 ==0)
        {
            d[i] = std::min(d[i],d[i/2]+1);
        }
        if(i%3 ==0)
        {
            d[i] = std::min(d[i],d[i/3]+1);
        }
    }
    std::cout << d[n];

}