#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int main(int argc, char* argv[])
{
    int pid=atoi(argv[1]);
    int new_number=atoi(argv[2]);
    int returned_value=set_priority(pid,new_number);
    if(returned_value<0)
    {
        printf("Error : could not change priority\n");
        exit(0);
    }
    return 0;
}