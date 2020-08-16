#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


#define Abs(x)                ( (x)>0.0?  (x) : (-(x)) ) 
#define SIGN(u,v)              ( (v)>=0.0 ? Abs(u) : -Abs(u) )
#define MAX(a,b) (((a)>(b))?(a):(b))

static volatile int aliveChildren = 4;


void** p1_arrays;	// These arrays are used to back-up addressess.
void** p2_arrays;	// So we can free them, when SIGINT signal comes, 
void** p3_arrays;
void** p4_arrays;
void** p5_arrays;
int fd_array[2];	// open files
char* Rfilename; 
char* Rfilename2; 
// To understand where the program is cut
int Rcount = 0, count1 = 0, count2 = 0, count3 = 0, count4 = 0, count5 = 0, fd_count = 0;	

// Free resources when interrupt happens
void cleanResources(int _count)
{
	for (int i = 0; i < _count; ++i)
	{
		free(p1_arrays[i]);
	}
}

// SIGCHLD handler
void handler(int signo)
{	
	int stat;
	while(waitpid(-1,&stat,WNOHANG) > 0){
		--aliveChildren;
	}
}

// SIGINT Handler for P1
void handler2(int signo)
{	
	cleanResources(count1);
	if (Rcount == 1)
	{
		free(Rfilename);
	}

	if (Rcount == 2)
	{
		free(Rfilename);
		free(Rfilename2);
	}
	
	if (fd_count == 1)
	{	
		close(fd_array[0]);
	}

	if (fd_count == 2)
	{
		close(fd_array[0]);
		close(fd_array[1]);
	}

	while(waitpid(-1,NULL,WNOHANG) > 0);
	exit(1);
}

// SIGINT Handler for P2
void handler3(int signo)
{	
	if (count2 == 1)
		free(p2_arrays[0]);
	if(count2 == 2){
		free(p2_arrays[0]);
		free(p2_arrays[1]);
	}

	exit(1);
}

// SIGINT Handler for P3
void handler4(int signo)
{	
	if (count3 == 1)
		free(p3_arrays[0]);
	if(count3 == 2){
		free(p3_arrays[0]);
		free(p3_arrays[1]);
	}

	exit(1);
}

// SIGINT Handler for P4
void handler5(int signo)
{	
	if (count4 == 1)
		free(p4_arrays[0]);
	if(count4 == 2){
		free(p4_arrays[0]);
		free(p4_arrays[1]);
	}

	exit(1);
}

// SIGINT Handler for P5
void handler6(int signo)
{	
	if (count5 == 1)
		free(p5_arrays[0]);
	if(count5 == 2){
		free(p5_arrays[0]);
		free(p5_arrays[1]);
	}

	exit(1);
}




/* 
 * svdcomp - SVD decomposition routine. 
 * Takes an mxn matrix a and decomposes it into udv, where u,v are
 * left and right orthogonal transformation matrices, and d is a 
 * diagonal matrix of singular values.
 *
 * This routine is adapted from svdecomp.c in XLISP-STAT 2.1 which is 
 * code from Numerical Recipes adapted by Luke Tierney and David Betz.
 *
 * Input to dsvd is as follows:
 *   a = mxn matrix to be decomposed, gets overwritten with u
 *   m = row dimension of a
 *   n = column dimension of a
 *   w = returns the vector of singular values of a
 *   v = returns the right orthogonal transformation matrix
*/
 
static double PYTHAG(double a, double b)
{
    double at = fabs(a), bt = fabs(b), ct, result;

    if (at > bt)       { ct = bt / at; result = at * sqrt(1.0 + ct * ct); }
    else if (bt > 0.0) { ct = at / bt; result = bt * sqrt(1.0 + ct * ct); }
    else result = 0.0;
    return(result);
}


int dsvd(float **a, int m, int n, float *w, float **v)
{
    int flag, i, its, j, jj, k, l, nm;
    double c, f, h, s, x, y, z;
    double anorm = 0.0, g = 0.0, scale = 0.0;
    double *rv1;
  
    if (m < n) 
    {
        fprintf(stderr, "#rows must be > #cols \n");
        return(0);
    }
  
    rv1 = (double *)malloc((unsigned int) n*sizeof(double));

/* Householder reduction to bidiagonal form */
    for (i = 0; i < n; i++) 
    {
        /* left-hand reduction */
        l = i + 1;
        rv1[i] = scale * g;
        g = s = scale = 0.0;
        if (i < m) 
        {
            for (k = i; k < m; k++) 
                scale += fabs((double)a[k][i]);
            if (scale) 
            {
                for (k = i; k < m; k++) 
                {
                    a[k][i] = (float)((double)a[k][i]/scale);
                    s += ((double)a[k][i] * (double)a[k][i]);
                }
                f = (double)a[i][i];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                a[i][i] = (float)(f - g);
                if (i != n - 1) 
                {
                    for (j = l; j < n; j++) 
                    {
                        for (s = 0.0, k = i; k < m; k++) 
                            s += ((double)a[k][i] * (double)a[k][j]);
                        f = s / h;
                        for (k = i; k < m; k++) 
                            a[k][j] += (float)(f * (double)a[k][i]);
                    }
                }
                for (k = i; k < m; k++) 
                    a[k][i] = (float)((double)a[k][i]*scale);
            }
        }
        w[i] = (float)(scale * g);
    
        /* right-hand reduction */
        g = s = scale = 0.0;
        if (i < m && i != n - 1) 
        {
            for (k = l; k < n; k++) 
                scale += fabs((double)a[i][k]);
            if (scale) 
            {
                for (k = l; k < n; k++) 
                {
                    a[i][k] = (float)((double)a[i][k]/scale);
                    s += ((double)a[i][k] * (double)a[i][k]);
                }
                f = (double)a[i][l];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                a[i][l] = (float)(f - g);
                for (k = l; k < n; k++) 
                    rv1[k] = (double)a[i][k] / h;
                if (i != m - 1) 
                {
                    for (j = l; j < m; j++) 
                    {
                        for (s = 0.0, k = l; k < n; k++) 
                            s += ((double)a[j][k] * (double)a[i][k]);
                        for (k = l; k < n; k++) 
                            a[j][k] += (float)(s * rv1[k]);
                    }
                }
                for (k = l; k < n; k++) 
                    a[i][k] = (float)((double)a[i][k]*scale);
            }
        }
        anorm = MAX(anorm, (fabs((double)w[i]) + fabs(rv1[i])));
    }
  
    /* accumulate the right-hand transformation */
    for (i = n - 1; i >= 0; i--) 
    {
        if (i < n - 1) 
        {
            if (g) 
            {
                for (j = l; j < n; j++)
                    v[j][i] = (float)(((double)a[i][j] / (double)a[i][l]) / g);
                    /* double division to avoid underflow */
                for (j = l; j < n; j++) 
                {
                    for (s = 0.0, k = l; k < n; k++) 
                        s += ((double)a[i][k] * (double)v[k][j]);
                    for (k = l; k < n; k++) 
                        v[k][j] += (float)(s * (double)v[k][i]);
                }
            }
            for (j = l; j < n; j++) 
                v[i][j] = v[j][i] = 0.0;
        }
        v[i][i] = 1.0;
        g = rv1[i];
        l = i;
    }
  
    /* accumulate the left-hand transformation */
    for (i = n - 1; i >= 0; i--) 
    {
        l = i + 1;
        g = (double)w[i];
        if (i < n - 1) 
            for (j = l; j < n; j++) 
                a[i][j] = 0.0;
        if (g) 
        {
            g = 1.0 / g;
            if (i != n - 1) 
            {
                for (j = l; j < n; j++) 
                {
                    for (s = 0.0, k = l; k < m; k++) 
                        s += ((double)a[k][i] * (double)a[k][j]);
                    f = (s / (double)a[i][i]) * g;
                    for (k = i; k < m; k++) 
                        a[k][j] += (float)(f * (double)a[k][i]);
                }
            }
            for (j = i; j < m; j++) 
                a[j][i] = (float)((double)a[j][i]*g);
        }
        else 
        {
            for (j = i; j < m; j++) 
                a[j][i] = 0.0;
        }
        ++a[i][i];
    }

    /* diagonalize the bidiagonal form */
    for (k = n - 1; k >= 0; k--) 
    {                             /* loop over singular values */
        for (its = 0; its < 30; its++) 
        {                         /* loop over allowed iterations */
            flag = 1;
            for (l = k; l >= 0; l--) 
            {                     /* test for splitting */
                nm = l - 1;
                if (fabs(rv1[l]) + anorm == anorm) 
                {
                    flag = 0;
                    break;
                }
                if (fabs((double)w[nm]) + anorm == anorm) 
                    break;
            }
            if (flag) 
            {
                c = 0.0;
                s = 1.0;
                for (i = l; i <= k; i++) 
                {
                    f = s * rv1[i];
                    if (fabs(f) + anorm != anorm) 
                    {
                        g = (double)w[i];
                        h = PYTHAG(f, g);
                        w[i] = (float)h; 
                        h = 1.0 / h;
                        c = g * h;
                        s = (- f * h);
                        for (j = 0; j < m; j++) 
                        {
                            y = (double)a[j][nm];
                            z = (double)a[j][i];
                            a[j][nm] = (float)(y * c + z * s);
                            a[j][i] = (float)(z * c - y * s);
                        }
                    }
                }
            }
            z = (double)w[k];
            if (l == k) 
            {                  /* convergence */
                if (z < 0.0) 
                {              /* make singular value nonnegative */
                    w[k] = (float)(-z);
                    for (j = 0; j < n; j++) 
                        v[j][k] = (-v[j][k]);
                }
                break;
            }
            if (its >= 30) {
                free((void*) rv1);
                fprintf(stderr, "No convergence after 30,000! iterations \n");
                return(0);
            }
    
            /* shift from bottom 2 x 2 minor */
            x = (double)w[l];
            nm = k - 1;
            y = (double)w[nm];
            g = rv1[nm];
            h = rv1[k];
            f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
            g = PYTHAG(f, 1.0);
            f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
          
            /* next QR transformation */
            c = s = 1.0;
            for (j = l; j <= nm; j++) 
            {
                i = j + 1;
                g = rv1[i];
                y = (double)w[i];
                h = s * g;
                g = c * g;
                z = PYTHAG(f, h);
                rv1[j] = z;
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = g * c - x * s;
                h = y * s;
                y = y * c;
                for (jj = 0; jj < n; jj++) 
                {
                    x = (double)v[jj][j];
                    z = (double)v[jj][i];
                    v[jj][j] = (float)(x * c + z * s);
                    v[jj][i] = (float)(z * c - x * s);
                }
                z = PYTHAG(f, h);
                w[j] = (float)z;
                if (z) 
                {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = (c * g) + (s * y);
                x = (c * y) - (s * g);
                for (jj = 0; jj < m; jj++) 
                {
                    y = (double)a[jj][j];
                    z = (double)a[jj][i];
                    a[jj][j] = (float)(y * c + z * s);
                    a[jj][i] = (float)(z * c - y * s);
                }
            }
            rv1[l] = 0.0;
            rv1[k] = f;
            w[k] = (float)x;
        }
    }
    free((void*) rv1);
    return(1);
}


/*
* Reads inputs from the files and calculates the product matrix and its singular values.
*/
void readFromFile(char* rFile, char* rFile2, int _n)
{	
	sigset_t newMask;
	sigfillset(&newMask);
	sigdelset(&newMask,SIGCHLD);

	int fd1, fd2, totalSize = pow(2,_n)*pow(2,_n), size = 0, i, j;

	char* buf1 = (char*) malloc(sizeof(char)*totalSize);
	p1_arrays[count1] = buf1;
	++count1;

	char* buf2 = (char*) malloc(sizeof(char)*totalSize);
	p1_arrays[count1] = buf2;
	++count1;


	// Open the input file in readonly mode
	fd1 = open(rFile,O_RDONLY);	
	fd_array[fd_count] = fd1;
	++fd_count;	
	if (fd1 == -1)
	{	
		// If an error is occured, exit gracefully
		perror("Failed to open input file");
		free(buf1);
		free(buf2);
		exit(1);
	}

	// Open the input2 file in readonly mode
	fd2 = open(rFile2,O_RDONLY);
	fd_array[fd_count] = fd2;	
	++fd_count;	
	if (fd2 == -1)
	{	
		// If an error is occured, exit gracefully
		perror("Failed to open input2 file");
		free(buf1);
		free(buf2);
		close(fd1);
		exit(1);
	}


	// Content of A
	while (size != totalSize)					
	{
		size = read(fd1,buf1,totalSize);
		if (size == -1)
		{	// If an error is occured, exit gracefully
			perror("Failed to read input file");
			free(buf1);
			free(buf2);
			close(fd1);
			close(fd2);
			exit(1);
		}

		// If file is not sufficient, print message and exit
		if (size < totalSize)
		{
			printf("Error : There are not sufficient characters in input1 file\n");
			free(buf1);
			free(buf2);
			close(fd1);
			close(fd2);
			exit(1);
		}
	}

	// Content of B
	size = 0; 
	while (size != totalSize)					
	{	// If an error is occured, exit gracefully
		size = read(fd2,buf2,totalSize);
		if (size == -1)
		{
			perror("Failed to read input2 file");
			free(buf1);
			free(buf2);
			close(fd1);
			close(fd2);
			exit(1);
		}

		// If file is not sufficient, print message and exit
		if (size < totalSize)
		{
			printf("Error : There are not sufficient characters in input2 file\n");
			free(buf1);
			free(buf2);
			close(fd1);
			close(fd2);
			exit(1);
		}
	}

	
	// n x n matrix
	int n = pow(2,_n);
	char matrixA[n][n];
	char matrixB[n][n];

	char* temp1 = (char*) malloc(sizeof(char)*(n*n/4*n*2));
	p1_arrays[count1] = temp1;
	++count1;

	char* temp2 = (char*) malloc(sizeof(char)*(n*n/4*n*2));
	p1_arrays[count1] = temp2;
	++count1;

	char* temp3 = (char*) malloc(sizeof(char)*(n*n/4*n*2));
	p1_arrays[count1] = temp3;
	++count1;

	char* temp4 = (char*) malloc(sizeof(char)*(n*n/4*n*2));
	p1_arrays[count1] = temp4;
	++count1;

	// Convert buffers to a matrix
	int k = 0;
	for (i = 0; i < n; ++i)
	{
		for (j = 0; j < n; ++j)
		{
			matrixA[i][j] = buf1[k];
			matrixB[i][j] = buf2[k];
			++k;
		}
	}


	// Save the required parts of matrix A and B in an array to calculate one quarter part of matrix C.
	// For First quarter
	int m = 0;
	for (i = 0; i < n/2 ; ++i)
	{
		for (j = 0; j < n/2; ++j)
		{
			k=0;
			while(k != n) {
				temp1[m] = matrixA[i][k];
				temp1[m+1] = matrixB[k][j];
				++k;
				m = m + 2;
			}
		}
	}

	// For Second quarter
	m = 0;
	for (i = 0; i < n/2 ; ++i)
	{
		for (j = n/2 ; j < n; ++j)
		{
			k=0;
			while(k != n) {

				temp2[m] = matrixA[i][k];
				temp2[m+1] = matrixB[k][j];
				++k;
				m = m + 2;
			}
		}
	}

	// For Third quarter
	m = 0;
	for (i = n/2; i < n ; ++i)
	{
		for (j = 0 ; j < n/2; ++j)
		{
			k=0;
			while(k != n) {
				
				temp3[m] = matrixA[i][k];
				temp3[m+1] = matrixB[k][j];
				++k;
				m = m + 2;
			}
		}
	}

	// For Fourth quarter
	m = 0;
	for (i = n/2; i < n ; ++i)
	{
		for (j = n/2 ; j < n; ++j)
		{
			k=0;
			while(k != n) {

				temp4[m] = matrixA[i][k];
				temp4[m+1] = matrixB[k][j];
				++k;
				m = m + 2;
			}
		}
	}


	/* Create bidirectional pipes between parent and its child processes  */
	int fds2_1[2];	// _1 represents first pipes
	int fds2_2[2];	// _2 represent second pipes
	int fds3_1[2];
	int fds3_2[2];	// fds2 is for P2 process, fds3 is for P3 process,
	int fds4_1[2];	// fds4 is for P4 process, fds5 is for P5 process
	int fds4_2[2];
	int fds5_1[2];
	int fds5_2[2];

	// Create pipes
	if (pipe(fds2_1) == -1 || pipe(fds2_2) == -1)
	{
		perror("Unable to create pipes between p1 and p2");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}

	if (pipe(fds3_1) == -1 || pipe(fds3_2) == -1)
	{
		perror("Unable to create pipes between p1 and p3");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}

	if (pipe(fds4_1) == -1 || pipe(fds4_2) == -1)
	{
		perror("Unable to create pipes between p1 and p4");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}

	if (pipe(fds5_1) == -1 || pipe(fds5_2) == -1)
	{
		perror("Unable to create pipes between p1 and p5");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}	


	// to read from pipe into buffer
	// row of the matrix is 2^n, column of the matrix is 2^n.
	// So matrix has 2^n x 2^n elements in it.
	// Every process handles the quarter part. So they should take (total elements / 4) elements.
	// To calculate one part, they need 2^n rows  and columns.
	// So size of the buffer should be (2^n x 2^n / 4) x (2^n x 2)
	int onePart = (n*n/4)*(n*2);

	int requirePart = n*2;
	int handledPart = (n*n) / 4;

	/* Creat child processes and runs them */
	pid_t id2, id3, id4, id5;
	fflush(stdout);

	id2 = fork();
	if (id2 == 0)	// If it is child
	{	
		struct sigaction sa_p2;
		memset(&sa_p2,0,sizeof(sa_p2));
		sa_p2.sa_handler = &handler3;

		// To catch SIGINT
		if (sigaction(SIGINT,&sa_p2,NULL) == -1)
		{
			perror("sigaction error is occured in P2 process");
			exit(1);
		}

		// Close write-end of the first pipe for child. It is unused.
		if(close(fds2_1[1]) == -1)
		{
			perror("Unable to close write-end of the first pipe");
			exit(1);
		}


		// Read from the pipe
		char* ch_buf2 = (char*) malloc(sizeof(char)*onePart);
		p2_arrays[count2] = ch_buf2;
		++count2;
		if(read(fds2_1[0],ch_buf2,onePart) == -1)
		{
			perror("Child1 failed to read from the first pipe");
			free(ch_buf2);
			exit(1);
		}
	
		int* results2 = (int*) malloc(sizeof(int)*n*n / 4);
		p2_arrays[count2] = results2;
		++count2;
		int m = 0, temp , res = 0, i = 0; j = 0; k = 0;
		while(i != handledPart)
		{
			while(j != requirePart)
			{
				temp = (unsigned char) ch_buf2[k] * (unsigned char) ch_buf2[k+1];
				res = res + temp;
				k = k + 2;
				j = j + 2;
			}
			
			results2[m] = res;	// Calculate the results of the quarter part and save them in an array.
			res = 0;
			++m;
			++i;
			j = requirePart;
			requirePart = requirePart + n*2;
		}

		// Close read end of the first pipe, operation is done. Close the read-end of second pipe, it is unused.
		if(close(fds2_1[0]) == -1 || close(fds2_2[0]) == -1)
		{
			perror("Child1 failed to close file descriptor of the pipe");
			free(ch_buf2);
			free(results2);
			exit(1);
		}

		// Write results to second pipe.
		if(write(fds2_2[1],results2,n*n*sizeof(int)/4) == -1)
		{
			perror("Child1 failed to write to second pipe");
			free(ch_buf2);
			free(results2);
			exit(1);
		}


		// Write operation is done. Close the write end of the second pipe.
		if(close(fds2_2[1]) == -1)
		{
			perror("Child1 failed to close write-end of second pipe");
			free(ch_buf2);
			free(results2);
			exit(1);
		}

		// Avoid double free if SIGINT signal is sent
		free(results2);
		--count2;
		free(ch_buf2);
		--count2;

		exit(0);
	}


	id3 = fork();
	if (id3 == 0)
	{	
		struct sigaction sa_p3;
		memset(&sa_p3,0,sizeof(sa_p3));
		sa_p3.sa_handler = &handler4;

		// To catch SIGINT
		if (sigaction(SIGINT,&sa_p3,NULL) == -1)
		{
			perror("sigaction error is occured in P3 process");
			exit(1);
		}


		// Close write end of the first pipe for child2. It is unused.
		if(close(fds3_1[1]) == -1)
		{
			perror("Child2 failed to close write-end of the first pipe");
			exit(1);
		}

		// Read from the first pipe
		char* ch_buf3 = (char*) malloc(sizeof(char)*onePart);
		p3_arrays[count3] = ch_buf3;
		++count3;
		if(read(fds3_1[0],ch_buf3,onePart) == -1)
		{
			perror("Child2 failed to read from first pipe");
			free(ch_buf3);
			exit(1);
		}

		int* results3 = (int*) malloc(sizeof(int)*n*n / 4);
		p3_arrays[count3] = results3;
		++count3;
		int m = 0, temp , res = 0, i = 0; j = 0; k = 0;
		while(i != handledPart)
		{
			while(j != requirePart)
			{
				temp = (unsigned char) ch_buf3[k] * (unsigned char) ch_buf3[k+1];
				res = res + temp;
				k = k + 2;
				j = j + 2;
			}

			results3[m] = res;	// Put the results of the blocks in results3 array.
			res = 0;
			++m;
			++i;
			j = requirePart;
			requirePart = requirePart + n*2;
		}

		// Close the read-end of the first pipe. Operation is done. Close the read-end of the second pipe. It is unused.
		if(close(fds3_1[0]) == -1 || close(fds3_2[0]) == -1)
		{
			perror("Child2 failed to close file descriptor of the pipe");
			free(ch_buf3);
			free(results3);
			exit(1);
		}

		// Write to second pipe.
		if(write(fds3_2[1],results3,n*n*sizeof(int)/4) == -1)
		{
			perror("Child2 failed to write to second pipe");
			free(ch_buf3);
			free(results3);
			exit(1);
		}

		// Close the write-end of the second pipe. Operation is done.
		if(close(fds3_2[1]) == -1)
		{
			perror("Child2 failed to close write-end of second pipe");
			free(ch_buf3);
			free(results3);
			exit(1);
		}

		// Avoid double free if SIGINT signal is sent
		free(results3);
		--count3;
		free(ch_buf3);
		--count3;
		exit(0);		
	}


	id4 = fork();
	if (id4 == 0)
	{	
		struct sigaction sa_p4;
		memset(&sa_p4,0,sizeof(sa_p4));
		sa_p4.sa_handler = &handler5;

		// To catch SIGINT
		if (sigaction(SIGINT,&sa_p4,NULL) == -1)
		{
			perror("sigaction error is occured in P4 process");
			exit(1);
		}


		// Close write end of the first pipe for child3
		if(close(fds4_1[1]) == -1)
		{
			perror("Child3 failed to close write-end of the first pipe");
			exit(1);
		}
		
		// Read from the first pipe
		char* ch_buf4 = (char*) malloc(sizeof(char)*onePart);
		p4_arrays[count4] = ch_buf4;
		++count4;
		if(read(fds4_1[0],ch_buf4,onePart) == -1)
		{
			perror("Child3 failed to read from first pipe");
			free(ch_buf4);
			exit(1);
		}

		
		int* results4 = (int*) malloc(sizeof(int)*n*n / 4);
		p4_arrays[count4] = results4;
		++count4;
		int m = 0, temp , res = 0, i = 0; j = 0; k = 0;
		
		while(i != handledPart)
		{	
			while(j != requirePart)
			{	
				temp = (unsigned char) ch_buf4[k] * (unsigned char) ch_buf4[k+1];
				res = res + temp;
				k = k + 2;
				j = j + 2;
			}

			results4[m] = res;	// Save results in a array
			res = 0;
			++m;
			++i;
			j = requirePart;
			requirePart = requirePart + n*2;
		}

		// close the read-end of the first pipe. Operation is done and close the read-end of the second pipe. It is unused.
		if(close(fds4_1[0]) == -1 || close(fds4_2[0]) == -1)
		{
			perror("Child3 failed to close file descriptor of the pipe");
			free(ch_buf4);
			free(results4);
			exit(1);
		}

		// Write to second pipe.
		if(write(fds4_2[1],results4,n*n*sizeof(int)/4) == -1)
		{
			perror("Child3 failed to write to second pipe");
			free(ch_buf4);
			free(results4);
			exit(1);
		}

		// Close the write end of the second pipe. Operation is done.
		if(close(fds4_2[1]) == -1)
		{
			perror("Child3 failed to close write-end of second pipe");
			free(ch_buf4);
			free(results4);
			exit(1);
		}

		// Avoid double free in case of SIGINT
		free(results4);
		--count4;
		free(ch_buf4);
		--count4;
		exit(0);
	}


	id5 = fork();
	if (id5 == 0)
	{	

		struct sigaction sa_p5;
		memset(&sa_p5,0,sizeof(sa_p5));
		sa_p5.sa_handler = &handler6;

		// To catch SIGINT
		if (sigaction(SIGINT,&sa_p5,NULL) == -1)
		{
			perror("sigaction error is occured in P5 process");
			exit(1);
		}


		// Close write-end of the first pipe for child. It is unused.
		if(close(fds5_1[1]) == -1)
		{
			perror("Child4 failed to close write-end of the first pipe");
			exit(1);
		}

		// Read from the first pipe.
		char* ch_buf5 = (char*) malloc(sizeof(char)*onePart);
		p5_arrays[count5] = ch_buf5;
		++count5;
		if (read(fds5_1[0],ch_buf5,onePart) == -1)
		{
			perror("Child4 failed to read from first pipe");
			free(ch_buf5);
			exit(1);
		}
	

		int* results5 = (int*) malloc(sizeof(int)*n*n / 4);
		p5_arrays[count5] = results5;
		++count5;
		int m = 0, temp , res = 0, i = 0; j = 0; k = 0;
		while(i != handledPart)
		{
			while(j != requirePart)
			{
				temp = (unsigned char) ch_buf5[k] * (unsigned char) ch_buf5[k+1];
				res = res + temp;
				k = k + 2;
				j = j + 2;
			}

			results5[m] = res;	// Save results in an array.
			res = 0;
			++m;
			++i;
			j = requirePart;
			requirePart = requirePart + n*2;
		}
		
		// Close the read-end of the first pipe. Operation is done and close the read-end of the second pipe. It is unused.
		if(close(fds5_1[0]) == -1 || close(fds5_2[0]) == -1)
		{
			perror("Child4 failed to close file descriptor of the pipe");
			free(ch_buf5);
			free(results5);
			exit(1);
		}

		// Write to second pipe.
		if(write(fds5_2[1],results5,n*n*sizeof(int)/4) == -1)
		{
			perror("Child4 failed to write to second pipe");
			free(ch_buf5);
			free(results5);
			exit(1);
		}

		// Close the write-end of the second pipe. Operation is done.
		if(close(fds5_2[1]) == -1)
		{
			perror("Child4 failed to close write-end of second pipe");
			exit(1);
		}

		
		// Avoid double free in case of SIGINT
		free(results5);
		--count5;
		free(ch_buf5);
		--count5;
		exit(0);
	}


	// PARENT
	// Close read-ends of the first pipes, they are unused for parent.
	if(close(fds2_1[0]) == -1 || close(fds3_1[0]) == -1 || close(fds4_1[0]) == -1 || close(fds5_1[0]) == -1 )
	{
		perror("Unable to close read-end of first pipes");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}
	

	// Send required informations to child processes.
	if(write(fds2_1[1],temp1,onePart) == -1 )
	{
		perror("Failed to write to first pipe4");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}
	if(close(fds2_1[1]) == -1 )
	{
		perror("Unable to close write-end of first pipe"); 
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}


	if(write(fds3_1[1],temp2,onePart) == -1 )
	{
		perror("Failed to write to second pipe");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}
	if(close(fds3_1[1]) == -1 )
	{
		perror("Unable to close write-end of second pipe"); 
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}

	
	if(write(fds4_1[1],temp3,onePart) == -1 )
	{
		perror("Failed to write to third pipe");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}
	if(close(fds4_1[1]) == -1 )
	{
		perror("Unable to close write-end of third pipe"); 
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}

	if(write(fds5_1[1],temp4,onePart) == -1 )
	{
		perror("Failed to write to fourth pipe");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}
	if(close(fds5_1[1]) == -1 )
	{
		perror("Unable to close write-end of fourth pipe"); 
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		close(fd1);
		close(fd2);
		exit(1);
	}

	
	// To read from the second pipes 
	int* result2 = (int*) malloc(sizeof(int)*n*n/4);
	p1_arrays[count1] = result2;
	++count1;

	int* result3 = (int*) malloc(sizeof(int)*n*n/4);
	p1_arrays[count1] = result3;
	++count1;

	int* result4 = (int*) malloc(sizeof(int)*n*n/4);
	p1_arrays[count1] = result4;
	++count1;

	int* result5 = (int*) malloc(sizeof(int)*n*n/4);
	p1_arrays[count1] = result5;
	++count1;


	// Close the write-end of the second pipes, they are unused.
	if (close(fds2_2[1]) == -1)
	{
		perror("Unable to close write-end of second pipe for P2");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		free(result2);
		free(result3);
		free(result4);
		free(result5);
		close(fd1);
		close(fd2);
		exit(1);
	}
	if(read(fds2_2[0],result2,n*n*sizeof(int)/4) ==  -1)
	{
		perror("Unable to read from secod pipe for P2");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		free(result2);
		free(result3);
		free(result4);
		free(result5);
		close(fd1);
		close(fd2);
		exit(1);
	}

	
	if (close(fds3_2[1]) == -1)
	{
		perror("Unable to close write-end of second pipe for P3");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		free(result2);
		free(result3);
		free(result4);
		free(result5);
		close(fd1);
		close(fd2);
		exit(1);
	}
	if(read(fds3_2[0],result3,n*n*sizeof(int)/4) == -1)
	{	
		perror("Unable to read from secod pipe for P3");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		free(result2);
		free(result3);
		free(result4);
		free(result5);
		close(fd1);
		close(fd2);
		exit(1);
	}


	if (close(fds4_2[1]) == -1)
	{
		perror("Unable to close write-end of second pipe for P4");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		free(result2);
		free(result3);
		free(result4);
		free(result5);
		close(fd1);
		close(fd2);
		exit(1);
	}
	if (read(fds4_2[0],result4,n*n*sizeof(int)/4) == -1)
	{
		perror("Unable to read from secod pipe for P4");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		free(result2);
		free(result3);
		free(result4);
		free(result5);
		close(fd1);
		close(fd2);
		exit(1);
	}


	if (close(fds5_2[1]) == -1)
	{
		perror("Unable to close write-end of second pipe for P5");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		free(result2);
		free(result3);
		free(result4);
		free(result5);
		close(fd1);
		close(fd2);
		exit(1);
	}
	if(read(fds5_2[0],result5,n*n*sizeof(int)/4) == -1)
	{
		perror("Unable to read from secod pipe for P5");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		free(result2);
		free(result3);
		free(result4);
		free(result5);
		close(fd1);
		close(fd2);
		exit(1);
	}


	// Product matrix
	float** matrixC = (float**) malloc(sizeof(float*)*n);
	for (int i = 0; i < n; ++i)
	{
		matrixC[i] = (float*) malloc(sizeof(float)*n);
	}

	// Combine the results of the quarter parts.
	// First quarter
	k = 0;
	for (i = 0; i < n/2; ++i)
	{
		for (j = 0; j < n/2; ++j)
		{
			matrixC[i][j] = result2[k];
			++k;
		}
	}

	// Second quarter
	k = 0;
	for (i = 0; i < n/2; ++i)
	{
		for (j = n/2; j < n; ++j)
		{
			matrixC[i][j] = result3[k];
			++k;
		}
	}

	// Third quarter
	k = 0;
	for (i = n/2; i < n; ++i)
	{
		for (j = 0; j < n/2; ++j)
		{
			matrixC[i][j] = result4[k];
			++k;
		}
	}

	// Fourth quarter
	k = 0;
	for (i = n/2; i < n; ++i)
	{
		for (j = n/2; j < n; ++j)
		{
			matrixC[i][j] = result5[k];
			++k;
		}
	}


	/* Singular value calculation */ 
	float* singularVAlues = (float*) malloc(sizeof(float)*(n*n*10));
	p1_arrays[count1] = singularVAlues;
	++count1;

	float** v = (float**) malloc(sizeof(float*)*n);
	for (int i = 0; i < n; ++i)
	{		
		v[i] = (float*) malloc(sizeof(float)*n);	// We should calculate only singular values.
	}												// So I use v as a parameter,but I don't print

	
	dsvd(matrixC,n,n,singularVAlues,v);

	printf("\n");
	printf("Singular Values of matrix C\n");
    for(i=0; i<n; ++i){
        printf("%0.3f  ", singularVAlues[i]);
    }
    printf("\n\n");


	// Close the read-end of the second pipes. Operation is done
	if(close(fds2_2[0]) == -1 || close(fds3_2[0]) == -1 || close(fds4_2[0]) == -1 || close(fds5_2[0]) == -1)
	{
		perror("Unable to close read-end of second pipes"); 
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		free(result2);
		free(result3);
		free(result4);
		free(result5);
		free(singularVAlues);
		for (i = 0; i < n; ++i)
			free(v[i]);
		free(v);

		for (i = 0; i < n; ++i)
			free(matrixC[i]);
		free(matrixC);

		close(fd1);
		close(fd2);
		exit(1);
	}


	// Wait for all children to be dead
	while(aliveChildren > 0){
		sigsuspend(&newMask);
	}


	// Close the input1 and input2 files 
	if (close(fd1) == -1 || close(fd2) == -1)
	{
		perror("Unable to close input files");
		free(buf1);
		free(buf2);
		free(temp1);
		free(temp2);
		free(temp3);
		free(temp4);
		free(result2);
		free(result3);
		free(result4);
		free(result5);
		free(singularVAlues);
		for (i = 0; i < n; ++i)
			free(v[i]);
		free(v);

		for (i = 0; i < n; ++i)
			free(matrixC[i]);
		free(matrixC);

		exit(1);
	}
	
	
	// free resources
	// Decrease coun1 to avoid double free in case of SIGINT
	free(singularVAlues);
	--count1;
	free(result5);
	--count1;
	free(result4);
	--count1;
	free(result3);
	--count1;
	free(result2);
	--count1;
	free(temp4);
	--count1;
	free(temp3);
	--count1;
	free(temp2);
	--count1;
	free(temp1);
	--count1;
	free(buf2);
	--count1;
	free(buf1);
	--count1;
}



int main(int argc, char *argv[])
{
	p1_arrays = (void**) malloc(sizeof(void*)*11);
	p2_arrays = (void**) malloc(sizeof(void*)*2);
	p3_arrays = (void**) malloc(sizeof(void*)*2);
	p4_arrays = (void**) malloc(sizeof(void*)*2);
	p5_arrays = (void**) malloc(sizeof(void*)*2);

	sigset_t set;

	// For SIGCHLD
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = &handler;

	// For SIGINT
	struct sigaction sa2;
	memset(&sa2,0,sizeof(sa2));
	sa2.sa_handler = &handler2;

	// To catch SIGCHLD
	if (sigaction(SIGCHLD,&sa,NULL) == -1)
	{
		perror("sigaction error is occured");
		exit(1);
	}

	// To catch SIGINT
	if (sigaction(SIGINT,&sa2,NULL) == -1)
	{
		perror("sigaction error is occured");
		exit(1);
	}

	if (sigemptyset(&set) == -1 || sigaddset(&set,SIGCHLD) == -1)
	{
		perror("Failed to initialize signal mask");
		exit(1);
	}
	// Block the SIGCHLD signal
	if (sigprocmask(SIG_BLOCK,&set,NULL) == -1)
	{
		perror("Failed to block operation");
		exit(1);
	}



	// Save command line arguments into this variables.
	Rfilename = (char*) malloc(sizeof(char)*100);
	++Rcount;
	Rfilename2 = (char*) malloc(sizeof(char)*100);
	++Rcount;

	int n;
	// Parse command line
	int count = 0; // To check all the options are entered by the user.
	int opt;
	while((opt = getopt(argc, argv, ":i:j:n:")) != -1)
	{
		switch(opt)
		{
			case 'i':
			{	
				++count;
				strcpy(Rfilename,optarg);
				break;
			}

			case 'j':
			{
				++count;
				strcpy(Rfilename2,optarg);
				break;
			}

			case 'n':
			{
				++count;
				n = atoi(optarg);
				if(n < 0)
				{
					printf("n must be positive\n");
					free(Rfilename);
					free(Rfilename2);
					exit(1);
				}
				break;
			}

			case ':':
			{
				printf("option needs a value\n");
				free(Rfilename);
				free(Rfilename2);
				exit(1);
				break;
			}

			case '?':
			{
				printf("Unknown option %c,You must enter a command line like this : -i input -o output\n", optopt);
				free(Rfilename);
				free(Rfilename2);
				exit(1);
				break;
			}
		}
	}

	for ( ; optind < argc; ++optind)
	{
		printf("You entered extra arguments %s,You must enter a command line like this : -i input -o output\n",argv[optind]);
		free(Rfilename);
		free(Rfilename2);
		exit(1);
	}

	if (count == 3){
		readFromFile(Rfilename,Rfilename2,n);
	}

	else{
		printf("You entered missing command lines, You must enter a command line like this : -i input -o output\n");
		free(Rfilename);
		free(Rfilename2);
		exit(1);
	}


	//Free all resources.
	free(Rfilename2);
	--Rcount;
	free(Rfilename);
	--Rcount;

	return 0;

}