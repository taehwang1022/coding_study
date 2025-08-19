#include <iostream>

int main()
{
    using namespace std;
    int a, b, c;
    int answer_box[3];
    cin >> a >> b >>c ;
    if(a<b && a <c)
    {
        answer_box[0] = a;
        if(b<c)
        {
                answer_box[1] = b;
                answer_box[2] = c;
        }
        else
           {
                answer_box[1] = c;
                answer_box[2] = b;
           }
      }
    else if(b<a && b <c)
    {
        answer_box[0] = b;
        if(a<c)
        {
                answer_box[1] = a;
                answer_box[2] = c;
        }
        else
           {
                answer_box[1] = c;
                answer_box[2] = a;
           }
      }    
      else
    {
        answer_box[0] = c;
        if(a<b)
        {
                answer_box[1] = a;
                answer_box[2] = b;
        }
        else
           {
                answer_box[1] = b;
                answer_box[2] = a;
           }
      }
      
      cout << answer_box[0] << " " <<answer_box[1] << " " <<answer_box[2] << " ";
}