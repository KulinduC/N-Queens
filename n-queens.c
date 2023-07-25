#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>



const char *str = "a";

void freeB(int **board, int m) {
  for (int i = 0; i < m; i++) {
    free(*(board + i));
  }
  free(board);
}

int max(int x, int y) { return (x > y) ? x : y; }


int canPlace(int **board, int row, int col, int m, int n) {
   int i,j;
   for (i = row; i >= 0; i--) {
        if (*(*(board + i) + col) == 1) return 0;
    }

    //upper left diagonal
    for (i = row-1,j=col-1; i>=0 && j>=0; i--,j--) {
        if (*(*(board + i) + j) == 1) return 0;
    }

    //upper right diagonal
    for (i = row-1,j=col+1; i>=0 && j<n; i--,j++) {
        if (*(*(board + i) + j) == 1) return 0;
    }
    return 1;
}
    
int sqp(int **board, int row, int m, int n, int *fd) {
    if (row == m) {
    #ifndef QUIET
        printf("P%d: Found a solution; notifying top-level parent\n", getpid());
    #endif
        write(*(fd + 1), "q", 1);
        return EXIT_SUCCESS;
    }
    int i = 0;
    int num = 0;
    while (i < n) {
        if (canPlace(board,row,i,m,n)) num++;
        i++;
    }

    if (num > 0) {
    #ifndef QUIET
        const char* proc = "process";
        const char* mov  = "move";

        if (num > 1) {
            proc = "processes"; 
            mov = "moves";
        }

        printf("P%d: %d possible %s at row #%d; creating %d child %s...\n",getpid(), num, mov, row, num, proc);
        fflush(stdout);
    #endif
    }
    else {
    #ifndef QUIET
        printf("P%d: Dead end at row #%d\n", getpid(), row);
        fflush(stdout);
    #endif
        char c = row + '0'; 
        write(*(fd + 1), &c, 1);
        return EXIT_FAILURE;
    }

    pid_t child;
    for (int i = 0; i < n; i++) {
        if (canPlace(board, row, i, m, n)) {
      
        child = fork();

        if (child < 0) {
            perror("ERROR: fork failed");
            exit(EXIT_FAILURE);
        } 
        else if (child == 0) {
            *(*(board + row) + i) = 1;
            sqp(board, row + 1, m, n, fd);


            freeB(board, m);
            close(*fd); //read
            close(*(fd + 1)); //write
            free(fd);
            exit(EXIT_SUCCESS);
        } 
        else { // Parent process
        #ifdef NO_PARALLEL
            int status;
            waitpid(-1, &status, 0);
        #endif
        }
    }
  }
  close(*(fd + 1)); 
  return EXIT_SUCCESS;
}


int main(int argc, char **argv) {
    if (argc != 3) {
        perror("ERROR: Invalid argument(s)\nUSAGE: hw2.out <m> <n>\n");
        return EXIT_FAILURE;
    }

    int m = atoi(*(argv + 1));
    int n = atoi(*(argv + 2));

    if (m < 0 || n < 0) {
        perror("ERROR: Invalid argument(s)\nUSAGE: hw2.out <m> <n>\n");
        return EXIT_FAILURE;
    }

    if (n < m) {
        int temp = m;
        m = n;
        n = temp;
    }

    if (m == 8 || m == 11) str = "an";

    int **board = (int **)calloc(m, sizeof(int *));
    for (int i = 0; i < m; i++) {
        *(board + i) = (int *)calloc(n, sizeof(int));
    }

    int *fd = calloc(2, sizeof(int));

    if (fd == NULL) {
        perror("ERROR: calloc() failed, aborting...\n");
        return EXIT_FAILURE;
    }

    if (pipe(fd) == -1) {
        perror("ERROR: pipe() failed");
        return EXIT_FAILURE;
    }

    pid_t p = fork();
    if (p == -1) {
        perror("ERROR: fork() failed");
        return EXIT_FAILURE;
    } 

    if (p == 0) {
        printf("P%d: Solving the (m,n)-Queens problem for %s %dx%d board\n",getpid(), str, m, n);
        fflush(stdout);

#ifdef QUIET
        const char* proc = "process";
        const char* mov  = "move";

        if (n > 1) {
            proc = "processes"; 
            mov = "moves";
        }

        printf("P%d: %d possible %s at row #0; creating %d child %s...\n",getpid(), n, mov, n, proc);
        fflush(stdout);
#endif
        sqp(board, 0, m, n, fd);
    }
    else {
        close(*(fd + 1)); 
        int rc;
        int sol = 0;
        int queens = 0;
        int fq = 0;

        char *buffer = calloc(129, sizeof(char));

        while ((rc = read(*fd, buffer, 128)) > 0) {
            *(buffer + rc) = '\0';
            for (char *c = buffer; *c != '\0'; c++) {
                if (*c == 'q') {
                    sol++;
                }
                else {
                    fq = *c - '0';
                    queens = max(fq, queens);
                }
            }
        }

        if (sol > 0) {
            printf("P%d: Search complete; found %d solutions for %s %dx%d board\n",getpid(), sol, str, m, n);
        } 
        else {
            printf("P%d: Search complete; only able to place %d Queens on %s %dx%d board\n",getpid(),queens, str, m, n);
        }

        close(*fd); 
        free(buffer);
    }
    free(fd);
    freeB(board,m);
    return EXIT_SUCCESS;
}