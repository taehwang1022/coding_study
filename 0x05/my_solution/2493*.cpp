#include <bits/stdc++.h>


int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int N ; // number of tower

    std::stack<std::pair<int,int>> number_height; // <tower height , number>
    number_height.push({100000001, 0});

    std::cin >> N;

    for(int i =1; i < N+1; i++)
    {
        int height;
        std::cin >> height;
        while(number_height.top().first < height)
        {
            number_height.pop();
        }
        std::cout << number_height.top().second << " ";
        number_height.push({height, i});
    }


}