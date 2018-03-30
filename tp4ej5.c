#include<stdio.h>
#include<stdlib.h>


#define DESPLROW 1		//DESPLROW es el desplazamiento que tengo entre una fila y la otra 
#define DESPLCOL 1		//DESPLCOL es el desplazamiento que tengo entre una celda y la otra
#define ROWS 3
#define COLS 3
#define CELL_ALIVE 0xFF
#define CELL_DEAD 0x00
#define CELL_BORN 0xF0
#define CELL_DYING 0x0F


unsigned char verifico(char cells[ROWS][COLS], int row, int col);
void cellsStateUpdate(char actual[ROWS][COLS], char future[ROWS][COLS], char changes[ROWS][COLS]);

int main()
{
	int numero1,/* numero2=2, numero3=0, numero4=0, */i, j;
	char actual[ROWS][COLS], future[ROWS][COLS], changes[ROWS][COLS];
	
	numero1= rand() %2;
	
	for(i=0;i<ROWS;i++)
	{
		for(j=0;j<COLS;j++)
		{
			numero1= rand() %2;
			actual[i][j]=numero1;
			printf("%d\t", actual[i][j]);
			//printf("|\t");
		}
		printf("\n");
	}

	
	
	printf("____________\n");
	
	cellsStateUpdate(actual, future, changes);

	for(i=0;i<ROWS;i++)
	{
		for(j=0;j<COLS;j++)
		{
			printf("%d\t", future[i][j]);
		}
		printf("\n");
	}

	printf("____________\n");

	for(i=0;i<ROWS;i++)
	{
		for(j=0;j<COLS;j++)
		{
			printf("%d\t", changes[i][j]);
		}
		printf("\n");
	}

	return 0;
}

void cellsStateUpdate(char actual[ROWS][COLS], char future[ROWS][COLS], char changes[ROWS][COLS])
{
	unsigned char countPeriphereal;	//countPeriphereal se usa para contar la cantidad de celdad adyacentes vivas de la celda en estudio 
	int row, col;			//row y col se usan como indices para recorrer la matriz
	
	for(row=0 ; row<ROWS ; row++)					//Mediante este bucle for me muevo por las filas
	{
		for(col=0 ; col<COLS ; col++)				//Mediante este bucle for me muevo por las columnas
		{	
			countPeriphereal= verifico(actual, row, col);
			if(countPeriphereal<2 || countPeriphereal>3)
			{
				future[row][col]=0;	//si la celda esta rodeada de mas de 3 o menos de 2 celdas vivas, muere
				changes[row][col] = ((actual[row][col]==0) ? CELL_DEAD : CELL_DYING); //dependiendo del estado anterior de la celda en estudio, si estaba viva indico en la matriz de cambio que la celda muere, sino indico que sigue muerta
			}
			else if(countPeriphereal==3)
			{
				future[row][col]=1;	//si tiene 3 celdas vecinas vivas (exactas), entonces la celda futura vive
				changes[row][col] = ((actual[row][col]==1) ? CELL_ALIVE : CELL_BORN);	//dependiendo del estado anterior de la celda en estudio, si estaba muerta indico en la matriz de cambio que la celda revive, sino indico que sigue viva
			}
			else		
			{
				future[row][col] = actual[row][col];	//si tiene 2 celdas vivas entonces mantiene el estado
				changes[row][col] = ((future[row][col]==1) ? CELL_ALIVE : CELL_DEAD);	//indico en la matriz de cambio que la celda mantiene el mismo valor
			}
		}
	}
}

unsigned char verifico(char cells[ROWS][COLS], int row, int col)
	{
		unsigned char countPeriphereal=0;	//countPeriphereal es un contador que contiene la cantidad de celdas vivas alrededor de la celda en estudio.
		char nextRow=DESPLROW, nextCol=DESPLCOL, value;	//nextRow y nextCol son variables que se utilizan para para de una fila a su adyacente y de una columna a su adyacente respectivamente	
		int offsetRow=0, offsetCol=0;		//estas variables se usan para moverme por la matriz de manera indexada respecto de la fila y columna que estoy analizando
		for(nextRow=DESPLROW ; (nextRow>=-1) ; nextRow--)
		{
			for(nextCol=DESPLCOL ; (nextCol>=-1) ; nextCol--)
			{
				offsetRow=row+nextRow;			//ajusto el ofsset de la fila
				offsetCol=col+nextCol;			//ajusto el offset de la columna

/* Las condiciones dentro del bloque if que viene a continuacion verifican que:
-El indice de fila y columna que quiero analizar no se vaya del rango (este dentro de la matriz)
-Solo se fije en las celdas adyacentes a la celda que quiero analizar
*/

			if(((offsetRow>=0 && offsetRow<ROWS) && (offsetCol>=0 && offsetCol<COLS)) && ((offsetRow!=row && offsetCol==col) || (offsetRow==row && offsetCol!=col) || (offsetRow!=row && offsetCol!=col)))
				{
				value= cells[offsetRow][offsetCol];	//leo el valor de la celda (puede ser cualquier cosa
				countPeriphereal+= (value!=0 ? 1 : 0);	//si la celda "leida" esta viva incremento el contador
				}
			}
		}
		
		return countPeriphereal;
	}
