#include <bits/stdc++.h>

int main()
{
    using namespace std;
    ios::sync_with_stdio(0);
    cin.tie(0);
    int N, K;
    cin >> N >> K;

    vector<int> answer;
    list<int> L;
    for(int i = 0; i < N; i++)
    {
        L.push_back(i+1);
    }

    auto cursor = L.begin();

    for(int i = 0; i < N; i++)
    {
        for (int j = 0; j < K - 1; j++)
        {
            cursor++;
            if (cursor == L.end()) cursor = L.begin();            
        }
        answer.push_back(*cursor);
        cursor = L.erase(cursor);
        if (cursor == L.end()) cursor = L.begin();        

    }

    cout << "<";
    for (int i = 0; i < answer.size(); i++) {
        if (i > 0) cout << ", ";
        cout << answer[i];
    }
    cout << ">\n";

}