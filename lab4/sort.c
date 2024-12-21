#include "sort.h"

int *sortBubble (int *arr, int left, int right) {
    int i, j;

    for (i = 0; i < 10 - 1; i++) {
        for (j = 0; j < 10 - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }

    return arr;
}

int *sortQuick (int *arr, int left, int right) {
    if (left < right) {
        int pivot = arr[right]; 
        int i = left - 1;      

        for (int j = left; j < right; j++) {
            if (arr[j] <= pivot) {
                i++;
                int temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }

        int temp = arr[i + 1];
        arr[i + 1] = arr[right];
        arr[right] = temp;

        int pi = i + 1;

        sortQuick(arr, left, pi - 1);
        sortQuick(arr, pi + 1, right);
    }
    return arr;
}