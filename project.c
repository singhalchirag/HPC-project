#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include "util.h"
#include <stdbool.h>
#define Array_Size 40000
//typedef enum { false, true } bool;


struct node {
  int id;
  int color;
  int neighbour[10];
  int ghost[10];
  bool isfilled; 
  int fillneighbour;
  int fillghost; 
};

/*struct Array {
  struct node *array;
  size_t used;
  size_t size;
};*/


/*void initArray(struct Array *a, size_t initialSize) {
	a->array = (struct node *)malloc(initialSize * sizeof(int));
	a->used = 0;
	a->size = initialSize;
}

void insertArray(struct Array *a,struct node element) {
     // Therefore a->used can go up to a->size 
	if (a->used == a->size) {
		a->size *= 2;
		printf("insert-1");
		a->array = (struct node *)realloc(a->array, a->size * sizeof(int));
		printf("insert-2");
	}
	a->array[a->used++] = element;
	printf("inserted\n");
}
  
void freeArray(struct Array *a){
	free(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}
*/
int main(int argc, char** argv) {


	//size_t pos = 0;
	//string delimiter = ":";
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int name_len;
	//std::ifstream file("Read.txt");
    	//std::string str;
	//string token;
	int tempnodevalue, p, N, mpirank;

	sscanf(argv[1], "%d", &N);	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpirank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	
	MPI_Get_processor_name(processor_name, &name_len);
  	printf("Rank %d/%d running on %s.\n", mpirank, p, processor_name);
	timestamp_type time1, time2;
  	get_timestamp(&time1); 
	FILE * fp;
    	char * line = NULL;
    	size_t len = 0;
    	ssize_t read;
	
	struct node vertexArray[Array_Size];
	struct node ghostArray[Array_Size];
	int uniqueTagArray[2*Array_Size];
	MPI_Request reqs[Array_Size];
        MPI_Status stats[Array_Size];
	
	int vertexcount=0;
	int ghostcount = 0;
	int uniqueTagCount = 0;
	int reqstatCount = 0;
	int totalRun = 0;
	printf("t1");
	
    fp = fopen("./graph-25000", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
	
    while ((read = getline(&line, &len, fp)) != -1) {
		int nodeValue,parentValue;
		char *token = strtok(line, ",");
		parentValue=atoi(token);
		//printf("taken:%s\n",token);
		if((( parentValue % N)) == mpirank){
			struct node vertex;
			//int vertetxNeighbours[Array_Size];
		
			//struct node vertetxGhosts[Array_Size];
			vertex.id=parentValue;
			//vertex.isfilled = false;
			vertex.color = -1;
			int countneighbour =0;
			int countghost =0;
			token = strtok(NULL, ",");
			while (token != NULL)
			{
			
				//printf("\n%d is neighbour of %d", atoi(token),parentValue);			
				nodeValue=atoi(token);
				//struct node tempvertex;
				//tempvertex.id = nodeValue;
				if((( nodeValue % N)) == mpirank){ 
					//vertetxNeighbours[countneighbour].id = tempvertex.id;
					//vertetxNeighbours[countneighbour]=tempvertex;
					
					vertex.neighbour[countneighbour] = nodeValue;
					countneighbour++;		
				}
				else {
					//vertetxGhosts[countghost].id = tempvertex.id;
					vertex.ghost[countghost]=nodeValue;
					countghost++;

					struct node ghost;
					ghost.id = nodeValue;
					//ghost.isfilled = false;
					
					ghost.color = -1;
					//if(parentValue==1){
					//	printf("\nset color for %d", nodeValue);
					//	ghost.color = 2;
					//}
					insertIntoArray(&ghostArray[ghostcount], ghost);
					ghostcount++;		
				}
				token = strtok(NULL, ",");
			}
			vertex.fillneighbour = countneighbour;
			vertex.fillghost = countghost;
			//if(vertex.id == 10){
		 	//		vertex.color = 2;
			//}
			insertIntoArray(&vertexArray[vertexcount], vertex);
			vertexcount++;
			
			//printf("\nAdding %d at location %d", vertex.id, vertexcount);
			//printV(vertex,mpirank);							
		//	printvertex(vertexArray, vertexcount, mpirank);
		}
	}
	
        fclose(fp);
	
	//printvertex(vertexArray,vertexcount,mpirank);
	
	//int totalNotColored = vertexcount;
	//while(totalNotColored != 0){
	int totalColored = 0;
	int i = 0;
	int repeat = 1;
	while(repeat == 1){
		repeat = 0;
		//if(mpirank == 0)
		//printf("\n %d       totalRun %d", mpirank, totalRun);
		for(i = 0; i < vertexcount; i++){
			int id = vertexArray[i].id;
			//if(mpirank == 0 && id == 6)
			//	printf("\nid found: %d", id);
			if(vertexArray[i].color != -1){
				continue;
			}
			int neighbourNotColored = checkAllNeighbourColored(vertexArray, vertexcount, id, mpirank, N);
			int ghostNotColored = checkAllGhostColored(vertexArray, vertexcount, id, ghostArray, ghostcount, mpirank, N);
			//if(mpirank == 0 && id == 6)
			//	printf("\nneughborNotColored: %d, ghostNot: %d, node: %d", neighbourNotColored, ghostNotColored, id);
			if( neighbourNotColored == -1 && ghostNotColored == -1 ) {
				int color = getHighestNeighbourColor(vertexArray, id, vertexcount, ghostArray, ghostcount) + 1;
				int m = 0;
				for(m = 0; m < color; m++){
					if(checkTaken(id, m, vertexArray, vertexcount, ghostArray, ghostcount) == 0){
						color = m;
					}

				}
				vertexArray[i].color = color;
				totalColored++;
				
				printf("\n       node: %d is colored:%d  \n", id, color);
				int j = 0;
				for(j = 0; j < vertexArray[i].fillghost; j++ ){
					int ghostid = vertexArray[i].ghost[j];
					if(ghostid < id){
						int uniqueTag = getUniqueTag(id, ghostid);
						MPI_Send(&color, 1, MPI_INT, ghostid % N, uniqueTag, MPI_COMM_WORLD);	
						//printf("\nSending from %d to %d with tag %d", id, ghostid, uniqueTag);
					}	
				}	
				//repeat = 1;
			}
			if(ghostNotColored != 1){
				int j = 0;
				for(j = 0; j < ghostcount; j++){
					int ghostid = ghostArray[j].id;
					if(ghostid == ghostNotColored ){
						int uniqueTag = getUniqueTag(id, ghostNotColored);
						if(checkIfUniqueTagPresent(uniqueTagArray, uniqueTagCount, uniqueTag) == 0 ){
							int c = 0;
							MPI_Irecv(&ghostArray[j].color, 1, MPI_INT, ghostNotColored % N, uniqueTag, MPI_COMM_WORLD, &reqs[reqstatCount]);
						//	printf("\nWaiting from %d to %d with tag %d", ghostNotColored, id, uniqueTag);
							reqstatCount++;
							//printf("reqstatcount %d %d", reqstatCount, mpirank);
							uniqueTagArray[uniqueTagCount] = uniqueTag;
							uniqueTagCount++;			
						}
						break;	
					}		
				}

			}
		}
		int k = 0;
		int flag = 0;
		//printf("asdasdasda %d %d", uniqueTagCount, reqstatCount);
		for(k = 0; k < reqstatCount; k++){
			flag = 0;
			MPI_Test(&reqs[k], &flag, &stats[k]);
			//printf("\nflag= %d", flag);
			//MPI_Waitsome(flag, );
			//printf("status %d",stats[k].MPI_SOURCE);
			if(flag != 1){
				repeat = 1;
			        //printf("/nNot received %d", stats[k].MPI_TAG);
				//break;				
			} else {
				//printf("Received %d", stats[k].MPI_TAG);
				//printf("\nflag = %d", flag);
			}
		}
		//printf(" \n Total run no %d %d", totalRun, mpirank);		
		if(totalColored != vertexcount){
			repeat = 1;
		}
		
	}	
	MPI_Barrier(MPI_COMM_WORLD);
  	get_timestamp(&time2);
  	double elapsed = timestamp_diff_in_seconds(time1,time2);
  	if (0 == mpirank) {
    		printf("Time elapsed is %f seconds.\n", elapsed);
  	}
	

	/*free(vertexArray);
	free(ghostArray);
	free(reqs);
	free(stats);
	free(uniqueTagArray);*/
	MPI_Finalize();
  	return 0;	
}

int checkTaken(int nodeValue, int m, struct node *vertexArray, int vertexCount, struct node *ghostArray, int ghostCount){
	int i = 0;
	for(i = 0; i < vertexCount; i++){
		if(vertexArray[i].id== nodeValue){
			int j = 0;
			for(j=0;j<vertexArray[i].fillghost;j++){
                                int ghostid = vertexArray[i].ghost[j];
                                int tempcolor = getColor(ghostArray,ghostid,ghostCount);
                                if(tempcolor == m){
                                    return 1;
                                }

                        }
			for(j=0;j<vertexArray[i].fillneighbour;j++){
                                int vertexid = vertexArray[i].neighbour[j];
                                int tempcolor = getColor(vertexArray,vertexid,vertexCount);
                                if(tempcolor == m){
                                        return 1;
                                }

                        }
			

		}	
	}
	return 0;

}


void insertIntoArray(struct node *vertexArray, struct node vertex){
	vertexArray->id = vertex.id;
	vertexArray->fillneighbour = vertex.fillneighbour;
	vertexArray->fillghost = vertex.fillghost;
	vertexArray->color = vertex.color;
	vertexArray->isfilled = vertex.isfilled;	
	memcpy(vertexArray->neighbour, vertex.neighbour, vertex.fillneighbour * sizeof(int));
	memcpy(vertexArray->ghost, vertex.ghost, vertex.fillghost * sizeof(int));	
}

int getUniqueTag(int a, int b){
	
	int x,y=0;
	if (a>b){
		y=b;
		x=a;
	}else{
		y=a;
		x=b;
	}
	
	int uniquevalue = ((x+y)/2*(x+y+1)) + x;
	return uniquevalue;	
}
	
int checkIfUniqueTagPresent(int *uniqueTagArray, int uniqueTagCount,  int uniqueTag){
	int i = 0;
	for(i = 0; i < uniqueTagCount; i++){
		if(uniqueTagArray[i]== uniqueTag){
		 	return 1;
		}
	}
	return 0;
}

int setColor(struct node *vertex, int color, int id, int vertexCount){
	int i=0;
	for(i=0;i<vertexCount;i++){
		if(vertex[i].id == id){
			// id found and color set; return 1
			vertex[i].color = color;
			return 1;
		}
	}
         return 0;                                                         
}

int getHighestNeighbourColor(struct node *vertex, int id, int vertexCount, struct node *ghostArray, int ghostCount){
	int highestGhost = gethighestghostcolor(vertex, id, vertexCount, ghostArray, ghostCount);
	int highestNeighbour = gethighestneighbourcolor(vertex, id, vertexCount);
	if (highestGhost > highestNeighbour){
		return highestGhost;
	} else {
		return highestNeighbour;
	}
}

int gethighestghostcolor(struct node *vertex, int id, int vertexCount, struct node *ghostArray, int GhostCount){
	
	int i=0;
	int j=0;
	int highestcolor =-1;
	for(i=0;i<vertexCount;i++){
		if(vertex[i].id == id){
			for(j=0;j<vertex[i].fillghost;j++){
				int ghostid = vertex[i].ghost[j];
				int tempcolor = getColor(ghostArray,ghostid,GhostCount);
				if(tempcolor > highestcolor){
					highestcolor = tempcolor;
				}
										
			}
			break;				
		}
	}
	return highestcolor;
}

int gethighestneighbourcolor(struct node *vertex, int id, int vertexCount){

        int i=0;
        int j=0;
        int highestcolor =-1;
        for(i=0;i<vertexCount;i++){
                if(vertex[i].id == id){
                        for(j=0;j<vertex[i].fillneighbour;j++){
                                int neighbourid = vertex[i].neighbour[j];
                                int tempcolor = getColor(vertex,neighbourid,vertexCount);
                                if(tempcolor > highestcolor){
                                        highestcolor = tempcolor;
                                }

                        }
                        break;
                }
        }
	return highestcolor;
}


int getColor(struct node *vertex, int id, int vertexCount){
	int i=0;
	//printf("inside go color::::%d",vertex[0].id);
	for(i=0;i<vertexCount;i++){
		if(vertex[i].id == id){
			return vertex[i].color;
		}
	}
	printf("Boom!!!!!!!!!!!!!!!!!!!");
	return -2;
}

int checkAllGhostColored(struct node *vertex, int vertexCount, int  id, struct node *ghostArray, int GhostCount, int mpirank, int N){

	int i=0;
	int j=0;
	int k=0;
	if(id % N != mpirank){
		printf("BOOM");
		return 0;
	}
	//printf("\nInsdecheckAllGosts");
	//printf("\n FirstVertex pased:%d",vertex[0].id);
	for(i=0; i<vertexCount;i++){
		if(vertex[i].id==id){
			
			for(j=0;j<vertex[i].fillghost;j++){
				int ghostid = vertex[i].ghost[j];
				
				if(ghostid > id && getColor(ghostArray,ghostid,GhostCount) == -1){
					//printf("\nghost %d not colored", ghostid);
					return ghostid;
				}		
			}
			break;
		}
	}
	// return all colored
	return -1;

}

int checkAllNeighbourColored(struct node *vertex, int vertexCount, int id, int mpirank, int N){

	int i=0;
	int j=0;
	if(id % N != mpirank){
		printf("BOOM");
		return 0;			
	}
	for(i=0; i<vertexCount; i++){
		if(vertex[i].id==id){
			for(j=0;j<vertex[i].fillneighbour;i++){
				int neighbourid = vertex[i].neighbour[j];
	
				if(neighbourid > id && getColor(vertex,neighbourid,vertexCount) == -1){
					return neighbourid;
				}
			}	
		}
	}	
	return -1;		
}

void printV(struct node vertex, int mpirank){
	//printf("\nVertexId:%d",vertex.id);
	int i=0;
	printf("\n");
	for(i=0;i<vertex.fillneighbour;i++){
		printf(",[%d] n [%d]: %d,",vertex.neighbour[i],vertex.id, mpirank);
	}
	for(i=0;i<vertex.fillghost;i++){
                printf(",[%d] g [%d]: %d",vertex.ghost[i],vertex.id, mpirank);
        }
}

void printvertex(struct node *vertex, int vertexcount, int mpirank){
	//printf("Vertex ID::::::::::::::::::::::::: %d\n",vertex[0].id);
	//struct Array neighbours= *(vertex.neighbour);
	//printf("\tthe value of my_pointer is %p\n", vertex.neighbour);
	//printf("test1");
	int i=0,j=0;
	for (j=0;j<vertexcount;j++){
		printV(vertex[j],mpirank);
		/*
 		*printf("Vertex ID::::::::::::::::::::::::: %d\n",vertex[j].id);
		for (i=0;i<vertex[j].fillneighbour;i++){	
			printf("MPIrank=>%d::::::::::::neighbour::::::::::::::::::: %d\n",mpirank,vertex[j].neighbour[i].id);
		
		}
		for (i=0;i<vertex[j].fillghost;i++){
                	printf("MPIrank=>%d::::::::::::ghost::::::::::::::::::: %d\n",mpirank,vertex[j].ghost[i].id);

        	}*/
	}
		
}
