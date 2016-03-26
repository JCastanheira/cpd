#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char ** argv){
	
	char * fileNameIn;
	FILE * fp;
	char str[60];
	int n_variables, n_clauses,nv,n;
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
	
	fscanf(fp,"%d %d", &n_variables, &n_clauses);
	printf("%d %d", n_variables, n_clauses);
	int mat[n_clauses][20];
	
	while(fgets(str,60,fp) != NULL){;	
		printf("%s",str);
		sleep(2);
		
		while((sscanf(str,"%d",&num) == 1){
			n++;
		}
		while((sscanf(str
		
		
	}
	exit(0);
}
