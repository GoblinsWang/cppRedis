
#include <iostream>
#include <chrono>
#include <ctime>
#include <thread>
using namespace std;
/*
 * 打印耗时，取变量构造函数与析构函数的时间差，单位ms
 */
class SpendTime
{
public:
    SpendTime() : _curTimePoint(std::move(std::chrono::steady_clock::now()))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }

    ~SpendTime()
    {
        auto curTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(curTime - _curTimePoint);
        cout << "SpendTime = " << duration.count() << "s" << endl;
    }

private:
    std::chrono::steady_clock::time_point _curTimePoint;
};

int main()
{
    SpendTime st;
}