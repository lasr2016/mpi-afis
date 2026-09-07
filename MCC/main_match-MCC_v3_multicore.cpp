/*
 * v3: Implementacion multi-hilo con OpenMP
 *
 * Esta versión se conecta con una base de datos mysql para cargar la BD y las consultas
 * También, utiliza un conjunto de hilos para resolver las consultas
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <mysql.h>
#include <omp.h>
#include "MCC.h"
#include <string.h>
#include "heap_desde_elem-0.h"

#define MAX_MINUCIA 96
#define MIN_MINUCIA 5
#define N_THREADS 36

//#define NUM_ELEM_BD 1000
//#define NUM_ELEM_BD 100000
#define NUM_ELEM_BD 50000
//#define NUM_ELEM_BD 180000
//#define NUM_ELEM_BD 1125000
//#define NUM_ELEM_Q 10
#define NUM_ELEM_Q 1000
#define TOPK 1
#define UMBRAL 0.09
//#define UMBRAL 0.14
//#define UMBRAL 0.10
//#define UMBRAL 0.21

//Hay que indicar qué medida se calculará. Al calcular el FRR también se calcula el TPR.
#define FRR 0
#define TN 1
#define FAR 0

using namespace std;

int main(int argc, char * argv[])
{
//	MCC a2;
	int num_minucias, i, I, j, num, *arr_id_Q, *arr_id_BD, n_elemH, acierto, desacierto, num_elem_q_g1, num_elem_q_g2, id_elem_medio;
	float numF;
	Elem h_temp;
    MYSQL *conn; 
	MYSQL_RES *res; 
	MYSQL_ROW row; 
	char server[13] = "69.69.69.201"; 
	char user[5] = "test"; 
	char password[10] = "1waxzsq2."; 
	char database[12] = "descriptors"; 
    char str[256], *token;
    struct timeval t1, t2;
	Elem ***mat_res;
	Elem *heap;

	if(argc < 2){
			cout << "Usage: MCC <fingerprint1>  -N {8|16} -C {LSS|LSSR|LSA|LSAR|LGS|NHS} [-H] [-B]" << endl;
			return 0;
	}

	mat_res = (Elem ***)malloc(sizeof(Elem **)*NUM_ELEM_Q);
	for (I=0; I<NUM_ELEM_Q; I++)
	{
		mat_res[I] = (Elem **)malloc(sizeof(Elem *)*N_THREADS);
		for (i=0; i < N_THREADS; i++)
			mat_res[I][i] = (Elem *)malloc(sizeof(Elem)*TOPK);
	}
	heap = (Elem *)malloc(sizeof(Elem)*TOPK);
	arr_id_Q = (int *)malloc(sizeof(int)*NUM_ELEM_Q);
	arr_id_BD = (int *)malloc(sizeof(int)*NUM_ELEM_BD);

	MCC::configureAlgorithm(argc, argv);

	Matrix<int> M_xyt(1, 3);
	Matrix<float> M_MCC(1, 3);
	vector<MCC> BD_MCC;
	vector<MCC> Queries_MCC;

	BD_MCC.reserve(NUM_ELEM_BD);//Reservando memoria para los objetos MCC
	Queries_MCC.reserve(NUM_ELEM_Q);//Reservando memoria para los objetos MCC


    conn = mysql_init(NULL); 
    // Connect to database  
    if (!mysql_real_connect(conn, server, user, password, database, 3307, NULL, 0)) 
    { 
        fprintf(stderr, "%s\n", mysql_error(conn)); 
        exit(1); 
    } 

    //Para calculo del FRR:
	//Rellenando las Consultas
    if (FRR || TN)
    {
            if (FRR)
                sprintf(str, "SELECT id, num_minucias, xyt, MCC FROM tbdescriptors_toma2 WHERE num_minucias <= %d and num_minucias > %d LIMIT %d", MAX_MINUCIA, MIN_MINUCIA, NUM_ELEM_Q);
            else if (TN) //Para el calculo de TN se debe contar con consultas que no estan en la BD:
                sprintf(str, "SELECT id, num_minucias, xyt, MCC FROM tbdescriptors_toma2 WHERE num_minucias <= %d and num_minucias > %d and id >= 400000 LIMIT %d", MAX_MINUCIA, MIN_MINUCIA, NUM_ELEM_Q);

            printf("\nEjecutando: %s\n\n", str);
            fflush(stdout);
            if (mysql_query(conn, str)) 
            { 
                fprintf(stderr, "%s\n", mysql_error(conn));  exit(1); 
            } 
            res = mysql_use_result(conn); 

            // output table name  
            //num_fields = mysql_num_fields(res);
            //ind_inicio_Q[I][0] = 0;
            for (I=0; (row = mysql_fetch_row(res)) != NULL; I++) 
            {
                //row[0] = id
                arr_id_Q[I] = atoi(row[0]);
                //printf("\nID = %d\n", arr_id[i]);

                //row[1] = num_minucias
                num_minucias = atoi(row[1]);
				M_xyt.resize(num_minucias, 3); //Es 3 columnas porque se lee x,y,t y no la calidad de la minucia porque no es necesario para el MCC.
				M_MCC.resize(num_minucias, 384);

				//rellenando xyt (row[2])
				for (i=0; i<num_minucias; i++)
				{
					for (j=0; j<4; j++)
					{
                    	if (i == 0 && j == 0)
                        	token = strtok(row[2], " ");
	                    else
    	                    token = strtok(NULL, " ");
						if (j < 3)//Esto es para no guardar el parametro de calidad
							M_xyt[i][j] = atoi(token);
					}
				}

				//rellenando MCC (row[3])
				for (i=0; i < num_minucias; i++)
				{
					for (j=0; j < 384; j++)
					{
                    	if (i == 0 && j == 0)
						{
                        	token = strtok(row[3], " ");
						}
	                    else
    	                    token = strtok(NULL, " ");

						M_MCC[i][j] = (float)(atof(token));
					}
				}
				Queries_MCC.push_back(MCC(M_xyt, M_MCC));
            }

            //Liberando los resultados de mysql
            mysql_free_result(res);
    } //end if (FRR || TN)
	else if (FAR)
	{
        //Para calculo del FAR se debe contar con consultas que estan y que no estan en la BD:
        num_elem_q_g1 = (int)(NUM_ELEM_Q/2);
        num_elem_q_g2 = NUM_ELEM_Q - num_elem_q_g1;
                
		sprintf(str, "SELECT id, num_minucias, xyt, MCC FROM tbdescriptors_toma2 WHERE num_minucias <= %d and num_minucias > %d  LIMIT %d", MAX_MINUCIA, MIN_MINUCIA, num_elem_q_g1);
		printf("\nEjecutando: %s\n\n", str);
		fflush(stdout);

		if (mysql_query(conn, str)) 
		{ 
			fprintf(stderr, "%s\n", mysql_error(conn));  exit(1); 
		} 

		res = mysql_use_result(conn); 
        
		// output table name  
		//num_fields = mysql_num_fields(res);
		//ind_inicio_Q[I][0] = 0;
		for (I=0; (row = mysql_fetch_row(res)) != NULL; I++) 
		{
			//row[0] = id
			arr_id_Q[I] = atoi(row[0]);
			//printf("\nID = %d\n", arr_id[i]);

			//row[1] = num_minucias
			num_minucias = atoi(row[1]);

			M_xyt.resize(num_minucias, 3); //Es 3 columnas porque se lee x,y,t y no la calidad de la minucia porque no es necesario para el MCC.
			M_MCC.resize(num_minucias, 384);

			//rellenando xyt (row[2])
			for (i=0; i<num_minucias; i++)
			{
				for (j=0; j<4; j++)
				{
					if (i == 0 && j == 0)
						token = strtok(row[2], " ");
					else
						token = strtok(NULL, " ");
					if (j < 3)//Esto es para no guardar el parametro de calidad
						M_xyt[i][j] = atoi(token);
				}
			}

			//rellenando MCC (row[3])
			for (i=0; i < num_minucias; i++)
			{
				for (j=0; j < 384; j++)
				{
					if (i == 0 && j == 0)
					{
						token = strtok(row[3], " ");
					}
					else
						token = strtok(NULL, " ");
					
					M_MCC[i][j] = (float)(atof(token));
				}
			}
			Queries_MCC.push_back(MCC(M_xyt, M_MCC));
		}

		//Liberando los resultados de mysql
		mysql_free_result(res);

		id_elem_medio = arr_id_Q[I-1];

		sprintf(str, "SELECT id, num_minucias, xyt, MCC FROM tbdescriptors_toma2 WHERE num_minucias <= %d and num_minucias > %d and id > 400000 LIMIT %d", MAX_MINUCIA, MIN_MINUCIA, num_elem_q_g2);
		printf("\nEjecutando: %s\n\n", str);
		fflush(stdout);

		if (mysql_query(conn, str)) 
		{ 
			fprintf(stderr, "%s\n", mysql_error(conn));  exit(1); 
		} 

		res = mysql_use_result(conn); 
        
		// output table name  
		//num_fields = mysql_num_fields(res);
		//ind_inicio_Q[I][0] = 0;
		for ( ; (row = mysql_fetch_row(res)) != NULL; I++) 
		{
			//row[0] = id
			arr_id_Q[I] = atoi(row[0]);
			//printf("\nID = %d\n", arr_id[i]);

			//row[1] = num_minucias
			num_minucias = atoi(row[1]);

			M_xyt.resize(num_minucias, 3); //Es 3 columnas porque se lee x,y,t y no la calidad de la minucia porque no es necesario para el MCC.
			M_MCC.resize(num_minucias, 384);

			//rellenando xyt (row[2])
			for (i=0; i<num_minucias; i++)
			{
				for (j=0; j<4; j++)
				{
					if (i == 0 && j == 0)
						token = strtok(row[2], " ");
					else
						token = strtok(NULL, " ");
					if (j < 3)//Esto es para no guardar el parametro de calidad
						M_xyt[i][j] = atoi(token);
				}
			}

			//rellenando MCC (row[3])
			for (i=0; i < num_minucias; i++)
			{
				for (j=0; j < 384; j++)
				{
					if (i == 0 && j == 0)
					{
						token = strtok(row[3], " ");
					}
					else
						token = strtok(NULL, " ");
					
					M_MCC[i][j] = (float)(atof(token));
				}
			}
			Queries_MCC.push_back(MCC(M_xyt, M_MCC));
		}

		//Liberando los resultados de mysql
		mysql_free_result(res);
	}

	//Rellenando la BD
	sprintf(str, "SELECT id, num_minucias, xyt, MCC FROM tbdescriptors WHERE num_minucias <= %d and num_minucias > %d LIMIT %d", MAX_MINUCIA, MIN_MINUCIA, NUM_ELEM_BD);
    
	printf("\nEjecutando: %s\n\n", str);
	fflush(stdout);
	if (mysql_query(conn, str)) 
	{ 
		fprintf(stderr, "%s\n", mysql_error(conn));  exit(1); 
	} 
	res = mysql_use_result(conn); 
    
	// output table name  
	//num_fields = mysql_num_fields(res);
	//ind_inicio_Q[I][0] = 0;
	for (I=0; (row = mysql_fetch_row(res)) != NULL; I++) 
	{
		//row[0] = id
		arr_id_BD[I] = atoi(row[0]);
		//printf("\nID = %d\n", arr_id[i]);
        
		//row[1] = num_minucias
		num_minucias = atoi(row[1]);
		M_xyt.resize(num_minucias, 3); //Es 3 columnas porque se lee x,y,t y no la calidad de la minucia porque no es necesario para el MCC.
		M_MCC.resize(num_minucias, 384);
		
		//rellenando xyt (row[2])
		for (i=0; i<num_minucias; i++)
		{
			for (j=0; j<4; j++)
			{
				if (i == 0 && j == 0)
					token = strtok(row[2], " ");
				else
					token = strtok(NULL, " ");
				if (j < 3)//Esto es para no guardar el parametro de calidad
					M_xyt[i][j] = atoi(token);
			}
		}

		//rellenando MCC (row[3])
		for (i=0; i < num_minucias; i++)
		{
			for (j=0; j < 384; j++)
			{
				if (i == 0 && j == 0)
					token = strtok(row[3], " ");
				else
					token = strtok(NULL, " ");
				
				M_MCC[i][j] = (float)(atof(token));
			}
		}
		BD_MCC.push_back(MCC(M_xyt, M_MCC));
	}
    
	//Liberando los resultados de mysql
	mysql_free_result(res);

	printf("\nMatching...\n");
	fflush(stdout);
    gettimeofday(&t1, 0);

	n_elemH = 0;
	acierto=0;
	desacierto=0;
	//omp_set_num_threads(omp_get_num_procs()); //Numero de hilos igual al numero de nucleos
	omp_set_num_threads(N_THREADS);
#pragma omp parallel private(I, i, h_temp, n_elemH) shared(Queries_MCC, BD_MCC, mat_res)
{
	int tid = omp_get_thread_num();
	Elem *heap;

	heap = (Elem *)malloc(sizeof(Elem)*TOPK);
	for (I=0; I < NUM_ELEM_Q; I++)
	{
		if (tid == 0)
		{
			printf("Query %d\n", I);
			fflush(stdout);
		}

		n_elemH = 0;
		//BD_MCC[1].printCylinders(cout);
		for (i=tid; i < NUM_ELEM_BD; i+=N_THREADS)
		{
			h_temp.dist = Queries_MCC[I].match(BD_MCC[i]);
			h_temp.ind = arr_id_BD[i];
			if (n_elemH < TOPK)
				inserta2E_min(heap, &h_temp, &n_elemH);
			else 
				if (heap[0].dist < h_temp.dist)
					popush2E_min(heap, &n_elemH, &h_temp);
		}
		for (i=0; i < TOPK; i++)
		{
			mat_res[I][tid][i].ind = heap[i].ind;
			mat_res[I][tid][i].dist = heap[i].dist;
		}
	}
	free(heap);
}


	for (I=0; I < NUM_ELEM_Q; I++)
	{
		//Rellenando el heap final de la I-esima consulta con los resultados parciales de cada hilo
		n_elemH = 0;
        for (i=0; i < N_THREADS; i++)
		{
        	for (j=0; j < TOPK; j++)
			{
				h_temp.dist = mat_res[I][i][j].dist;
				h_temp.ind = mat_res[I][i][j].ind;
				if (n_elemH < TOPK)
					inserta2E_min(heap, &h_temp, &n_elemH);
				else 
					if (heap[0].dist < h_temp.dist)
						popush2E_min(heap, &n_elemH, &h_temp);
			}
		}

        if (FRR)
        {
            //Preguntando por acierto de huellas existentes en la BD (para calcular FRR)
            for (i=0; i < TOPK; i++)
            {
                if (heap[i].ind == arr_id_Q[I] && heap[i].dist >= UMBRAL)
                {
                    acierto++;
					printf("Query %d :: Acierto :: Consulta ID = %d :: ind = %d :: dist = %f\n", I, arr_id_Q[I], heap[i].ind, heap[i].dist);
					fflush(stdout);
                    break;
                }
                if (i+1 == TOPK)
                {
                    desacierto++;
                    printf("Query %d :: DESACIERTO :: Consulta ID = %d \n", I, arr_id_Q[I]);
					fflush(stdout);
                }
            }
        }
		else if (TN)
		{
            //Preguntando por acierto de huellas Inexistentes en la BD (para calcular TN)
            for (i=0; i < TOPK; i++)
			{
	            if (heap[i].dist >= UMBRAL)
    	        {
        	        desacierto++; //Desacierto porque ninguna consulta existe en la BD
            	    printf("DESACIERTO :: Consulta ID = %d :: heap[0].dist = %f :: heap[0].ind = %d\n", arr_id_Q[I], heap[0].dist, heap[0].ind);
                	fflush(stdout);
					break;
	            }
    	        if (i+1 == TOPK)
				{
            	    acierto++;
					printf("Query %d :: Acierto :: Consulta ID = %d :: heap[0].ind = %d :: heap[0].dist = %f\n", I, arr_id_Q[I], heap[0].ind, heap[0].dist);
				}
			}
		}
        else if (FAR)
        {
            //Preguntando por acierto si el elemento está en la BD (para calcular FAR)
            if (heap[0].ind <= id_elem_medio)
            {
                if (heap[0].dist >= UMBRAL && heap[0].ind == arr_id_Q[I])
				{
                    acierto++;
					printf("Query %d :: Acierto :: Consulta ID = %d :: heap[0].ind = %d :: heap[0].dist = %f\n", I, arr_id_Q[I], heap[0].ind, heap[0].dist);
				}
                else
				{
                    desacierto++; //Desacierto porque la consulta si existe en la BD pero no se encontró en el elemento respuesta
                    printf("DESACIERTO :: Consulta ID = %d :: heap[0].dist = %f :: heap[0].ind = %d\n", arr_id_Q[I], heap[0].dist, heap[0].ind);
				}
            }
            else //Preguntando por acierto si el elemento no está en la BD (para calcular FAR)
            {
                if (heap[0].dist >= UMBRAL)
                {
                    desacierto++; //Desacierto porque la consulta no existe en la BD
                    printf("DESACIERTO :: Consulta ID = %d :: heap[0].dist = %f :: heap[0].ind = %d\n", arr_id_Q[I], heap[0].dist, heap[0].ind);
                }
                else
				{
                    acierto++;
					printf("Query %d :: Acierto :: Consulta ID = %d :: heap[0].ind = %d :: heap[0].dist = %f\n", I, arr_id_Q[I], heap[0].ind, heap[0].dist);
				}
            }
        }
	}
	/*	
		for (i=0; i < TOPK; i++)
		{
			h_temp = extrae2E_min(heap, &n_elemH);
			printf("\tDist. con elem[%d] = %f\n", h_temp.ind, h_temp.dist);
			fflush(stdout);
		}
	*/	
	

    gettimeofday(&t2, 0);

    if (FRR)
        printf("\nUMBRAL = %f :: TOPK = %d :: NUM_ELEM_Q = %d :: Acierto = %d :: Desacierto = %d :: FRR (False Rejection Rate) = %lf%% :: TPR (True Positive Rate) = %lf%% :: Tiempo = %lf (seg.)\n", UMBRAL, TOPK, NUM_ELEM_Q, acierto, desacierto, (double)((double)desacierto/(double)NUM_ELEM_Q)*100.0, (double)((double)acierto/(double)NUM_ELEM_Q)*100.0, (double)(t2.tv_sec - t1.tv_sec) + ((double)(t2.tv_usec - t1.tv_usec)/1000000.0));
	else if (TN)
        printf("\nUMBRAL = %f :: TOPK = %d :: NUM_ELEM_Q = %d :: Acierto = %d :: Desacierto = %d :: TN (True Negatives) = %lf%% :: Tiempo = %lf (seg.)\n", UMBRAL, TOPK, NUM_ELEM_Q, acierto, desacierto, (double)((double)desacierto/(double)NUM_ELEM_Q)*100.0, (double)(t2.tv_sec - t1.tv_sec) + ((double)(t2.tv_usec - t1.tv_usec)/1000000.0));
    else if (FAR)
        printf("\nUMBRAL = %f :: TOPK = %d :: NUM_ELEM_Q = %d :: Acierto = %d :: Desacierto = %d :: FAR (False Acceptance Rate) = %lf%%\n", UMBRAL, TOPK, NUM_ELEM_Q, acierto, desacierto, (double)((double)desacierto/(double)NUM_ELEM_Q)*100.0);

//	a1(M_xyt, M_MCC);


//    M_xyt.resize(new_rows, new_columns);
/*
	if(a1.readFile(argv[1])!=0){
			cout << "Error opening fingerprint files: " << argv[1] << endl;
			return 0;
	}
	if(a2.readFile(argv[2])!=0){
			cout << "Error opening fingerprint files: " << argv[2] << endl;
			return 0;
	}
*/
	//a1.initialize();
	//a2.initialize();
	
	//cout << "First fingerprint: " << endl;
	//a1.printCylinders(cout);

	//cout << a1.match(a2) << endl;

	return 0;
}



