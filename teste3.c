#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>
#include <mpi.h>
#include <math.h>

/* Compile using:
 * mpicc -g -fopenmp maxsat-mpi.c -o maxsat-mpi -lm
 * Run using:
 * mpirun -np <num_procs> maxsat-mpi ex<n>.in
 */

int maxVar(int* linha){
	int j, max=0;
	for(j=0;j<20;j++){
		if(abs(linha[j])>max){
			max=abs(linha[j]);
		}
	}
	return max;
}

void searchTree(int node, int n_clauses, int mat[][20], int* status, int imp_clauses, int *maxsat, int *maxsat_count, int n_vars, int* best, int* current, int id, int idf, int p, int pr, MPI_Request *reqr, int *newmax){
	int i,j;
	int flag;
	MPI_Status stat;
	for(i=0;i<pr;i++){
		//printf("id: %d, i: %d\n", id, i);fflush(stdout);
		flag=0;
		if(id!=i){
			//printf("uh... id: %d, i: %d\n", id, i);fflush(stdout);
			MPI_Test(&reqr[i],&flag,&stat);
		}
		if(flag){
			//printf("dafuq2 id: %d, i: %d, newmax: %d\n", id, i, newmax[i]);fflush(stdout);

			//printf("dafuq id: %d, i: %d\n", id, i);fflush(stdout);
			if(newmax[i]>(*maxsat)){
				//printf("dafuq id: %d, i: %d, newmax: %d\n", id, i, newmax[i]);fflush(stdout);
				(*maxsat)=newmax[i];
				(*maxsat_count)=0;
			}
		}
	}
	
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
			searchTree(-abs(node)-1,n_clauses,mat,status,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current,id,idf,p/2,pr,&(*reqr),&(*newmax));
		}else{
			searchTree(abs(node)+1,n_clauses,mat,status_c,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current,id,idf-(p/2),p-p/2,pr,&(*reqr),&(*newmax));
		}
	}else{
		searchTree(-abs(node)-1,n_clauses,mat,status,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current,id,0,1,pr,&(*reqr),&(*newmax));
		searchTree(abs(node)+1,n_clauses,mat,status_c,imp_clauses,&(*maxsat),&(*maxsat_count),n_vars,best,current,id,0,1,pr,&(*reqr),&(*newmax));
	}
}

int main(int argc, char ** argv){
	double start = omp_get_wtime();
	char * fileNameIn;
	FILE * fp;
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
	/*if(id==1){
		for(i=0;i<n_clauses;i++){
			for(j=0;j<20;j++){
				printf("%d ",mat[i][j]);fflush(stdout);
			}
			printf("id: %d\n", id);fflush(stdout);
		}
	}*/
	
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
	int nodes=pow(2.0,n_vars-1);
	MPI_Request *reqr;
	int *newmax;
	reqr=(MPI_Request*)malloc(p*sizeof(MPI_Request));
	newmax=(int*)malloc(p*sizeof(int));
	for(i=0;i<p;i++){
		newmax[i]=0;
		if(i!=id){
			MPI_Irecv(&newmax[i],1,MPI_INT,i,i,MPI_COMM_WORLD,&reqr[i]);
		}
	}
	if(nodes>=p){
		if(id<p/2){
			printf("Node %d in part 1\n",id);fflush(stdout);
			searchTree(-1,n_clauses,mat,status,0,&maxsat,&maxsat_count,n_vars,&best[0],&current[0],id,id,p/2,p,&reqr[0],&newmax[0]);
		}else{
			for(i=0;i<n_clauses;i++){
				status[i]=0;
			}
			printf("Node %d in part 2\n",id);fflush(stdout);
			searchTree(1,n_clauses,mat,status,0,&maxsat,&maxsat_count,n_vars,&best[0],&current[0],id,id-(p/2),p-(p/2),p,&reqr[0],&newmax[0]);
		}
	}else{
		if(id<nodes/2){
			printf("Node %d in part 1\n",id);fflush(stdout);
			searchTree(-1,n_clauses,mat,status,0,&maxsat,&maxsat_count,n_vars,&best[0],&current[0],id,id,nodes/2,p,&reqr[0],&newmax[0]);
		}else{
			if(id<nodes){
				for(i=0;i<n_clauses;i++){
					status[i]=0;
				}
				printf("Node %d in part 2\n",id);fflush(stdout);
				searchTree(1,n_clauses,mat,status,0,&maxsat,&maxsat_count,n_vars,&best[0],&current[0],id,id-(nodes/2),nodes-(nodes/2),p,&reqr[0],&newmax[0]);
			}
		}
	}
	MPI_Request req[p];
	for(i=0;i<p;i++){
		if(i!=id){
			MPI_Isend(&maxsat,1,MPI_INT,i,id,MPI_COMM_WORLD,&req[i]);
			//MPI_Cancel(&reqr[i]);
		}
	}

	printf("Node %d ms %d count %d\n",id,maxsat,maxsat_count);fflush(stdout);
	MPI_Barrier(MPI_COMM_WORLD);
	int *ms, *mscount;
	ms=(int*)malloc(p*sizeof(int));
	mscount=(int*)malloc(p*sizeof(int));
	/*MPI_Gather instead?*/
	MPI_Gather(&maxsat, 1, MPI_INT, ms, 1, MPI_INT,0, MPI_COMM_WORLD);
	MPI_Gather(&maxsat_count, 1, MPI_INT, mscount, 1, MPI_INT,0, MPI_COMM_WORLD);
	/*MPI_Reduce(&maxsat,&ms,1,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);use MPI_MAXLOC instead?*/
	
	int hasbest=0;
	int finalcount=0;
	if(!id){
		printf("ms: ");
		for(i=0;i<p;i++){
			printf("%d ",ms[i]);
		}
		printf("\nmscount: ");
		for(i=0;i<p;i++){
			printf("%d ",mscount[i]);
		}
		printf("\n");
		
		int hasmax[p];
		for(i=0;i<p;i++){
			if(ms[i]>maxsat){
				maxsat=ms[i];
				hasmax[i]=1;
				if(mscount[i]>0){
					printf("%d wut\n",i);
					hasbest=i;
				}
				for(j=0;j<i;j++){
					hasmax[j]=0;
				}
			}
			if(ms[i]==maxsat){
				hasmax[i]=1;
				if(mscount[i]>0){
					printf("%d wut2\n",i);
					hasbest=i;
				}
			}
			if(ms[i]<maxsat){
				hasmax[i]=0;
			}
		}
		for(i=0;i<p;i++){
			if(hasmax[i]){
				printf("%d here, msc: %d\n",i,mscount[i]);
				finalcount+=mscount[i];
			}
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(&hasbest,1,MPI_INT,0,MPI_COMM_WORLD);	
	if(id==hasbest){
		MPI_Send(best,n_vars,MPI_INT,0,0,MPI_COMM_WORLD);
	}
	
	if(!id){
		MPI_Recv(best,n_vars,MPI_INT,hasbest,0,MPI_COMM_WORLD,&stat);

		printf("%d %d\n",maxsat,finalcount);fflush(stdout);

		for(i=0;i<n_vars;i++){
			printf("%d ",best[i]);fflush(stdout);

		}
		printf("\n");

		double end = omp_get_wtime();
		printf("Time: %f\n",end-start);
	}
	free(ms);
	free(mscount);
	MPI_Finalize();
	exit(0);
}
