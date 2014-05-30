#include <stdio.h>
#include "HTA.h"
#include "Debug.h"

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    int x = 100;

#if DEBUG
    printf("DEBUG is on. The program should abort for assertion check fail.\n");
    printf("DBG_LVL is set to %d\n", DBG_LVL);
#else
    printf("DEBUG is off. The program should exit with return value 0.\n");
#endif

    DBG(0, "debug log level 0\n");
    DBG(1, "debug log level 1\n");
    DBG(2, "debug log level 2\n");
    DBG(3, "debug log level 3\n");
    DBG(4, "debug log level 4\n");
    DBG(5, "debug log level 5\n");
    DBG(6, "debug log level 6\n");
    DBG(7, "debug log level 7\n");
    DBG(8, "debug log level 8\n");
    DBG(9, "debug log level 9\n");
    DBG(10, "debug log level is larger than 9!!\n");

    ASSERT(x != 100);

    return 0;
}

