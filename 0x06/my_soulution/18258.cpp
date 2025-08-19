#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);
    std::queue<int> q;
    int repeat_num;
    std::cin >> repeat_num;

    for(int i = 0; i < repeat_num; i++)
    {
        std::string order;
        std::cin >> order;

        if(order == "push")
        {
            int N;
            std::cin >> N;
            q.push(N);
        }
        else if(order == "pop")
        {
            if(q.empty())
            {
                std::cout << "-1" << "\n";
            }
            else
            {
                std::cout << q.front() << "\n";
                q.pop();
            }
        }
        else if(order == "size")
        {
            std::cout << q.size() << "\n";
        }
        else if(order == "empty")
        {
            if(q.empty())
            {
                std::cout << "1" << "\n";
            }
            else
            {
                std::cout << "0" << "\n";
            }
        }
        else if(order == "front")
        {
            if(q.empty())
            {
                std::cout << "-1" << "\n";
            }
            else
            {
                std::cout << q.front() << "\n";
            }
        }
        else if(order == "back")
        {
            if(q.empty())
            {
                std::cout << "-1" << "\n";
            }
            else
            {
                std::cout << q.back() << "\n";
            }
        }
    }

    return 0;
}