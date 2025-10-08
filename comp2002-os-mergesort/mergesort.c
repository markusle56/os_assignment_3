/*
 * This file implements parallel mergesort.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc, free */
#include <pthread.h>
#include "mergesort.h"

/* merge two sorted subarrays A[leftstart..leftend] and A[rightstart..rightend]
 * into temporary array B, then copy back into A. */
void merge(int leftstart, int leftend, int rightstart, int rightend){
	int i = leftstart;
	int j = rightstart;
	int k = leftstart;

	while (i <= leftend && j <= rightend) {
		if (A[i] <= A[j]) {
			B[k++] = A[i++];
		} else {
			B[k++] = A[j++];
		}
	}

	if (i <= leftend) {
		size_t cnt = (size_t)(leftend - i + 1);
		memcpy(&B[k], &A[i], cnt * sizeof(int));
		k += cnt;
	}

	if (j <= rightend) {
		size_t cnt = (size_t)(rightend - j + 1);
		memcpy(&B[k], &A[j], cnt * sizeof(int));
		k += cnt;
	}
	size_t total = (size_t)(rightend - leftstart + 1);
	memcpy(&A[leftstart], &B[leftstart], total * sizeof(int));
}

/* serial recursive mergesort used as base-case */
void my_mergesort(int left, int right){
	if (left >= right) return;
	int mid = left + (right - left) / 2;
	my_mergesort(left, mid);
	my_mergesort(mid + 1, right);
	merge(left, mid, mid + 1, right);
}

/* parallel mergesort entry for pthreads */
void * parallel_mergesort(void *arg){
	struct argument *args = (struct argument *)arg;
	int left = args->left;
	int right = args->right;
	int level = args->level;

	if (left >= right) return NULL;

	if (level >= cutoff) {
		my_mergesort(left, right);
		return NULL;
	}

	int mid = left + (right - left) / 2;

	pthread_t t1, t2;
	int created1 = FALSE, created2 = FALSE;

	struct argument *argleft = buildArgs(left, mid, level + 1);
	struct argument *argright = buildArgs(mid + 1, right, level + 1);

	if (pthread_create(&t1, NULL, parallel_mergesort, argleft) == 0) {
		created1 = TRUE;
	} 
	if (pthread_create(&t2, NULL, parallel_mergesort, argright) == 0) {
		created2 = TRUE;
	}

	if (created1) pthread_join(t1, NULL);
	if (created2) pthread_join(t2, NULL);

	
	free(argleft);
	free(argright);

	merge(left, mid, mid + 1, right);

	return NULL;
}

/* allocate and return an argument struct for a thread */
struct argument * buildArgs(int left, int right, int level){
	struct argument * arg = (struct argument *) malloc(sizeof(struct argument));
	if (arg == NULL) {
		fprintf(stderr, "buildArgs: malloc failed\n");
		exit(EXIT_FAILURE);
	}
	arg->left = left;
	arg->right = right;
	arg->level = level;
	return arg;
}


