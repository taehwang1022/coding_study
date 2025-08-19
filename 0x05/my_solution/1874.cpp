#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int N;
    std::cin >> N;

    std::stack<int> num_box;
    std::stack<int> answer_box;
    int cannot_flag = 0;
    std::string ans;

    for(int i = 0; i < N; i++)
    {
        num_box.push(N-i);
    }

    for(int i = 0; i < N; i++)
    {
        int target_num;
        std::cin >> target_num;

        while(answer_box.empty() || target_num != answer_box.top())
        {
            if (num_box.empty()) {
                cannot_flag = 1;
                break;
            }
            
            answer_box.push(num_box.top());
            num_box.pop();
            ans += "+\n" ;
        }

        if(target_num == answer_box.top())
        {
            ans += "-\n" ;
            answer_box.pop();

        }
    }

    if(cannot_flag == 1)
    {
        std::cout << "NO";
    }
    else
    {
        std::cout << ans;
    }



}