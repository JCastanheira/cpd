#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>
#include <mpi.h>
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

void searchTree(int node, int n_clauses, int mat[][20], int* status, int imp_clauses, int *maxsat, int *maxsat_count, int n_vars, int* best, int* current, int id, int idf, int p){
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
	if(mscount>(*maxsat)){
		(*maxsat)=mscount;
		for(i=0;i<n_vars;i++){
			best[i]=current[i];
		}
		(*maxsat_count)=(int) pow(2.0,n_vars-abs(node));
		return;
	}
	if(mscount==(*maxsat)){
		(*maxsat_count)+=(int) pow(2.0,n_vars-abs(node));
		return;
	}
	if(mscount<(*maxsat)&&mscount>=0){
		return;
	}
	int status_c[n_clauses];
	for(i=0;i<n_clauses;i++){
		status_c[i]=status[i];
	}
	if(p>1){
		if(idf<p/2){
			searchTree(-abs(node)-1,n_clauses,mat,status,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current,id,idf,p/2);
		}else{
			searchTree(abs(node)+1,n_clauses,mat,status_c,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current,id,idf-(p/2),p-p/2);
		}
	}else{
		searchTree(-abs(node)-1,n_clauses,mat,status,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current,id,0,1);
		searchTree(abs(node)+1,n_clauses,mat,status_c,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current,id,0,1);
	}
}

int main(int argc, char ** argv){
	double start = omp_get_wtime();
	char * fileNameIn;
	char * fileNameOut;
	FILE * fp;
	char extOut[] = ".outg";
	char str[60];
	int n_vars, n_clauses,n=0;
	int i=0,j=0,num;
	MPI_Status stat;
    int id, p;
	
	MPI_Init(&argc, &argv);

    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);
	
	if(argc < 2){
		if (!id) printf("Usage: maxsat [filename]\n");
		MPI_Finalize();
		exit(1);
	}
	
	/*int k=(int)log2((double)p);*/
	
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
	int status[n_clauses];
	for(i=0;i<n_clauses;i++){
		status[i]=0;
	}
	int maxsat=0, maxsat_count=0;
	int best[n_vars],current[n_vars];
	for(i=1;i<=n_vars;i++){
		best[i-1]=-i;
		current[i-1]=-i;
	}
	if(id<p/2){
		searchTree(-1,n_clauses,mat,status,0,&maxsat,&maxsat_count,n_vars,&best[0],&current[0],id,id,p/2);
	}else{
		for(i=0;i<n_clauses;i++){
			status[i]=0;
		}
		searchTree(1,n_clauses,mat,status,0,&maxsat,&maxsat_count,n_vars,&best[0],&current[0],id,id-(p/2),p-(p/2));
	}
	MPI_Barrier(MPI_COMM_WORLD);
	int ms;
	/*MPI_Gather instead?*/
	MPI_Reduce(&maxsat,&ms,1,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);/*use MPI_MAXLOC instead?*/
	
	if(!id){
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
		printf("%d %d\n",maxsat,maxsat_count);fflush(stdout);
		fprintf(fp,"%d %d\n",maxsat,maxsat_count);
		for(i=0;i<n_vars;i++){
			printf("%d ",best[i]);fflush(stdout);
			fprintf(fp,"%d ",best[i]);
		}
		printf("\n");
		fprintf(fp,"\n");
		fclose(fp);
		free(fileNameOut);
		double end = omp_get_wtime();
		printf("Time: %f\n",end-start);
	}

	MPI_Finalize();
	exit(0);
}
