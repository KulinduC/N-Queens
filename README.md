# N-Queens
Solving n-queens problem using forking and recursion

Proper command line usage 
gcc -Wall -Werror n-queens.c 


./a.out n m

Where n is the number of rows on the chess board and m is the number of columns
Starts to fail after 10x10 because forking starts to lack the necessary resources to make a child process. 
