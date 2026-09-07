/*
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

#define NUM_ELEM_BD 1000
//#define NUM_ELEM_BD 180000
//#define NUM_ELEM_BD 1125000
#define NUM_ELEM_Q 10
#define TOPK 1
#define UMBRAL 0.12

//Hay que indicar qué medida se calculará. Al calcular el FRR también se calcula el TPR.
#define FRR 1
#define TN 0
#define FAR 0

using namespace std;

int main(int argc, char * argv[])
{
//	MCC a2;
	int num_minucias, i, I, j, num, *arr_id_Q, *arr_id_BD, n_elemH, acierto, desacierto;
	float numF;
	Elem *heap, h_temp;
    MYSQL *conn; 
	MYSQL_RES *res; 
	MYSQL_ROW row; 
	char server[13] = "69.69.69.201"; 
	char user[5] = "test"; 
	char password[10] = "1waxzsq2."; 
	char database[12] = "descriptors"; 
    char str[256], *token;
    struct timeval t1, t2;

	if(argc < 2){
			cout << "Usage: MCC <fingerprint1>  -N {8|16} -C {LSS|LSSR|LSA|LSAR|LGS|NHS} [-H] [-B]" << endl;
			return 0;
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
//            printf("\nLALA 1\n");
//            fflush(stdout);
                //row[0] = id
                arr_id_Q[I] = atoi(row[0]);
                //printf("\nID = %d\n", arr_id[i]);

                //row[1] = num_minucias
                num_minucias = atoi(row[1]);
				M_xyt.resize(num_minucias, 3); //Es 3 columnas porque se lee x,y,t y no la calidad de la minucia porque no es necesario para el MCC.
				M_MCC.resize(num_minucias, 384);

//            printf("\nLALA 2\n");
//            fflush(stdout);
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

//            printf("\nLALA 3 :: num_minucias = %d\n", num_minucias);
//            fflush(stdout);
				//rellenando MCC (row[3])
				for (i=0; i < num_minucias; i++)
				{
//            		printf("\tLALA 3 (i = %d) \n", i);
//            		fflush(stdout);
					for (j=0; j < 384; j++)
					{
                    	if (i == 0 && j == 0)
						{
                        	token = strtok(row[3], " ");

						}
	                    else
    	                    token = strtok(NULL, " ");

//            				printf("\t\tLALA 3 (i = %d) INI :: atof(token) = %lf\n", i, atof(token));
//            				fflush(stdout);

						M_MCC[i][j] = (float)(atof(token));

//            				printf("\t\tLALA 3 (i = %d) FIN\n", i);
//            				fflush(stdout);
					}
				}
//            printf("\nLALA 4\n");
//            fflush(stdout);
				Queries_MCC.push_back(MCC(M_xyt, M_MCC));
            }
//            printf("\nLALA 5\n");
//            fflush(stdout);

            //Liberando los resultados de mysql
            mysql_free_result(res);
    } //end if (FRR || TN)

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
	omp_set_num_threads(omp_get_num_procs()); //Numero de hilos igual al numero de nucleos
#pragma omp parallel private(I, i, h_temp, n_elemH, heap) shared(Queries_MCC, BD_MCC, mat_res)
{
	int tid = omp_get_thread_num(), nprocs = omp_get_num_threads();
	for (I=0; I < NUM_ELEM_Q; I++)
	{
		if (tid == 0)
		{
			printf("Query %d\n", I);
			fflush(stdout);
		}

		n_elemH = 0;
		//BD_MCC[1].printCylinders(cout);
		for (i=tid; i < NUM_ELEM_BD; i+=nprocs)
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
			mat_res[tid][i].ind = heap[i].ind;
			mat_res[tid][i].dist = heap[i].dist;
		}
	}
}
//		printf("\nQuery [%d]:\n", arr_id_Q[I]);
//			printf("\t\tDist. con elem[%d] = %f\n", arr_id_Q[I], Queries_MCC[I].match(BD_MCC[ arr_id_Q[I] ]));
//		fflush(stdout);
/*
		for (i=0; i < TOPK; i++)
		{
			//h_temp = extrae2E(heap, &n_elemH);
			printf("\tDist. con elem[%d] = %f\n", heap[i].ind, heap[i].dist);
			fflush(stdout);
		}
		*/

        if (FRR)
        {
            //Preguntando por acierto de huellas existentes en la BD (para calcular FRR)
            for (i=0; i < TOPK; i++)
            {
                if (heap[i].ind == arr_id_Q[I] && heap[i].dist >= UMBRAL)
                {
                    acierto++;
					printf("Acierto :: Consulta ID = %d :: ind = %d :: dist = %f\n", arr_id_Q[I], heap[i].ind, heap[i].dist);
					fflush(stdout);
                    break;
                }
                if (i+1 == TOPK)
                {
                    desacierto++;
                    printf("DESACIERTO :: Consulta ID = %d \n", arr_id_Q[I]);
					fflush(stdout);
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
        printf("\nTOPK = %d :: NUM_ELEM_Q = %d :: Acierto = %d :: Desacierto = %d :: FRR (False Rejection Rate) = %lf%% :: TPR (True Positive Rate) = %lf%% :: Tiempo = %lf (seg.)\n", TOPK, NUM_ELEM_Q, acierto, desacierto, (double)((double)desacierto/(double)NUM_ELEM_Q)*100.0, (double)((double)acierto/(double)NUM_ELEM_Q)*100.0, (double)(t2.tv_sec - t1.tv_sec) + ((double)(t2.tv_usec - t1.tv_usec)/1000000.0));

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



