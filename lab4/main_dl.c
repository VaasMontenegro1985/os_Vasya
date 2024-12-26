#include <stdio.h>
#include <stdbool.h>
#include <dlfcn.h>

typedef int (*PrimeCountFunction)(int, int);
typedef int (*SortFunction)(int *, int, int);

int main () {
    void *libPrimeCountHandle = NULL;
    void *libSort = NULL;

    PrimeCountFunction primeCountFunction = NULL;
    SortFunction sortFunction = NULL;

    char *error;

    while (1) {
        int command, calcWay, a, b, res;
        bool fail = false;

        printf("Input command:\n");
        printf("0 - load libraties\n");
        printf("1 - count primes\n");
        printf("2 - sort\n");
        printf("-1 - exit\n");

        scanf("%d", &command);

        if (command == -1) break;

        switch (command) {
            case 0:
                if (libPrimeCountHandle) dlclose(libPrimeCountHandle);
                if (libSort) dlclose(libSort);

                char primeCountLibName[50], sortLibName[50];
                printf("Input prime count library name: ");
                scanf("%50s", primeCountLibName);

                libPrimeCountHandle = dlopen(primeCountLibName, RTLD_LAZY);
                if (!libPrimeCountHandle) {
                    fprintf(stderr, "Error loading %s: %s\n", primeCountLibName, dlerror());
                    break;
                }

                printf("Input sort library name: ");
                scanf("%50s", sortLibName);

                libSort = dlopen(sortLibName, RTLD_LAZY);
                if (!libSort) {
                    fprintf(stderr, "Error loading %s: %s\n", sortLibName, dlerror());
                    dlclose(libPrimeCountHandle);
                    break;
                }

                printf("Libraries loaded\n");
                break;

            case 1:
                printf("Input segment borders: ");
                scanf("%d %d", &a, &b);

                printf("Input way to count primes:\n");
                printf("1 - naive\n");
                printf("2 - Eratosthenes' sieve\n");
                scanf("%d", &calcWay);

                switch (calcWay) {
                    case 1:
                        primeCountFunction = (PrimeCountFunction)dlsym(libPrimeCountHandle, "primeCountNaive");
                        break;

                    case 2:
                        primeCountFunction = (PrimeCountFunction)dlsym(libPrimeCountHandle, "primeCountEratosthenes");
                        break;
                    
                    default:
                        printf("Unknown way\n");
                        fail = true;
                }

                if ((error = dlerror()) != NULL) {
                    fprintf(stderr, "Function loading error: %s\n", error);
                    break;
                }

                if (!fail) {
                    res = primeCountFunction(a, b);
                    printf("%d prime numbers\n", res);
                } 

                break;

            case 2:
                printf("Input numbers\n");

                int arr[10];
                
                for (int i = 0; i < 10; i++) scanf("%d", arr + i);

                printf("Input way to sort:\n");
                printf("1 - bubble\n");
                printf("2 - quick\n");
                scanf("%d", &calcWay);

                switch (calcWay) {
                    case 1:
                        sortFunction = (SortFunction)dlsym(libSort, "sortBubble");
                        break;

                    case 2:
                        sortFunction = (SortFunction)dlsym(libSort, "sortQuick");
                        break;
                    
                    default:
                        printf("Unknown way\n");
                        fail = true;
                }

                if ((error = dlerror()) != NULL) {
                    fprintf(stderr, "Function loading error: %s\n", error);
                    break;
                }

                if (!fail) {
                    sortFunction(arr, 0, 9);
                    for (int i = 0; i < 10; i++) printf("%d ", arr[i]);
                    printf("\n");
                } 

                break;
            
            default:
                printf("Unknown command\n");
                break;
        }

        printf("\n");
    }

    if (libPrimeCountHandle) dlclose(libPrimeCountHandle);
    if (libSort) dlclose(libSort);
    
    printf("exit\n");

    return 0;
}
