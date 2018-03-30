#include <stdio.h>

/* Constantes del juego de la vida */

#define DESPLROW 1		    //DESPLROW es el desplazamiento que tengo entre una fila y la otra
#define DESPLCOL 1		    //DESPLCOL es el desplazamiento que tengo entre una celda y la otra
#define ROWS 10           // Cantidad de filas del mundo 2D
#define COLS 10           // Cantidad de columnas del mundo 2D
#define CELL_ALIVE  0xFF  // Cuando una celula esta viva
#define CELL_DEAD   0x00  // Cuando una celula esta muerta
#define CELL_BORN   0xF0  // Cuando una celula acaba de nacer
#define CELL_DYING  0x0F  // Cuando una celula acaba de morir

#define ERROR    -1       // Como se devuelven errores
#define NOT_ERROR 0

#define TRUE  1
#define FALSE 0

/* Funciones control de cambio entre generaciones */
unsigned char isInside(int row, int col, int maxRow, int maxCol);
unsigned char cellsAround(char cells[ROWS][COLS], int row, int col);
unsigned char cellBorn(char cells[ROWS][COLS], int row, int col);
unsigned char cellDies(char cells[ROWS][COLS], int row, int col);
void cellsStateUpdate(char actual[ROWS][COLS], char new[ROWS][COLS], char changes[ROWS][COLS]);

/* Funciones de interaccion con el usuario IO */
void printScreen(char cells[ROWS][COLS]);
void clearScreen(void);
unsigned char getInput(char *str);
unsigned char commandFinder(char *str, char *command, int *value);
unsigned int getNewCells(char cells[ROWS][COLS]);

/* Funciones generales */
unsigned int createNewCells(char cells[ROWS][COLS]);
void copyArray(char *from, char *to, int length);
void fixChanges(char cells[ROWS][COLS]);


/* Definicion de funciones */
unsigned char isInside(int row, int col, int maxRow, int maxCol)
{
	/*
	Verifica que la columna y fila
	son una coordenada interna al mundo 2D
	*/

	if( row >= 0 && row < maxRow ){
		if( col >= 0 && col < maxCol ){
			return TRUE;
		}
	}

	return FALSE;
}

unsigned char cellBorn(char cells[ROWS][COLS], int row, int col)
{
	/*
	Esta funcion se fija si la celula en row y col
	cumple condiciones para nacer
	*/

	unsigned char countPeriphereal;

	countPeriphereal = cellsAround(cells, row, col);

	if( countPeriphereal == 3 && cells[row][col] == CELL_DEAD ){
		return TRUE;
	}
	return FALSE;
}

unsigned char cellDies(char cells[ROWS][COLS], int row, int col)
{
	/*
	Esta funcion se fija si la celula en row y col
	cumple las condiciones para morir
	*/

	unsigned char countPeriphereal;

	countPeriphereal = cellsAround(cells, row, col);
	if( cells[row][col] == CELL_ALIVE && (countPeriphereal < 2 || countPeriphereal > 3) )
	{
		return TRUE;
	}
	return FALSE;
}

void cellsStateUpdate(char actual[ROWS][COLS], char future[ROWS][COLS], char changes[ROWS][COLS])
{
	int row, col;			//row y col se usan como indices para recorrer la matriz

	for(row = 0 ; row < ROWS ; row++)					//Mediante este bucle for me muevo por las filas
	{
		for(col = 0 ; col < COLS ; col++)				//Mediante este bucle for me muevo por las columnas
		{
			if( actual[row][col] == CELL_ALIVE )
			{
				if( cellDies(actual, row, col) )
				{
					future[row][col] = CELL_DEAD;
					changes[row][col] = CELL_DYING;
				}
			}else{
				if( cellBorn(actual, row, col) )
				{
					future[row][col] = CELL_ALIVE;
					changes[row][col] = CELL_BORN;
				}
			}
		}
	}
}

unsigned char cellsAround(char cells[ROWS][COLS], int row, int col)
{
	/*
	Esta funcion cuenta la cantidad de celulas vivas
	alrededor de la celda indicada por row y col
	*/
	unsigned char countPeriphereal = 0;										//countPeriphereal es un contador que contiene la cantidad de celdas vivas alrededor de la celda en estudio.
	unsigned char nextRow, nextCol;		//nextRow y nextCol son variables que se utilizan para para de una fila a su adyacente y de una columna a su adyacente respectivamente

	for(nextRow = DESPLROW+row ; (nextRow >= row-1) ; nextRow--)
	{
		for(nextCol = DESPLCOL+col ; (nextCol >= col-1) ; nextCol--)
		{
			/* Las condiciones dentro del bloque if que viene a continuacion verifican que:
			-El indice de fila y columna que quiero analizar no se vaya del rango (este dentro de la matriz)
			-Solo se fije en las celdas adyacentes a la celda que quiero analizar
			*/
			if( isInside(nextRow, nextCol, ROWS, COLS) ){
				if( nextRow != row || nextCol != col ){
					if( cells[nextRow][nextCol] == CELL_ALIVE ){
						countPeriphereal++;
					}
				}
			}
		}
	}

	return countPeriphereal;
}
