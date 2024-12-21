#include <stdio.h>
#include <stdbool.h>
#include "primecount.h"
#include "sort.h"

int main () {
    while (1) {
        int command, calcWay, a, b, res;
        bool fail = false;

        printf("Input command:\n");
        printf("1 - count primes\n");
        printf("2 - sort\n");
        printf("-1 - exit\n");

        scanf("%d", &command);

        if (command == -1) break;

        switch (command) {
            case 1:
                printf("Input segment borders: ");
                scanf("%d %d", &a, &b);

                printf("Input way to count primes:\n");
                printf("1 - naive\n");
                printf("2 - Eratosthenes' sieve\n");
                scanf("%d", &calcWay);

                switch (calcWay) {
                    case 1:
                        res = primeCountNaive(a, b);
                        break;

                    case 2:
                        res = primeCountEratosthenes(a, b);
                        break;
                    
                    default:
                        printf("Unknown way\n");
                        fail = true;
                }

                if (!fail) printf("%d prime numbers\n", res);

                break;

            case 2:
                printf("Input numbers\n");

                int arr[10];
                
                for (int i = 0; i < 10; i++) scanf("%d", arr + i);

                printf("Input way to sort:\n");
                printf("1 - Bubble \n");
                printf("2 - Quick\n");
                scanf("%d", &calcWay);

                switch (calcWay) {
                    case 1:
                        sortBubble(arr, 0, 9);
                        break;

                    case 2:
                        sortQuick(arr, 0, 9);
                        break;
                    
                    default:
                        printf("Unknown way\n");
                        fail = true;
                }

                if (!fail) for (int i = 0; i < 10; i++) printf("%d ", arr[i]);
                printf("\n");

                break;
            
            default:
                printf("Unknown command\n");
                break;
        }

        printf("\n");
    }
    
    printf("exit\n");

    return 0;
}
