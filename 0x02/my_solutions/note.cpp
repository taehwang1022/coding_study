#include <iostream>

int main()
{
	using namespace std;

	int repeat;
	cin >> repeat;
	
	int* num_box = new int[repeat];

	for (int i =0 ; i < repeat ; ++i)
		cin >> num_box[i];
	int count = 0;
	int remain = 1;
	for (int i = 0; i < repeat; ++i)
	{
		for (int j = 0; j < num_box[i]; ++j)
		{
			if (num_box[i] == 1)
			{
				count++;
				break;
			}
			if (((num_box[i]) - (j + 1)) > 0)
			{
				remain = num_box[i] % ((num_box[i]) - (j + 1));
			}
			

			if (remain == 0 && (((num_box[i]) - (j + 1)) > 1))
			{
				cout << "지금 숫자 : " << num_box[i] << "카운트댐" << endl;
				count++;
				remain = 1;
				break;

			}
			
			  
		}
	}
	cout << repeat << endl;
	cout << count <<" 카운트개수" << endl;

	cout << repeat - count << endl;

	delete[] num_box;
}