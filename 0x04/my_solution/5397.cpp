#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int repeat_num;
    std::cin >> repeat_num;



    while(repeat_num--)
    {
        std::string password;
        std::cin >> password;

        std::list<char> L;
        auto cursor = L.end();
        for(int i = 0; i< password.length(); i++)
        {
            char input;
            input = password[i];
            if(input == '<')
            {
                if(cursor != L.begin())
                {
                    cursor--;
                }
            }
            else if(input == '>')
            {
                if(cursor != L.end())
                {
                    cursor++;
                }
            }
            else if(input == '-')
            {
                if(cursor != L.begin())
                {
                    cursor--;
                    cursor = L.erase(cursor);
                }
            }
            else
            {
                L.insert(cursor, input);
            }
            // for(auto c : L)
            // {
            //     std::cout << c;
            // }
            // std::cout << "\n";
        }
        for(auto c : L)
        {
            std::cout << c;
        }
        std::cout << "\n";
    } 


}