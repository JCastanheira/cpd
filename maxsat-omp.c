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
	/*current[abs(node)-1]=node;*/
	printf("-node: %d\n",node);fflush(stdout);
	for(i=0;i<n_vars;i++){
		printf("curr: %d ",current[i]);fflush(stdout);
	}
	printf("\n");
	/*#pragma omp for private(j)*/
	for(i=0;i<n_clauses;i++){
		if(status[i]!=0){
			continue;
		}
		for(j=0;mat[i][j]!=0;j++){
			if(mat[i][j]==node){
				#pragma omp critical
				status[i]=1;
			}
		}
		if(maxVar(mat[i])<=abs(node) && status[i]==0){
			#pragma omp critical
			{
			status[i]=-1;
			imp_clauses++;
		}
		}
	}
	#pragma omp barrier
	printf("help %d\n", node);fflush(stdout);
	for(i=0;i<n_clauses;i++){
		printf("status: %d, node: %d\n",status[i],node);fflush(stdout);
	}
	if((n_clauses-imp_clauses)<(*maxsat)){
		return;
	}
	int mscount=0;

	/*#pragma omp for*/
	for(i=0;i<n_clauses;i++){
		if(status[i]==0){
			#pragma omp critical
			mscount=-1;
		}
		if(status[i]==1 && mscount>=0){
			#pragma omp critical
			mscount++;
		}
	}

	printf("node: %d\n",node);fflush(stdout);
	for(i=0;i<n_clauses;i++){
		printf("2status: %d, node: %d\n",status[i],node);fflush(stdout);
	}
	printf("imp_clauses: %d\n",imp_clauses);fflush(stdout);
	printf("mscount: %d\n",mscount);fflush(stdout);
	if(mscount>(*maxsat)){
		#pragma omp critical
		(*maxsat)=mscount;
		/*#pragma omp for*/
		for(i=0;i<n_vars;i++){
			best[i]=current[i];
		}
		#pragma omp critical	
		(*maxsat_count)=(2^(n_vars-abs(node)))-1;
		printf("maxsat: %d, node %d\n",(*maxsat),node);fflush(stdout);
		printf("maxsat_count: %d %d, node %d\n",(*maxsat_count),2^(n_vars-abs(node)), node);fflush(stdout);
		return;
	}
	if(mscount==(*maxsat)){
		#pragma omp critical
		{
		(*maxsat_count)+=(2^(n_vars-abs(node)))-1;
	}
		printf("BROKEN? %d\n", node);fflush(stdout);
		printf("maxsat_count: %d\n",(*maxsat_count));fflush(stdout);
		return;
	}
	if(mscount<(*maxsat)&&mscount>=0){
		return;
	}
	/*#pragma omp barrier*/
	printf("Leggo %d\n", node);
	int status_c[n_clauses];
	/*#pragma omp for*/
	for(i=0;i<n_clauses;i++){
		status_c[i]=status[i];
	}
	int current2[n_vars];
	for(i=0;i<n_vars;i++){
		current2[i]=current[i];
	}
	current[abs(node)]=-abs(node)-1;
	current2[abs(node)]=abs(node)+1;
	#pragma omp sections
	{
	#pragma omp section
	{	
	searchTree(-abs(node)-1,n_clauses,mat,status,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current);
	}
	#pragma omp section
	{	
	searchTree(abs(node)+1,n_clauses,mat,status_c,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current2);
	}
	}
	#pragma omp barrier
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
	int status[n_clauses], status2[n_clauses];
	int maxsat=0, maxsat_count=0;
	int best[n_vars],current[n_vars],current2[n_vars];
	#pragma omp parallel
{
	#pragma omp for
	for(i=0;i<n_clauses;i++){
		status[i]=0;
		status2[i]=0;
		/*printf("%d\n",maxvec[i]);*/
	}
	#pragma omp for
	for(i=1;i<=n_vars;i++){
		best[i-1]=-i;
		current[i-1]=-i;
		current2[i-1]=-i;
	}
	current2[0]=1;
	#pragma omp sections
	{
	#pragma omp section
	{
		printf("Hi.1\n");
	searchTree(-1,n_clauses,mat,status,0,&maxsat,&maxsat_count,n_vars,best,current);
	}

	/*for(i=0;i<n_clauses;i++){
		status[i]=0;
	}*/
	#pragma omp section
	{
		printf("Hi.2\n");
	searchTree(1,n_clauses,mat,status2,0,&maxsat,&maxsat_count,n_vars,best,current2);
	}
	}
	#pragma omp barrier
}
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
