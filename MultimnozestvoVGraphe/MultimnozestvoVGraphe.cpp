#include <iostream>
#include <omp.h>
#include <vector>
#include <ctime>
#include <cmath>    
#include "mpi.h"

using namespace std;

#define FIRST_THREAD 0

vector <double> S;
vector <double> S1;
vector <double> S2;
vector <double> MIN;

int main(int argc, char** argv)
{
    int thread, thread_size, processor_name_length;
    int* thread_range;
    char* processor_name = new char[MPI_MAX_PROCESSOR_NAME * sizeof(char)];
    MPI_Status status;

    float test;
    int k;
    int n = 4;
    float min = 32000;
    float sum = 0;
    float sum1;

    // Инициализируем работу MPI
    MPI_Init(&argc, &argv);

    // Получаем имя физического процессора
    MPI_Get_processor_name(processor_name, &processor_name_length);

    // Получаем номер конкретного процесса на котором запущена программа
    MPI_Comm_rank(MPI_COMM_WORLD, &thread);

    // Получаем количество запущенных процессов
    MPI_Comm_size(MPI_COMM_WORLD, &thread_size);

    srand(time(NULL));

    if (thread == FIRST_THREAD)
    {


        S.resize(n);
        for (int i = 0; i < S.size(); i++)
        {
            S[i] = (rand() % 100) / (float)(rand() % 5 + 1);
            sum += S[i];
        }
        cout << "sum = " << sum << endl;
        cout << "---------------S---------------" << endl;
        for (int i = 0; i < S.size(); i++)
        {
            cout << S[i] << " ";
        }
        cout << endl << "-------------------------------" << endl;
        auto iter = S.begin();

        for (int to_thread = 1; to_thread < thread_size; to_thread++)
        {
            MPI_Send(&sum, 1, MPI_FLOAT, to_thread, 1, MPI_COMM_WORLD);
            MPI_Send(&S1, n, MPI_DOUBLE, to_thread, 1, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(&sum, 1, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&S1, n, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }

//#pragma omp parallel for private(i, j, k, sum1, S1) shared(S, MIN)
    //for (int i = 0; i < 1ull << S.size(); ++i)
    {

        k = 0;
        sum1 = 0;
        S1.clear();
        while (thread >= 1 << k)
        {
            if (thread & 1 << k)
            {
                sum1 = sum1 + S[k];
                S1.push_back(S[k]);
            }
            ++k;
        }
//#pragma omp critical
        {
            if (abs(sum1 - (sum - sum1)) < min)
            {
                MIN.clear();
                min = abs(sum1 - (sum - sum1));
                for (int i = 0; i < S1.size(); i++)
                {
                    MIN.push_back(S1[i]);
                }
                test = min;
            }
        }
        S1.clear();
    }
    cout << "Minimal defference: " << test << endl;
    cout << "---------------S1--------------" << endl;
    for (long long i = 0; i < MIN.size(); i++)
    {
        cout << MIN[i] << " ";
    }
    cout << endl << "-------------------------------";
    cout << endl << "---------------S2--------------" << endl;
    int x = 0;
    for (long long i = 0; i < S.size(); i++)
    {
        if (x < MIN.size())
        {
            if (S[i] == MIN[x])
            {
                x++;
                continue;
            }
        }
        cout << S[i] << " ";
    }
    cout << endl << "-------------------------------" << endl;

    MPI_Finalize();
}