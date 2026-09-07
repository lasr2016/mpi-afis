#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char * argv[])
{
    int i,j, id_dir_ini, id_dir_fin, id_dir, largo_ini, largo_fin;
    char str[512], str2[512], str3[512], str_copy_1[512], str_copy_2[512];
    int ID = 5;

                    sprintf(str, "scp ~/nas1-HuellasDB72M/");

                    id_dir = (int)(ID / 50000);
                    id_dir_ini = (id_dir * 50000)+1;
                    id_dir_fin = (id_dir + 1)*50000;

                    sprintf(str2, "%d", id_dir_ini);
                    //largo_ini es la cantidad de ceros antes del numero ID del directorio de la izquierda
                    largo_ini = strlen(str2);
                    largo_ini = 10 - largo_ini;

                    str3[0]='\0';
                    for (j=0; j < largo_ini; j++)
                       strcat(str3, "0");
                    strcat(str, str3);
                    strcat(str, str2);
                    strcat(str, "-");

                    sprintf(str2, "%d", id_dir_fin);
                    //largo_fin es la cantidad de ceros antes del numero ID del directorio de la derecha
                    largo_fin = strlen(str2);
                    largo_fin = 10 - largo_fin;

                    str3[0]='\0';
                    for (j=0; j < largo_fin; j++)
                       strcat(str3, "0");
                    strcat(str, str3);
                    strcat(str, str2);
                    strcat(str, "/Huellas-72M_");

                    sprintf(str2, "%d", ID);
                    strcat(str, str2);

                    strcpy(str_copy_1, str);
                    strcpy(str_copy_2, str);
                    strcat(str_copy_1, "_1.wsq ~/Huellas_malas_MCC/");
                    strcat(str_copy_2, "_2.wsq ~/Huellas_malas_MCC/");

                    printf("%s\n", str_copy_1);
                    printf("%s\n", str_copy_2);

                    return 0;
}
