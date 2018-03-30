#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

/* Constantes sistema unix */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* Funciones control de cambio entre generaciones */
unsigned char isInside(int row, int col, int maxRow, int maxCol);
unsigned char cellsAround(unsigned char cells[ROWS][COLS], int row, int col);
unsigned char cellBorn(unsigned char cells[ROWS][COLS], int row, int col);
unsigned char cellDies(unsigned char cells[ROWS][COLS], int row, int col);
void cellsStateUpdate(unsigned char actual[ROWS][COLS], unsigned char new[ROWS][COLS], unsigned char changes[ROWS][COLS]);

/* Funciones de interaccion con el usuario IO */
void printScreen(unsigned char cells[ROWS][COLS], unsigned int stage);
void clearScreen(void);
unsigned char getInput(char *str);
unsigned char commandFinder(char *str, char *command, int *value);
unsigned int getNewCells(unsigned char cells[ROWS][COLS]);

/* Funciones generales */
void createNewCells(unsigned char cells[ROWS][COLS]);
void copyArray(char *from, char *to, int length);
void fixChanges(unsigned char cells[ROWS][COLS]);

int main(void)
{
  unsigned char cells[ROWS][COLS];
  int seed;

  /* Inicializacion de parametros para el programa */
  seed = time(NULL);
  srandom(seed);
  createNewCells(cells);
  
  clearScreen();
  printScreen(cells, 0);

  return 0;
}

/* Definicion de funciones */
void createNewCells(unsigned char cells[ROWS][COLS])
{
  /*
  Esta funcion crea una matriz 2D
  con celulas iniciales ubicadas
  de forma aleatoria
  */

  unsigned int i, j;

  for(i = 0 ; i < ROWS ; i++)
  {
    for(j = 0 ; j < COLS ; j++)
    {
      if( rand() % 2 ){
        cells[i][j] = CELL_ALIVE;
      }else{
        cells[i][j] = CELL_DEAD;
      }
    }
  }
}

void printScreen(unsigned char cells[ROWS][COLS], unsigned int stage)
{
  /*
  Esta funcion imprime en pantalla el mundo
  2D de celulas del juego  con el formato
  que corresponde
  */

  unsigned int i, j;

  printf("\t\t\t\t[EL JUEGO DE LA VIDA]\n");
  printf("Generacion N°: %d\n\n", stage);

  for(i = 0 ; i < ROWS ; i++)
  {
    for(j = 0 ; j < COLS ; j++)
    {
      printf("|");
      switch( cells[i][j] ){
        case CELL_DEAD:
          printf("   ");
          break;
        case CELL_BORN:
          printf(ANSI_COLOR_GREEN " * " ANSI_COLOR_RESET);
          break;
        case CELL_DYING:
          printf(ANSI_COLOR_RED " * " ANSI_COLOR_RESET);
          break;
        case CELL_ALIVE:
          printf(" * ");
          break;
      }
    }
    printf("|\n");
  }

  printf("\n\n");
  printf("Celula *: Viva\t\t" ANSI_COLOR_RED "Celula *: Acaba de morir\t" ANSI_COLOR_GREEN "Celula *: Acaba de nacer" ANSI_COLOR_RESET "\n");
}

void clearScreen(void)
{
  /*
  Limpia la pantalla de la terminal
  */

  system("clear");
}

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

unsigned char cellBorn(unsigned char cells[ROWS][COLS], int row, int col)
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

unsigned char cellDies(unsigned char cells[ROWS][COLS], int row, int col)
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

void cellsStateUpdate(unsigned char actual[ROWS][COLS], unsigned char future[ROWS][COLS], unsigned char changes[ROWS][COLS])
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

unsigned char cellsAround(unsigned char cells[ROWS][COLS], int row, int col)
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
