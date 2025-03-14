#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int repeat_num;
    std::cin >> repeat_num;
    std::stack<int> S;
    int answer = 0;
    while(repeat_num--)
    {
        int num;
        std::cin >> num;

        if(num == 0)
        {
            S.pop();
        }
        else
        {
            S.push(num);
        }
    }

    while(!S.empty())
    {
        answer += S.top();
        S.pop();
    }

    std::cout << answer;
}