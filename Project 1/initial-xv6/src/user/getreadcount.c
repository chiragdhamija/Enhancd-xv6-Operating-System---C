//https://gist.github.com/bridgesign/e932115f1d58c7e763e6e443500c6561
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"
int main(void)
{
    int read_c=getreadcount();
    printf("Read count till now is %d",read_c);
    exit(1);
}