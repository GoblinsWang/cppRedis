#include "redis_client.h"
using namespace cppredis;

int main()
{
    RedisClient client("127.1", 6379, 2, 10); // 172.23.128.1
    std::string value;
    client.get("stest1", value);
    client.get("stest1", value);
    std::cout << value << std::endl;
    return 0;
}