#include <utils/check.h>

int test(int a) {
    return -1;
}

int main(int argc, char **argv) {
    DIE_IF_ERR(test(25));
    return 0;
}
