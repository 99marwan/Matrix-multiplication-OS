#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

//first matrix
int **mat1;
//second matrix
int **mat2;
//output
int **matOut;
// # of rows in first matrix
int row1;
// # of cols in first matrix
int col1;
// # of rows in second matrix
int row2;
// # of cols in second matrix
int col2;
// # of threads in first method
int counter1;
// # of threads in second method
int counter2;
// # of threads in third method
int counter3;
//pointer to c.out
FILE *out;

struct timeval stop, start;

//parameters of third method
struct parameters {
    int i;
    int j;
} ;
//method 1
void * call0(){
    //matrix multiplication
    for (int i = 0; i < row1; i++) {
        for (int j = 0; j < col2; j++) {
            int temp = 0;
            for (int k = 0; k < col1; k++) {
                temp+=mat1[i][k]*mat2[k][j];
            }
            matOut[i][j]=temp;
        }
    }
}
//method2
void * call1(void *msg){
    long i;
    i = (long )msg;
    //specific row multiplication by second matrix
    for (int j = 0; j < col2; ++j) {
        int temp = 0;
        for (int k = 0; k < col1; ++k) {
            temp+=mat1[i][k]*mat2[k][j];
        }
        matOut[i][j]=temp;

    }
    //kill thread
    pthread_exit(NULL);
}
//method 3
void *call2(void *msg){
    struct parameters *args = (struct parameters *) msg;
    int i =args->i;
    int j=args->j;
    int temp = 0;
    //specific row multiplication by specific col
    for (int k = 0; k < col1; ++k) {
        temp+=mat1[i][k]*mat2[k][j];
    }
    matOut[i][j] = temp;
    //free memory allocation of the struct
    free (args);
    //kill thread
    pthread_exit (NULL);
}
//read matrix from file
void getMatrix(FILE *fp,int **arr,int row,int col){

    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            int number;
            fscanf(fp, "%d", &number);
            arr[i][j] = number;
        }
    }

}
//memory allocation of first matrix after get size
void allocateMat1(){
    mat1 = (int **)malloc(row1 * sizeof(int *));
    for (int i=0; i<row1; i++)
        mat1[i] = (int *)malloc(col1 * sizeof(int));
}
//memory allocation of second matrix after get size
void allocateMate2(){
    mat2 = (int **)malloc(row2 * sizeof(int *));
    for (int i=0; i<row2; i++)
        mat2[i] = (int *)malloc(col2 * sizeof(int));
}
//memory allocation of output matrix after get size
void allocateMatOut(){
    matOut = (int **)malloc(row1 * sizeof(int *));
    for (int i=0; i<row1; i++)
        matOut[i] = (int *)malloc(col2 * sizeof(int));
}
//write the output of each method in file
void writeOutput(int counter){
    for (int i = 0; i < row1; i++) {
        for (int j = 0; j < col2; j++) {
            fprintf(out,"%d\t",matOut[i][j]);
        }
        fprintf(out,"\n");
    }
    fprintf(out,"\n");
    fprintf(out,"Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    fprintf(out,"Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    fprintf(out,"Number of Threads: %d\n\n\n", counter);
}
//clear the output matrix and make its elements
//equal zero
void clearMatOut(){
    for (int i = 0; i < row1; i++) {
        for (int j = 0; j < col2; j++) {
            matOut[i][j]=0;
        }
    }
}

void main(int argc, char *argv[] )  {
    //array of files
    char *files[3];
    //if no args passed
    if(argc < 2){
        //default files
        files[0]="a.txt";
        files[1]="b.txt";
        files[2]="c.out";
    }
    else if(argc == 2){
        files[0]=argv[1];
        files[1]="b.txt";
        files[2]="c.out";
    }
    else if(argc == 3){
        files[0]=argv[1];
        files[1]=argv[2];
        files[2]="c.out";
    }
    else{
        files[0]=argv[1];
        files[1]=argv[2];
        files[2]=argv[3];
    }

    //open first matrix file
    FILE *fp1;
    fp1 = fopen(files[0], "r");
    if (fp1 == NULL){
        printf("Could not open file %s\n",files[0]);
        exit(-1);
    }
    //get the value of rows and cols
    fscanf(fp1, "row=%d col=%d", &row1, &col1);
    //memory allocation of first matrix after get size
    allocateMat1();
    //read matrix
    getMatrix(fp1,mat1,row1,col1);
    fclose(fp1);

    //open first matrix file
    FILE *fp2;

    fp2 = fopen(files[1], "r");
    if (fp2 == NULL){
        printf("Could not open file %s\n",files[1]);
        exit(-1);
    }
    //get the value of rows and cols
    fscanf(fp2, "row=%d col=%d", &row2, &col2);
    //memory allocation of second matrix after get size
    allocateMate2();
    //read matrix
    getMatrix(fp2,mat2,row2,col2);
    fclose(fp2);
    //check dimension validity
    if (col1 != row2){
        printf("Could not do multiplication operation because of dimensions\n");
        exit(-1);
    }

    //memory allocation of output matrix after get size
    allocateMatOut();


    gettimeofday(&start, NULL); //start checking time
    //first method call
    call0();
    counter1++;
    gettimeofday(&stop, NULL); //end checking time

    //open output file
    out = fopen(files[2],"w");
    if (out == NULL)
    {
        printf("Could not open file %s\n",files[2]);
        exit(-1);
    }
    //write results in file
    writeOutput(counter1);

    clearMatOut();

    gettimeofday(&start, NULL); //start checking time
    //second method
    pthread_t t[row1];
    for (long i = 0; i < row1; ++i) {
        int rc = pthread_create(&t[i],NULL,&call1,(void *) i);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
        counter2++;
    }

    //joining its threads
    for (int i = 0; i < row1; ++i) {
        pthread_join(t[i],NULL);
    }
    gettimeofday(&stop, NULL); //end checking time
    //write results in file
    writeOutput(counter2);

    clearMatOut();

    gettimeofday(&start, NULL); //start checking time
    //method 3
    pthread_t t3[row1][col2];
    for (int i = 0; i < row1; ++i) {
        for (int j = 0; j < col2; ++j) {
            struct parameters *args = malloc (sizeof (struct parameters));
            args->i = i;
            args->j = j;
            int rc = pthread_create(&t3[i][j],NULL,&call2,(void *)args);
            if (rc){
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
            counter3++;
        }

    }
    //joining its threads
    for (int i = 0; i < row1; ++i) {
        for (int j = 0; j < col2; ++j) {
            pthread_join(t3[i][j],NULL);
        }

    }
    gettimeofday(&stop, NULL); //end checking time
    //write results in file
    writeOutput(counter3);
    fclose(out);

    //free allocation
    free(mat1);
    free(mat2);
    free(matOut);

}

