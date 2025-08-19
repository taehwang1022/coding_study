#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int arr[5];
    int avg =0;

    for(int i=0 ;i<5;i++)
    {
        std::cin >> arr[i];
        avg += arr[i];
    }

    std::sort(arr,arr+5);

    avg /= 5;

    std::cout << avg << "\n" << arr[2];



}