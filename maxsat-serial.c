#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int maxVar(int* linha){
	int j, max=0;
	for(j=0;j<20;j++){
		if(abs(linha[j])>max){
			max=abs(linha[j]);
		}
	}
	return max;
}

int compMax(const void* linha1, const void* linha2){
	if(maxVar((int*)linha1)>maxVar((int*)linha2)){
		return 1;
	}
	if(maxVar((int*)linha1)<maxVar((int*)linha2)){
		return -1;
	}
	return 0;
}

int main(int argc, char ** argv){
	
	char * fileNameIn;
	FILE * fp;
	char str[60];
	int n_variables, n_clauses,n=0;
	int i=0,j=0,num;
	
	
	if(argc < 2){
		printf("Usage: maxsat [filename]\n");
		exit(1);
	}
	
	fileNameIn= argv[1];
	printf("%s\n", fileNameIn);
	
	fp  = fopen(fileNameIn, "r");
	
	if(fp == NULL){
		printf("Open error of input file.\n");
		exit(2);
	}
	fgets(str,60,fp);
	sscanf(str,"%d %d", &n_variables, &n_clauses);
	/*printf("%d %d\n", n_variables, n_clauses);*/
	int mat[n_clauses][20];
	for(i=0;i<n_clauses;i++){
		for(j=0;j<20;j++){
			mat[i][j]=0;
		}
	}
	/*printf("Buff\n");*/
	for(i = 0; i < n_clauses; i++){
		fgets(str,60,fp);
		/*printf("%s",str);*/
		n=0,num=0;
		while (sscanf(&(str[n]), "%d %n", &(mat[i][num]), &j) == 1) {
			/*printf("%d\n",mat[i][num]);*/
			n += j;
			num++;			
			/*sleep(1);*/
		}
	}
	for(i=0;i<n_clauses;i++){
		for(j=0;j<20;j++){
			printf("%d ", mat[i][j]);
		}
		printf("\n");
	}
	/*int max, maxvec[n_clauses];*/
	for(i=0;i<n_clauses;i++){
		printf("%d\n",maxVar(mat[i]));
	}
	qsort(mat,n_clauses,20*sizeof(int),compMax);
	printf("Sorted:\n");
	for(i=0;i<n_clauses;i++){
		for(j=0;j<20;j++){
			printf("%d ", mat[i][j]);
		}
		printf("\n");
	}
	exit(0);
}
