#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    while (1)
    {
        std::string sentence;
        std::getline(std::cin, sentence);
        int is_wrong =0;
        std::stack<char> stack_box;
        if(sentence[0] == '.')
        {
            break;
        }
        for(auto s : sentence)
        {
            if(s == '(')
            {
                stack_box.push(s);
            }
            else if(s == '[')
            {
                stack_box.push(s);
            }
            if(s== ')')
            {
                if(stack_box.empty() || stack_box.top() !='(')
                {
                    is_wrong =1;
                    break;
                }
                stack_box.pop();
            }
            if(s== ']')
            {
                if(stack_box.empty() || stack_box.top() !='[')
                {
                    is_wrong =1;
                    break;
                }
                stack_box.pop();
            }

        }
        if(stack_box.empty() && is_wrong!=1)
        {
            std::cout << "yes" <<'\n';           
        }
        else
        {
            std::cout << "no" <<'\n';
        }

    }
    
}