
#include <iostream>
#include "MCC.h"
#include <string.h>

#define MAX_MINUCIAS 96

using namespace std;

int main(int argc, char * argv[])
{
//	MCC a2;
	int num_minucias, i, I, j, num;
	float numF;

	if(argc < 2){
			cout << "Usage: MCC <fingerprint1>  -N {8|16} -C {LSS|LSSR|LSA|LSAR|LGS|NHS} [-H] [-B]" << endl;
			return 0;
	}

	MCC::configureAlgorithm(argc, argv);

	Matrix<int> M_xyt(1, 3);
	Matrix<float> M_MCC(1, 3);
	vector<MCC> BD_MCC;

	BD_MCC.reserve(4);//Reservando memoria para los objetos MCC

	for (I=0; I < 2; I++)
	{
		scanf("%d", &num_minucias);
		printf("\nnum_minucias = %d\n", num_minucias);

		M_xyt.resize(num_minucias, 3); //Es 3 columnas porque se lee x,y,t y no la calidad de la minucia porque no es necesario para el MCC.

		for (i=0; i<num_minucias; i++)
		{
			for (j=0; j<4; j++)
			{
				scanf("%d", &num);
				if (j < 3)//Esto es para no guardar el parametro de calidad
					M_xyt[i][j] = num;
			}
		}

		M_MCC.resize(num_minucias, 384);
		for (i=0; i<num_minucias; i++)
		{
			for (j=0; j<384; j++)
			{
				scanf("%f", &numF);
				M_MCC[i][j] = numF;

			}
//			if (i == num_minucias-1)
//				printf("\nUltimo dato leido = %f\n", numF);
		}
/*
		for (i=0; i<num_minucias; i++)
		{
			for (j=0; j<384; j++)
			{
				scanf("%f", &numF);
				M_MCC[i][j] = numF;
			}
		}
*/
		BD_MCC.push_back(MCC(M_xyt, M_MCC));
//		BD_MCC[I].printCylinders(cout);
	}

	for (i=1; i < 2; i++)
	{

		//BD_MCC[1].printCylinders(cout);
		cout << endl << "match = " << BD_MCC[0].match(BD_MCC[i]) << endl;

	}
		
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



