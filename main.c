#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Constantes del juego de la vida */

#define ROWS        15    // Cantidad de filas del mundo 2D
#define COLS        15    // Cantidad de columnas del mundo 2D
#define CELL_ALIVE  0xFF  // Cuando una celula esta viva
#define CELL_DEAD   0x00  // Cuando una celula esta muerta
#define CELL_BORN   0xF0  // Cuando una celula acaba de nacer
#define CELL_DYING  0x0F  // Cuando una celula acaba de morir

#define ACTUAL_CELLS  0
#define FUTURE_CELLS  1
#define CHANGED_CELLS 2

#define ERROR    -1       // Como se devuelven errores
#define NOT_ERROR 0
#define TRUE      1
#define FALSE     0

#define SHOW_GENERATION 0
#define SHOW_CHANGES    1

/* Constantes sistema unix */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define COMMAND_LINE       "\x1b[30;47m"

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
void copyArray(unsigned char from[ROWS][COLS], unsigned char to[ROWS][COLS]);
void fixChanges(unsigned char cells[ROWS][COLS]);
void cellsInit(unsigned char cells[ROWS][COLS]);
void cmdLine(void);

int main(void)
{
  unsigned char cells[3][ROWS][COLS];
  int seed;
  unsigned int stage = 1;
  char c, step = SHOW_GENERATION;

  /* Inicializacion de parametros para el programa */
  seed = time(NULL);
  srandom(seed);
  createNewCells(cells[ACTUAL_CELLS]);
  cellsInit(cells[FUTURE_CELLS]);
  copyArray(cells[ACTUAL_CELLS], cells[CHANGED_CELLS]);

  do
  {
    clearScreen();
    switch( step )
    {
      case SHOW_GENERATION:
        printScreen(cells[ACTUAL_CELLS], stage++);
        cellsStateUpdate(cells[ACTUAL_CELLS], cells[FUTURE_CELLS], cells[CHANGED_CELLS]);
        copyArray(cells[FUTURE_CELLS], cells[ACTUAL_CELLS]);
        step = SHOW_CHANGES;
        break;
      case SHOW_CHANGES:
        printScreen(cells[CHANGED_CELLS], stage);
        fixChanges(cells[CHANGED_CELLS]);
        step = SHOW_GENERATION;
        break;
    }
    c = getchar();
  }while( c != 'Q' );

  return 0;
}

/* Definicion de funciones */
void cmdLine(void)
{
  unsigned int i;

  printf("\n\n" COMMAND_LINE);
  for(i = 0 ; i < 80 ; i++)
  {
    printf(" ");
  }
  printf("\033[%d;0H", 9+ROWS);
}

void fixChanges(unsigned char cells[ROWS][COLS])
{
  /*
  Toma la matriz con los cambios
  y a aquellas celulas que estan en cambio
  la fija de forma permanente
  en viva o muerta
  */

  unsigned int i, j;

  for(i = 0 ; i < ROWS ; i++)
  {
    for(j = 0 ; j < COLS ; j++)
    {
      if( cells[i][j] == CELL_BORN )
      {
        cells[i][j] = CELL_ALIVE;
      }else if( cells[i][j] == CELL_DYING )
      {
        cells[i][j] = CELL_DEAD;
      }
    }
  }
}

void cellsInit(unsigned char cells[ROWS][COLS])
{
  /*
  Inicializa un mundo 2D con celulas muertas.
  */

  unsigned int i, j;

  for(i = 0 ; i < ROWS ; i++)
  {
    for(j = 0 ; j < COLS ; j++)
    {
      cells[i][j] = CELL_DEAD;
    }
  }
}

void copyArray(unsigned char from[ROWS][COLS], unsigned char to[ROWS][COLS])
{
  /*
  Copia un arreglo a otro,
  se puede aplicar a cualquier dimension
  de arreglos simplemente calculando bien
  el largo
  */

  unsigned int i,j;

  for(i = 0 ; i < ROWS ; i++)
  {
    for(j = 0 ; j < COLS ; j++)
    {
      to[i][j] = from[i][j];
    }
  }
}

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
        default:
          printf("%x", cells[i][j]);
          break;
      }
    }
    printf("|\n");
  }

  printf("\n\n");
  printf("Celula *: Viva\t\t" ANSI_COLOR_RED "Celula *: Acaba de morir\t" ANSI_COLOR_GREEN "Celula *: Acaba de nacer" ANSI_COLOR_RESET "\n");

  cmdLine();
  printf(ANSI_COLOR_RESET);
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
				}else
        {
					future[row][col] = CELL_ALIVE;
        }
			}else{
				if( cellBorn(actual, row, col) )
				{
					future[row][col] = CELL_ALIVE;
					changes[row][col] = CELL_BORN;
				}else
        {
					future[row][col] = CELL_DEAD;
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
	int nextRow, nextCol;		//nextRow y nextCol son variables que se utilizan para para de una fila a su adyacente y de una columna a su adyacente respectivamente

	for(nextRow = 1+row ; (nextRow >= row-1) ; nextRow--)
	{
		for(nextCol = 1+col ; (nextCol >= col-1) ; nextCol--)
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
