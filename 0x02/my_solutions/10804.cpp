#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int arr[20];

    for(int i = 0 ;  i < 20; i++)
    {
        arr[i] = i+1;
    }

    for(int i =0 ; i< 10; i++)
    {
        int A,B;
        std::cin >> A >> B;

        for(int j=0; j<((B-A+1)/2);j++)
        {
            std::swap(arr[A+j-1],arr[B-j-1]);
        }
    }


    for(int i = 0 ;  i < 20; i++)
    {
        std::cout << arr[i] << " ";
    }
}