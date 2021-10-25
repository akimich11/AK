#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <mpi.h>

#define MINSIZE 10  // threshold for switching to bubbleSort
#define TAG 1001

// Bubble sort for the base cases
//
void bubbleSort(int *array, int low, int high) {
    if (low >= high)
        return;
    for (int i = low; i <= high; i++)
        for (int j = i + 1; j <= high; j++)
            if (array[i] > array[j])
                std::swap(array[i], array[j]);
}

// Pick an arbitrary element as pivot. Rearrange array
// elements into [smaller one, pivot, larger ones].
// Return pivot's index.
//
int partition(int *array, int low, int high) {
    int pivot = array[high];    // use highest element as pivot
    int middle = low;
    for (int i = low; i < high; i++)
        if (array[i] < pivot) {
            std::swap(array[i], array[middle]);
            middle++;
        }
    std::swap(array[high], array[middle]);
    return middle;
}

// QuickSort an array range
//
void quickSort(int *array, int low, int high) {
    if (high - low < MINSIZE) {
        bubbleSort(array, low, high);
        return;
    }
    int middle = partition(array, low, high);
    if (low < middle)
        quickSort(array, low, middle - 1);
    if (middle < high)
        quickSort(array, middle + 1, high);
}


int setStep(int num1, int num2) {
    int x = num1 / num2;
    double y = (double) num1 / (double) num2;

    //round down
    if (y - x < .5)
        return num1 / num2;

    else
        return (num1 / num2) + 1;
}


void fillBuff(int *buff, const int *data, int num, int start) {
    int k = 0;
    for (int i = start; k < num; i++) {
        buff[k] = data[i];
        k++;
    }
}


// Main routine for testing quickSort
//
int main(int argc, char **argv) {
    int j, P, rank, N, *data, *par, *bond, *myBucket, *tempBuff, *receive, total, k, start, current;
    MPI_File fh_in, fh_out;
    MPI_Status st;
    // MPI_Request req;
    char host[20];


    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &P);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    gethostname(host, 20);
    printf("Rank: %d is starting on %s\n", rank, host);


    // open file for read
    MPI_File_open(MPI_COMM_WORLD, "input", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh_in);

    //Step 1

    if (rank == 0) {
        MPI_Offset temp = 0;
        MPI_File_get_size(fh_in, &temp);
        N = int(temp) / 4;
        printf("Master: Sending sequence size to MPI-Processes\n");
    }
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printf("rank: %d, N: %d, P: %d\n", rank, N, P);

    if (N % P != 0 || P * P > N) {
        printf("Rank: %d is ending on %s data does not meet requirements\n", rank, host);
        MPI_Finalize();
        return 0;
    }


    //Step 2

    data = new int[N / P];
    // set starting offset for the read operation
    MPI_File_set_view(fh_in, rank * N / P * 4, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
    // read two integers from the file
    MPI_File_read(fh_in, data, N / P, MPI_INT, &st);

    //Step 3

    quickSort(data, 0, N / P - 1);

    //Step 4

    par = new int[P];
    par[0] = N + 1;

    if (rank == 0) {
        int step = setStep((N / P), P);
        printf("step: %d\n", step);
        int s = 0;
        for (int i = 0; s < P - 1; i++) {
            if ((i + 1) % step == 0) {
                par[s] = data[i];
                printf("par[%d]: %d\n", s, par[s]);
                s++;
            }
        }
    }
    MPI_Bcast(par, P - 1, MPI_INT, 0, MPI_COMM_WORLD);

    //Step 5

    bond = new int[P];
    bond[0] = N / P - 1;
    j = 0;

    for (int i = 0; i < N / P && j < P - 1; i++) {

        if (par[j] < data[i]) {
            bond[j] = i - 1;
            i--;
            j++;
        } else if (par[j] == data[i]) {
            bond[j] = i;
            i--;
            j++;
        }
        //fill the rest
        if (i == N / P - 1) {
            for (; j < P - 1; j++) {
                bond[j] = i;
            }

        }
    }


    //print out debugging data
    printf("bucket size: %d\n", (P / 2 + 1) * (N / P));

    int l = 0;
    printf("Rank: %d: ", rank);
    for (int i = 0; i < N / P || l < P - 1; i++) {

        while (l < P - 1 && bond[l] == i - 1) {
            printf(" | ");
            l++;
        }
        if (i < N / P)
            printf("%d ", data[i]);
    }
    printf("\n");


    //Step 6

    myBucket = new int[(P / 2 + 1) * (N / P)];

    k = 0;//temp to hold the number of things to send to the other threads
    total = 0; //temp to hold the number of things to send to the other threads
    start = 0;
    current = 0; //variable for adding elements to myBucket

    //send the number elements that you will be sending to the specific process
    for (int i = 0; i < P; i++) {
        //Calculate the amount to send to the ith thread
        //printf("rank: %d bond%d: %d", rank, i, bond[i]);
        if (i < P - 1 && i > 0) {
            k = bond[i] - bond[i - 1];
        } else if (i < P - 1 && i == 0) {
            k = bond[i] + 1;
        } else {
            k = N / P - total;
        }

        total += k;

        printf("Sending: %d from rank: %d\n", k, rank);


        //send the number of elements (k) to the respective thread (i)
        MPI_Send(&k, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);

        //dont send if empty
        if (k > 0) {
            tempBuff = new int[k]; //create a temp buffer to send
            //prep the data to be sent over
            fillBuff(tempBuff, data, k, start);
            //Send the data
            MPI_Send(tempBuff, k, MPI_INT, i, TAG, MPI_COMM_WORLD);
            free(tempBuff);//deallocate the temp buffer
        }


        start = bond[i] + 1;
    }

    for (int x = 0; x < P; x++) {
        j = 0;
        //receive the number of elements to receive
        MPI_Recv(&j, 1, MPI_INT, x, TAG, MPI_COMM_WORLD, &st);

        //dont receive if empty
        if (j > 0) {
            receive = new int[j]; //allocate space for the elements to be received
            //receive the data
            MPI_Recv(receive, j, MPI_INT, x, TAG, MPI_COMM_WORLD, &st);

            //transfer the new items into the myBucket array
            for (int t = 0; t < j; t++) {
                myBucket[current] = receive[t];
                printf("adding: rank= %d, myBucket=%d, received=%d, current=%d\n", rank, myBucket[current], receive[t],
                       current);
                current++;
            }
            free(receive);
        }
    }


    //Step 7

    quickSort(myBucket, 0, current - 1);

    //step 8



    int currentOffset = 0;

    if (rank == 0) {
        MPI_File_open(MPI_COMM_SELF, "output", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh_out);
        MPI_File_set_view(fh_out, currentOffset * 4, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
        MPI_File_write(fh_out, myBucket, current, MPI_INT, &st);
        currentOffset += current;
        if (rank < P - 1)
            MPI_Send(&currentOffset, 1, MPI_INT, rank + 1, TAG, MPI_COMM_WORLD);
    } else {
        MPI_Recv(&currentOffset, 1, MPI_INT, rank - 1, TAG, MPI_COMM_WORLD, &st);
        MPI_File_open(MPI_COMM_SELF, "output", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh_out);
        MPI_File_set_view(fh_out, currentOffset * 4, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
        MPI_File_write(fh_out, myBucket, current, MPI_INT, &st);
        currentOffset += current;
        if (rank < P - 1)
            MPI_Send(&currentOffset, 1, MPI_INT, rank + 1, TAG, MPI_COMM_WORLD);
    }


    for (int i = 0; i < current; i++) {
        printf("rank=%d: %d\n", rank, myBucket[i]);
    }


    printf("Rank: %d is ending on %s\n", rank, host);
    free(myBucket);
    free(bond);
    free(par);
    free(data);

    MPI_File_close(&fh_in);
    MPI_File_close(&fh_out);

    MPI_Finalize();

//re add in lepton
    return 0;


}
