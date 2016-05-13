#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>
#include <math.h>

int maxVar(int* linha){
	int j, max=0;
	for(j=0;j<20;j++){
		if(abs(linha[j])>max){
			max=abs(linha[j]);
		}
	}
	return max;
}

void searchTree(int node, int n_clauses, int mat[][20], int* status, int imp_clauses, int *maxsat, int *maxsat_count, int n_vars, int* best, int* current, int* n_threads){
	(*n_threads)--;
	int i,j;
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
	int a;
	#pragma omp critical(ms_update)
	a=(*maxsat);
	if((n_clauses-imp_clauses)<a){
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
	#pragma omp critical(ms_update)
	a=(*maxsat);
	if(mscount>a){
		#pragma omp critical(ms_update)		
		(*maxsat)=mscount;
		#pragma omp critical
		for(i=0;i<n_vars;i++){
			best[i]=current[i];
		}	
		#pragma omp critical(mc_update)
		(*maxsat_count)=(int) pow(2.0,n_vars-abs(node));	
		return;
	}
	#pragma omp critical(ms_update)
	a=(*maxsat);
	if(mscount==a){
		#pragma omp critical(mc_update)
		(*maxsat_count)+=(int) pow(2.0,n_vars-abs(node));
		return;
	}
	#pragma omp critical(ms_update)
	a=(*maxsat);
	if(mscount<a&&mscount>=0){
		return;
	}
	int status_c[n_clauses];
	for(i=0;i<n_clauses;i++){
		status_c[i]=status[i];
	}
	int current2[n_vars];
	for(i=0;i<n_vars;i++){
		current2[i]=current[i];
	}
	current[abs(node)]=-abs(node)-1;
	current2[abs(node)]=abs(node)+1;
	(*n_threads)++;
	#pragma omp parallel if((*n_threads)>1) num_threads(2)
	{
	#pragma omp single nowait
	searchTree(-abs(node)-1,n_clauses,mat,status,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current,n_threads);
	#pragma omp single nowait
	searchTree(abs(node)+1,n_clauses,mat,status_c,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current2,n_threads);
	}
}

int main(int argc, char ** argv){
	//Uncomment to show time
	//double start = omp_get_wtime();
	char * fileNameIn;
	FILE * fp;
	char str[60];
	int n_vars, n_clauses,n=0;
	int i=0,j=0,num;
	
	
	if(argc < 2){
		printf("Usage: maxsat [filename]\n");
		exit(1);
	}
	
	fileNameIn= argv[1];
	
	fp  = fopen(fileNameIn, "r");
	
	if(fp == NULL){
		printf("Open error of input file.\n");
		exit(2);
	}
	fgets(str,60,fp);
	sscanf(str,"%d %d", &n_vars, &n_clauses);
	int mat[n_clauses][20];

	for(i=0;i<n_clauses;i++){
		for(j=0;j<20;j++){
			mat[i][j]=0;
		}
	}
	
	for(i = 0; i < n_clauses; i++){
		fgets(str,60,fp);
		n=0,num=0;
		while (sscanf(&(str[n]), "%d %n", &(mat[i][num]), &j) == 1) {
			n += j;
			num++;			
		}
	}
	fclose(fp);
	int status[n_clauses], status2[n_clauses];
	int maxsat=0, maxsat_count=0;
	int best[n_vars],current[n_vars],current2[n_vars];
	for(i=0;i<n_clauses;i++){
		status[i]=0;
		status2[i]=0;
	}
	for(i=1;i<=n_vars;i++){
		best[i-1]=-i;
		current[i-1]=-i;
		current2[i-1]=-i;
	}
	current2[0]=1;
	omp_set_nested(1);
	int n_threads;
	#pragma omp parallel
	{
	n_threads=omp_get_num_threads();
	}
	#pragma omp parallel if(n_threads>1) num_threads(2)
	{
	#pragma omp single nowait
	searchTree(-1,n_clauses,mat,status,0,&maxsat,&maxsat_count,n_vars,best,current,&n_threads);
	#pragma omp single
	searchTree(1,n_clauses,mat,status2,0,&maxsat,&maxsat_count,n_vars,best,current2,&n_threads);
	}
	
	printf("%d %d\n",maxsat,maxsat_count);
	for(i=0;i<n_vars;i++){
		printf("%d ",best[i]);
	}
	printf("\n");
	//Uncomment to show time
	//double end = omp_get_wtime();
	//printf("Time: %f\n",end-start);
	exit(0);
}
