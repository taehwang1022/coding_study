#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    std::string initial_sentence;
    std::cin >> initial_sentence;

    std::list<char> L;

    for (auto c : initial_sentence)
    {
        L.push_back(c);
    }

    auto cursor = L.end();

    int repeat_num;
    std::cin >> repeat_num;

    while(repeat_num--)
    {
        char status;
        std::cin >> status;

        // move cursor to left if it is initial point ignore this status.
        if(status == 'L')
        {
            if(cursor != L.begin())
            {
                cursor--;
            }        
        }
        // move cursor to right. if it is last point ignore this status.
        else if(status == 'D')
        {
            if(cursor != L.end())
            {
                cursor++;
            }
        }
        // erase word to the left of the cursor
        else if(status == 'B')
        {
            if(cursor != L.begin())
            {
                cursor--;
                cursor = L.erase(cursor);
            }
            
        }
        // insert word to the left of the cursor
        else if(status == 'P')
        {
            char add;
            std::cin >> add;
            L.insert(cursor, add);
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
}