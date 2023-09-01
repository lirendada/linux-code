#include "poll_server.hpp"
#include <memory>
using namespace std;
using namespace poll_space;

static void Usage(const string& proc)
{
    cerr << "\nUsage:\n\t" << proc << " port\n\n"; 
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }

    unique_ptr<poll_server> svr(new poll_server(atoi(argv[1])));
    svr->init();
    svr->run();
    return 0;
}