#include "MakeAble.h"

MakeAble able;

int main()
{
    able.init();

    while(1)
    {
        DMESG("tick");
        able.sleep(500);
    }
}

