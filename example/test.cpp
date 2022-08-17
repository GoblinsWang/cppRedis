#include "redis_client.h"

using namespace std;

void do_test();

void doTest();

int main()
{
    RedisClient client("127.0.0.1", 6379, 2, 10);
    std::string value;
    client.get("stest1", value);
    cout << value << endl;
    return 0;
}