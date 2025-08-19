#include <iostream>
#include <queue>
#include <cmath>
#include <algorithm>

using namespace std;

char board[1024][1024];
int board_fire[1024][1024];
int board_people[1024][1024];

int vis[1024][1024];
int vis_fire[1024][1024];

int dx[4] = {1, 0 ,-1, 0};
int dy[4] = {0, 1, 0, -1};

queue<pair<int,int>> fire_q;
queue<pair<int,int>> people_q;

// #: 벽
// .: 지나갈 수 있는 공간
// J: 지훈이의 미로에서의 초기위치 (지나갈 수 있는 공간)
// F: 불이 난 공간

void print_board(int R, int C , auto board[][1024])
{
    cout << "--------print_board---------\n";
    for (int i =0 ;i <R; i++)
    {
        for(int j =0 ; j< C; j++)
        {
            cout << board[i][j];
        }
        cout << "\n";  
    }
}

void initialize_board(int R, int C , auto board[][1024])
{
    for (int i =0 ;i <R; i++)
    {
        for(int j =0 ; j< C; j++)
        {
            board[i][j] = -1;
        }
    }
}
int main()
{

    int R; // 행
    int C; // 열

    cin >> R >> C;

    initialize_board(R,C,board_fire);
    initialize_board(R,C,board_people);

    for (int i =0 ;i <R; i++)
    {
        string s;
        cin >> s;
        for(int j =0 ; j< C; j++)
        {
            board[i][j] = s[j];
            if(s[j] == 'F')
            {
                fire_q.push({i,j});
                vis_fire[i][j]=1;
                board_fire[i][j] =0;
            }
            else if(s[j] == 'J')
            {
                people_q.push({i,j});
                vis[i][j] =1;
                board_people[i][j] =0;
            }
        }
    }

    while(!fire_q.empty())
    {
        pair<int,int> fire_cur;
        fire_cur = fire_q.front();
        fire_q.pop();
        

        for(int i =0 ; i < 4; i++)
        {
            int x = fire_cur.first + dx[i];
            int y = fire_cur.second + dy[i];

            if(x < 0 || x >=R || y <0 || y>=C)
            {
                continue;
            }
            if(board[x][y] == '#' || vis_fire[x][y])
            {
                vis_fire[x][y] =1;
                continue;
            }
            board_fire[x][y] = board_fire[fire_cur.first][fire_cur.second] +1;
            vis_fire[x][y] = 1;
            fire_q.push({x,y});

        }
    }

    while(!people_q.empty())
    {
        pair<int,int> cur;
        cur = people_q.front();
        people_q.pop();
        

        for(int i =0 ; i < 4; i++)
        {
            int x = cur.first + dx[i];
            int y = cur.second + dy[i];

            if(x < 0 || x >=R || y <0 || y>=C)
            {
                cout << board_people[cur.first][cur.second] +1;
                return 0;
            }
            if(board[x][y] == '#' || vis[x][y] ==1)
            {
                vis[x][y] =1;
                continue;
            }
            if(board_fire[x][y] != -1 && board_fire[x][y] <= board_people[cur.first][cur.second] +1)
            {
                vis[x][y] =1;
                continue;
            }
            board_people[x][y] = board_people[cur.first][cur.second] +1;
            vis[x][y] = 1;
            people_q.push({x,y});

        }
    }

    cout << "IMPOSSIBLE";


    // print_board(R,C,board_fire);

    // print_board(R,C,board_people);







}