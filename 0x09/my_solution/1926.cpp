#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <queue>
#include <utility>
#include <stack>
#include <map>
#include <cmath>

using namespace std;

int n , m; // 가로 세로

int board [502][502];   // 그림판
int vis [502][502]; // 방문한 곳

int dx[4] = {1,0,-1,0};
int dy[4] = {0,1,0,-1};


int main(void){
  ios::sync_with_stdio(0);
  cin.tie(0);

  cin >> n >> m;

  for(int i=0 ; i< n; i++)
  {
    for(int j =0 ; j<m;j++)
    {
        cin >> board[i][j];
    }
  }

  int mx =0;
  int num =0;

  for(int i =0 ; i < n ; i++)
  {
    for(int j =0 ; j<m; j++)
    {
        if(board[i][j] == 0 || vis[i][j] == 1)
        {
            continue;
        }
        num ++; //그림 숫자 하나 추가
        queue<pair<int, int>> Q;
        vis[i][j] =1;
        Q.push({i,j});

        int area =0;
        while(!Q.empty())
        {
            area++;

            pair<int,int> cur = Q.front();
            Q.pop();
            for(int i =0; i< 4; i ++)
            {
                int nx = cur.first + dx[i];
                int ny = cur.second + dy[i];
                if(nx<0 || nx >=n || ny<0 || ny>=m)
                {
                    continue;
                }
                if(board[nx][ny] != 1 || vis[nx][ny])
                {
                    continue;
                }
                vis[nx][ny] = 1;
                Q.push({nx,ny});
            }
        }

        mx = max(mx,area);
    }
  }

  cout << num << "\n" << mx;

}
