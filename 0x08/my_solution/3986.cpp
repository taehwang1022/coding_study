#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int N;
    int answer =0;

    std::cin >> N;
    while(N--)
    {
        std::string sentence;
        std::stack<char> stack_box;

        std::cin >> sentence;

        for(auto s : sentence)
        {
            if(s == 'A')
            {
                if(stack_box.empty())
                {
                    stack_box.push(s);
                }
                else if (stack_box.top()=='B')
                {
                    {
                        stack_box.push(s);
                    }
                }
                else if (stack_box.top()=='A')
                {
                    {
                        stack_box.pop();
                    }
                }
            }
            if(s == 'B')
            {
                if(stack_box.empty())
                {
                    stack_box.push(s);
                }
                else if (stack_box.top()=='A')
                {
                    {
                        stack_box.push(s);
                    }
                }
                else if (stack_box.top()=='B')
                {
                    {
                        stack_box.pop();
                    }
                }
            }
        }
        if(stack_box.empty())
        {
            answer++;
        }

    }
    std::cout << answer;
}