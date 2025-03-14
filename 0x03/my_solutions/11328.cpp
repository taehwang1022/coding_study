#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int N ;


    std::cin >> N;

    int answer_box[N] = {};

    for(int i =0 ; i < N ; i++)
    {
        std::string a, b;

        int a_answer_box[26]={}, b_answer_box[26]={};

        std::cin >> a >> b;

        if (a.size() != b.size()) 
        {
            answer_box[i] = 1;
            continue;
        }

        for(int j = 0 ; j< a.size(); j++)
        {
            a_answer_box[a[j]-'a']++;
            b_answer_box[b[j]-'a']++;
        }

        for(int j = 0; j< 26; j++)
        {
            if(a_answer_box[j] != b_answer_box[j])
            {
                answer_box[i] = 1;
                break;
            }
        }
    }

    for(int i =0 ; i<N ; i++)
    {
        if(answer_box[i] == 1)
            std::cout << "Impossible\n";  
        else
            std::cout << "Possible\n";  
    }
}



