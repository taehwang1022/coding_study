#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int a, b, c;

    std::cin >> a >> b >> c;

    int dice_num[3];
    dice_num[0] =a;
    dice_num[1] =b;
    dice_num[2] =c;
    int arr[7] ={0};
    int state=1;
    int prize;

    for(int i =0 ; i< 3 ; i++)
    {
        arr[dice_num[i]] +=1;
    }

    for(int i =1 ; i< 7 ; i++)
    {
        if(arr[i] ==2) state = 2;
        else if(arr[i]==3) state = 3;
    }

    if(state == 1)
    {
        for(int i =1 ; i< 7 ; i++)
        {
            if(arr[i] == 1)
            {
                prize = i;
            }
        }
        prize *= 100;
    }
    else if(state ==2)
    {
        for(int i =1 ; i< 7 ; i++)
        {
            if(arr[i] == 2)
            {
                prize = i;
            }
        }
        prize = 1000 + prize*100;
    }
    else
    {
        for(int i =1 ; i< 7 ; i++)
        {
            if(arr[i] == 3)
            {
                prize = i;
            }
        }
        prize = 10000 + prize*1000;
    }

    std::cout << prize;
    
}
