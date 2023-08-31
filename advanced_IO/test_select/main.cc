#include "select_server.hpp"
#include <memory>
using namespace std;
using namespace select_space;

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

    unique_ptr<select_server> svr(new select_server(atoi(argv[1])));
    svr->init();
    svr->run();
    return 0;
}