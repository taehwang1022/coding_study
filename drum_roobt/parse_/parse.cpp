#include <iostream>
#include <Eigen/Dense>
#include <vector>
#include <cmath>

using namespace std;
using namespace Eigen;

void parseMatrix(Eigen::MatrixXd &measureMatrix, double timeStep = 0.05)
{
    int numCols = measureMatrix.cols();
    std::vector<Eigen::VectorXd> newRows;

    double prevTotalTime = 0.0;

    for (int i = 0; i < measureMatrix.rows(); ++i)
    {
        double duration = measureMatrix(i, 1);
        double endTime = measureMatrix(i, 8);
        int steps = static_cast<int>(round(duration / timeStep));
        double sumStep = (endTime-prevTotalTime) / steps;

        for (int s = 1; s <= steps; ++s)
        {
            Eigen::VectorXd row(numCols);
            row.setZero();

            row(0) = measureMatrix(i, 0);                 
            row(1) = timeStep;                              
            row(8) = prevTotalTime + s * sumStep;          

            if (s == steps)
            {
                for (int j = 2; j <= 7; ++j)
                {
                    row(j) = measureMatrix(i, j);
                }
            }

            newRows.push_back(row);
        }

        prevTotalTime = endTime;
    }

    Eigen::MatrixXd parsedMatrix(newRows.size(), numCols);
    for (size_t i = 0; i < newRows.size(); ++i)
    {
        parsedMatrix.row(i) = newRows[i];
    }

}

int main()
{
    // 초기 measureMatrix 생성
    Eigen::MatrixXd measureMatrix(3, 9);
    measureMatrix << 0, 0.6, 0, 0, 1, 1, 0, 1, 1.2,
         0, 0.6, 1, 1, 0, 0, 0, 0, 2.4,
         0, 0.6, 1, 1, 1, 1, 1, 1, 3.6;
    

    cout << "[Original Matrix]\n" << measureMatrix << "\n\n";

    // 파싱 함수 호출
    parseMatrix(measureMatrix);

    cout << "[Parsed Matrix]\n" << measureMatrix << "\n";

    return 0;
}
