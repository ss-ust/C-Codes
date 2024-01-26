#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

sem_t semaphore;

int rows, cols;
int *col_totals = NULL;
int *matrix = NULL;

void* sumOfCols(void* arg) {

    int th_num = *(int*)arg;

    int sum = 0;

    for(int i = 0; i < rows; i++) {
        sum += matrix[(cols * i) + th_num];
    }

    col_totals[th_num] = sum;
    
    sem_post(&semaphore);

    free(arg);
}

void* sortAndPrint() {
    sem_wait(&semaphore);
    
    int swapped, tmp;
    
    for (int i = 0; i < cols - 1; i++) {
        swapped = 0;
        for (int j = 0; j < cols - i - 1; j++) {
            if (col_totals[j] > col_totals[j + 1]) {
                tmp = col_totals[j];
                col_totals[j] = col_totals[j + 1];
                col_totals[j + 1] = tmp;
                swapped = 1;
            }
        }
 
        if (swapped == 0)
            break;
    }

    for(int k = 0; k < cols; k++) {
        printf("%d", col_totals[k]);
        if(k < cols - 1)
            printf(" ");    
    }
}

int main() {

    scanf("%d%d", &rows, &cols);

    col_totals = (int*)malloc(cols * sizeof(int));
    matrix = (int*)malloc(((rows * cols)) * sizeof(int));

    for(int i = 0; i < rows*cols; i++) {
        scanf("%d", &matrix[i]);
    }

    pthread_t th[cols+1];
    sem_init(&semaphore, 0, -cols + 1); 
    /*
    The semaphore is initialized with a value of -cols + 1 to allow multiple threads
    calculating the sum of columns to run simultaneously without blocking. Each thread
    computes the sum of a different column in the matrix. As each thread completes its
    computation, the semaphore's value increases by 1. When all threads finish, the
    semaphore's value becomes 1, indicating that all column sums are computed. At this
    point, the thread responsible for ordering or processing the sums can proceed safely
    knowing that all required computations are complete.
    */

    for(int i = 0; i < cols + 1; i++) {
        if(i < cols) {
            int *a = (int*)malloc(sizeof(int));
            *a = i;
            if(pthread_create(&th[i], NULL, &sumOfCols, a) != 0) { // Create threads to calculate the sum of columns concurrently.
                perror("Error creating threads.");
                return 1;
            }
        }
        else {
            if(pthread_create(&th[i], NULL, &sortAndPrint, NULL) != 0) { // Create thread to order sum of columns.
                perror("Error creating threads.");
                return 1;
            }
        }
    }

    for(int i = 0; i < cols + 1; i++) {
        if(pthread_join(th[i], NULL) != 0) {
            perror("Error joining threads.");
            return 2;
        }
    }

    free(col_totals);
    free(matrix);

    sem_destroy(&semaphore);

    return 0;
}
