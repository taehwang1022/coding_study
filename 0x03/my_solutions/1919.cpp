#include <bits/stdc++.h>

int main()
{
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    int word1_element[26]={0};
    int word2_element[26]={0};
    
    int answer =0;
    std::string word_1;
    std::string word_2;

    std:: cin >> word_1 >> word_2;

    for(int i =0 ; i< word_1.size(); i++)
    {
        word1_element[word_1[i] - 'a']++;
    }

    for(int i =0 ; i< word_2.size(); i++)
    {
        word2_element[word_2[i] - 'a']++;
    }

    for(int i =0 ; i< 26; i++)
    {
        answer = answer + ((word1_element[i]>= word2_element[i]) ? word1_element[i]-word2_element[i] : word2_element[i] - word1_element[i]);
        // std::cout << "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm" << "\n";
        // std::cout << "loop count  : " << i << "\n";
        // std::cout << "answer : " << answer << "\n";
        // std::cout << "word1_element : " << word1_element[i] << "\n";
        // std::cout << "word2_element : " << word2_element[i] << "\n";
        // std::cout << "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm" << "\n";
    }
    
    std::cout << answer;

}