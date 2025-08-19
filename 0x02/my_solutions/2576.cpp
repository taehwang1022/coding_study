#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int arr[7];
    int sum =0;
    int min =100;
    int state =0;

    for (int i=0 ; i<7; i++)
    {
        std::cin >> arr[i];
        if(arr[i]%2 == 1)
        {
            sum += arr[i];
            if(arr[i]<min)
            {
                min = arr[i];
            }
            state =1;
        }
    }
    if(state ==0) std::cout << "-1";
    else std::cout << sum <<"\n" << min;
}