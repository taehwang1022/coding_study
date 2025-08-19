#include <bits/stdc++.h>

int main()
{

    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int answer_box[10];

    std::fill(answer_box, answer_box + 10, 0);
    std::string s;

    std::cin >> s;

    for( char e : s)
    {
        if((e -'0') == 6 ||(e -'0') == 9 )
        {
        answer_box[6]++;
        }
        else
        {
            answer_box[e-'0']++;
        }
    }

    answer_box[6] = (answer_box[6] + 1) / 2;

        int answer=0;
    for(int e : answer_box)
    {
        if(answer < e)
        answer = e;
    }

    std::cout << answer;



}