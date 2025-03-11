#include <stdio.h>
#include <stdlib.h>
#include "collatz.h"
#ifdef DYNAMIC
    #include <dlfcn.h>
#endif

int main() {
    int inputs_size = 4;
    int inputs[] = {6, 19, 27, 837799}; 
    int max_iter = 300;
    #ifdef DYNAMIC
        void *handle = dlopen("./libcollatz.so", RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "Library loading error: %s\n", dlerror());
            return 1;
        }

        int (*test_collatz_convergence)(int, int, int *) = (int (*)(int, int, int *)) dlsym(handle, "test_collatz_convergence");
        if (!test_collatz_convergence) {
            fprintf(stderr, "Function loading error: %s\n", dlerror());
            dlclose(handle);
            return 1;
        }
    #endif

    for (int i = 0; i < inputs_size; i++) {
        int input = inputs[i];
        int steps[max_iter];
        printf("Input number: %d\n", input);
        int output = test_collatz_convergence(input, max_iter, steps);
        
        if (output == 0) {
            printf("Collatz sequence did not converge within %d iterations.\n", max_iter);
            return 1;
        }
        else {
            printf("Collatz sequence reached 1 in %d steps.\nSequence: ", output);
            for (int i = 0; i <= output; i++) {
                printf("%d ", steps[i]);
            }
            printf("\n\n");
        }
    }
    #ifdef DYNAMIC
        dlclose(handle);
    #endif
    return 0;
}