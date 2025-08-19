#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int arr[9];
    int total=0;
    int num;

    int exit_num1, exit_num2;
    for(int i =0; i<9;i++)
    {
        std::cin >> arr[i];
        total += arr[i];
    }

    std::sort(arr, arr+9);

    total -= 100;


    for(int i =0; i<9;i++)
    {
        for(int n=i+1; n<9;n++)
        {
            if(total == (arr[i]+arr[n]))
            {
                exit_num1 = i;
                exit_num2 = n;
            }
        }

    }

    for(int i =0; i<9;i++)
    {
        if(i == exit_num1 || i==exit_num2)
        {

        }
        else 
        {
            std::cout << arr[i] <<"\n";
        }
    }

}