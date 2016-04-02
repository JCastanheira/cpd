#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>

int maxVar(int* linha){
	int j, max=0;
	for(j=0;j<20;j++){
		if(abs(linha[j])>max){
			max=abs(linha[j]);
		}
	}
	return max;
}
/*
int compMax(const void* linha1, const void* linha2){
	if(maxVar((int*)linha1)>maxVar((int*)linha2)){
		return 1;
	}
	if(maxVar((int*)linha1)<maxVar((int*)linha2)){
		return -1;
	}
	return 0;
}
*/

void searchTree(int node, int n_clauses, int mat[][20], int* status, int imp_clauses, int *maxsat, int *maxsat_count, int n_vars, int* best, int* current){
	int i,j;
	current[abs(node)-1]=node;
	for(i=0;i<n_clauses;i++){
		if(status[i]!=0){
			continue;
		}
		for(j=0;mat[i][j]!=0;j++){
			if(mat[i][j]==node){
				status[i]=1;
			}
		}
		if(maxVar(mat[i])<=abs(node) && status[i]==0){
			status[i]=-1;
			imp_clauses++;
		}
	}
	if((n_clauses-imp_clauses)<(*maxsat)){
		return;
	}
	int mscount=0;
	for(i=0;i<n_clauses;i++){
		if(status[i]==0){
			mscount=-1;
			break;
		}
		if(status[i]==1){
			mscount++;
		}
	}
	/*printf("node: %d\n",node);
	for(i=0;i<n_clauses;i++){
		printf("status: %d\n",status[i]);
	}
	printf("imp_clauses: %d\n",imp_clauses);
	printf("mscount: %d\n",mscount);*/
	if(mscount>(*maxsat)){
		(*maxsat)=mscount;
		for(i=0;i<n_vars;i++){
			best[i]=current[i];
		}
		(*maxsat_count)=0;
		(*maxsat_count)+=(2^(n_vars-abs(node)))-1;
		/*printf("maxsat_count: %d %d\n",(*maxsat_count),2^(n_vars-abs(node)));*/
		return;
	}
	if(mscount==(*maxsat)){
		(*maxsat_count)+=(2^(n_vars-abs(node)))-1;
		/*printf("maxsat_count: %d\n",(*maxsat_count));*/
		return;
	}
	if(mscount<(*maxsat)&&mscount>=0){
		return;
	}
	int status_c[n_clauses];
	for(i=0;i<n_clauses;i++){
		status_c[i]=status[i];
	}
	searchTree(-abs(node)-1,n_clauses,mat,status,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current);
	searchTree(abs(node)+1,n_clauses,mat,status_c,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current);
}

int main(int argc, char ** argv){
	double start = omp_get_wtime();
	char * fileNameIn;
	char * fileNameOut;
	FILE * fp;
	char extOut[] = ".solng";/*mudar depois*/
	char str[60];
	int n_vars, n_clauses,n=0;
	int i=0,j=0,num;
	
	
	if(argc < 2){
		printf("Usage: maxsat [filename]\n");
		exit(1);
	}
	
	fileNameIn= argv[1];
	/*printf("%s\n", fileNameIn);*/
	
	fp  = fopen(fileNameIn, "r");
	
	if(fp == NULL){
		printf("Open error of input file.\n");
		exit(2);
	}
	fgets(str,60,fp);
	sscanf(str,"%d %d", &n_vars, &n_clauses);
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
	fclose(fp);
	/*for(i=0;i<n_clauses;i++){
		for(j=0;j<20;j++){
			printf("%d ", mat[i][j]);
		}
		printf("\n");
	}*/
	int status[n_clauses];
	for(i=0;i<n_clauses;i++){
		status[i]=0;
	}
	/*printf("n_vars: %d\n",n_vars);*/
	int maxsat=0, maxsat_count=0;
	int best[n_vars],current[n_vars];
	for(i=1;i<=n_vars;i++){
		best[i-1]=-i;
		current[i-1]=-i;
	}
	searchTree(-1,n_clauses,mat,status,0,&maxsat,&maxsat_count,n_vars,&best[0],&current[0]);
	for(i=0;i<n_clauses;i++){
		status[i]=0;
	}
	searchTree(1,n_clauses,mat,status,0,&maxsat,&maxsat_count,n_vars,&best[0],&current[0]);
	printf("%d %d\n",maxsat,maxsat_count);
	/*qsort(mat,n_clauses,20*sizeof(int),compMax);
	printf("Sorted:\n");
	for(i=0;i<n_clauses;i++){
		for(j=0;j<20;j++){
			printf("%d ", mat[i][j]);
		}
		printf("\n");
	}*/
	printf("Best:\n");
	for(i=0;i<n_vars;i++){
		printf("%d ",best[i]);
	}
	printf("\n");
	
	fileNameIn[strlen(fileNameIn)-3] = '\0';
	
	fileNameOut = (char *) malloc(sizeof(char) * 
                                 (strlen(fileNameIn) + 
									  strlen(extOut) + 1));
	if(fileNameOut == NULL){
		printf("Memory allocation error for fileNameOut.\n");
		exit(1);
	}

	strcpy(fileNameOut, fileNameIn);
	strcat(fileNameOut, extOut);

	fp = fopen(fileNameOut, "w");
	if(fp == NULL){
		printf("Open error of output file.\n");
		exit(2);
	}
	
	fprintf(fp,"%d %d\n",maxsat,maxsat_count);
	for(i=0;i<n_vars;i++){
		fprintf(fp,"%d ",best[i]);
	}
	fprintf(fp,"\n");
	fclose(fp);
	free(fileNameOut);
	double end = omp_get_wtime();
	printf("Time: %f\n",end-start);
	exit(0);
}
