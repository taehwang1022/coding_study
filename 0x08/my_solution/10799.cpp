#include <bits/stdc++.h>

int main()
{
    using namespace std;

    // ios::sync_with_stdio(0);
    // cin.tie(0);
    int razer_flag =1;
    int stic_num =0;
    string sentence;

    stack<char> stack_box;
    cin >> sentence;
    for(auto s : sentence)
    {
        if(s == '(')
        {
            if(stack_box.empty())
            {
                razer_flag =1;
            }
            stack_box.push(s);
            if(stack_box.size() >= 2)
            {
                razer_flag =1;
            }

        }
        if(s == ')')
        {
            if(razer_flag == 1)
            {
                stack_box.pop();
                // cout << s <<"  레이저 쿠와아아ㅏㄱ : " << stack_box.size() << '\n';
                stic_num += stack_box.size();
                razer_flag = 0;
            }
            else
            {
                stic_num += 1;
                stack_box.pop();
            }
        }
        
    }
    cout << stic_num;
}