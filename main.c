#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Constantes del juego de la vida */
#define MAX_CMD     2     // Maximas palabras en comando
#define MAX_LENGTH  20    // Maximo largo de comandos
#define ROWS        15    // Cantidad de filas del mundo 2D
#define COLS        15    // Cantidad de columnas del mundo 2D
#define CELL_ALIVE  0xFF  // Cuando una celula esta viva
#define CELL_DEAD   0x00  // Cuando una celula esta muerta
#define CELL_BORN   0xF0  // Cuando una celula acaba de nacer
#define CELL_DYING  0x0F  // Cuando una celula acaba de morir

/* Comandos */
#define CMD_NONE    0
#define CMD_ROWS    1
#define CMD_COLS    2
#define CMD_START   3
#define CMD_RESTART 4
#define CMD_EXIT    5
#define CMD_CHANGES 6
#define CMD_AUTO    7

/* Identificadores de matriz 2D en la 3D */
#define ACTUAL_CELLS  0
#define FUTURE_CELLS  1
#define CHANGED_CELLS 2

#define ERROR          -1       // Como se devuelven errores
#define NOT_ERROR       0
#define TRUE            1
#define FALSE           0

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
void clearBuffer(void);

/* Funciones generales */
void createNewCells(unsigned char cells[ROWS][COLS]);
void copyArray(unsigned char from[ROWS][COLS], unsigned char to[ROWS][COLS]);
void fixChanges(unsigned char cells[ROWS][COLS]);
void cellsInit(unsigned char cells[ROWS][COLS]);

/* Funciones para linea de comando */
void cmdLine(void);
int splitStr(char words[][MAX_LENGTH], char *str, char separator, int max);
unsigned char onlyNumbers(char *str);
unsigned char onlyLetters(char *str);
unsigned char isNumber(char c);
unsigned char isLetter(char c);
unsigned char commandFinder(char *str, char cmdList[][MAX_LENGTH], int cmd_length);
void readConsole(int *cmd_id, int *arg);
unsigned int toNumber(char *str);

int main(void)
{
  unsigned char cells[3][ROWS][COLS], cellBoard[ROWS][COLS];
  int seed, cmd_id = 0, arg = 0;
  unsigned int stage = 0;
  unsigned char finish = FALSE, showChanges = FALSE;

  /* Inicializacion de parametros para el programa */
  seed = time(NULL);
  srandom(seed);
  cellsInit(cells[FUTURE_CELLS]);
  cellsInit(cells[CHANGED_CELLS]);
  cellsInit(cells[ACTUAL_CELLS]);
  cellsInit(cellBoard);

  while( !finish )
  {
    clearScreen();
    printScreen(cellBoard, stage);
    readConsole(&cmd_id, &arg);
    clearBuffer();

    switch( cmd_id ){
      case CMD_EXIT:
        finish = TRUE;
        break;
      case CMD_START: case CMD_RESTART:
        createNewCells(cells[ACTUAL_CELLS]);
        cellsInit(cells[FUTURE_CELLS]);
        cellsInit(cells[CHANGED_CELLS]);
        copyArray(cells[ACTUAL_CELLS], cellBoard);
        stage = 1;
        break;
      default:
        if( !showChanges )
        {
          cellsStateUpdate(cells[ACTUAL_CELLS], cells[FUTURE_CELLS], cells[CHANGED_CELLS]);
          copyArray(cells[FUTURE_CELLS], cells[ACTUAL_CELLS]);
          copyArray(cells[CHANGED_CELLS], cellBoard);
          stage++;
        }else
        {
          fixChanges(cells[CHANGED_CELLS]);
          copyArray(cells[ACTUAL_CELLS], cellBoard);
        }
        showChanges = ~showChanges;
        break;
    }
  }

  clearScreen();
  return 0;
}

/* Definicion de funciones */
void clearBuffer(void)
{
  /*
  Limpia el buffer de ingreso
  por teclado
  */

  while( getchar() != '\n' );
}

unsigned int toNumber(char *str)
{
  /*
  Convierte el string con numeros ascii
  en una variable entera con el mismo numero
  */

  unsigned int number = 0;
  unsigned char c;

  while( *str )
  {
      c = *str++ - '0';
      number *= 10;
      number += c;
  }

  return number;
}

void readConsole(int *cmd_id, int *arg)
{
  /*
  Esta funcion espera el ingreso
  del usuario de un texto y lo procesa
  para ver si existe o no un comando.
  Devuelve ERROR si no hay comando.
  */

  char input[MAX_LENGTH], cmd_input[MAX_CMD][MAX_LENGTH];
  char commands[][MAX_LENGTH] = {"","rows", "cols", "start", "restart", "exit", "changes", "auto"};
  char argExpected[] = {0, 1, 1, 0, 0, 0, 1, 1};
  int numberOfWords;

  scanf("%[^\n]", input);   // Espero texto hasta que aprete enter

  numberOfWords = splitStr(cmd_input, input, ' ', MAX_CMD); // Lo separo en partes por espacios

  if( numberOfWords != ERROR )
  {
    if( onlyLetters(&cmd_input[0][0]) )
    {
      *cmd_id = commandFinder(&cmd_input[0][0], commands, 8);
      if( *cmd_id != CMD_NONE )
      {
        if( numberOfWords > 1 && argExpected[*cmd_id] )
        {
          if( onlyNumbers(&cmd_input[1][0]) )
          {
            *arg = toNumber(&cmd_input[1][0]);
          }else
          {
            *cmd_id = CMD_NONE;
          }
        }else if( numberOfWords > 1 && !argExpected[*cmd_id] )
        {
          *cmd_id = CMD_NONE;
        }else if( numberOfWords == 1 && argExpected[*cmd_id]  )
        {
          *cmd_id = CMD_NONE;
        }
      }
    }else
    {
      *cmd_id = CMD_NONE;
    }
  }
}

unsigned char commandFinder(char *str, char cmdList[][MAX_LENGTH], int cmd_length)
{
  /*
  Se fija si el string es uno de los
  comandos que estan en la lista de
  comandos y devuelve su identificador
  */

  unsigned int i, j;
  unsigned char found = FALSE;

  for(i = 0 ; i < cmd_length && !found ; i++)
  {
    j = 0;
    while( str[j] && cmdList[i][j] && str[j] == cmdList[i][j] )
    {
      j++;
    }
    if( !str[j] && !cmdList[i][j] )
    {
      found = TRUE;
    }
  }

  if( i >= cmd_length )
  {
    return CMD_NONE;
  }else
  {
    return i-1;
  }
}

unsigned char isNumber(char c)
{
  /*
  Compruebo que sea
  un numero en ascii
  */

  return (c >= '0' && c <= '9');
}

unsigned char isLetter(char c)
{
  /*
  Compruebo que sea una
  letra en ascii
  */

  if( c < 'a' && c > 'z' )
  {
    if( c < 'A' && c > 'Z' )
    {
      return FALSE;
    }
  }
  return TRUE;
}

unsigned char onlyNumbers(char *str)
{
  /* Compruebo que
  el string se componga unicamente
  de numeros */

  unsigned char check = TRUE;

  while( *str && check )
  {
    check = isNumber( *str++ );
  }

  return check;
}

unsigned char onlyLetters(char *str)
{
  /* Compruebo que el
  string contenga solamente
  letras en ascii */

  unsigned char check = TRUE;

  while( *str && check )
  {
    check = isLetter( *str++ );
  }

  return check;
}

int splitStr(char words[][MAX_LENGTH], char *str, char separator, int max)
{
  /*
  Separa un string en varios string
  dentro de una matriz separandolo segun
  un separador.
  Si supera maximo de palabras, error.
  Si algun string supera el largo maximo, error.
  */

  unsigned int i = 0, j = 0;
  while( *str && i < max && j < MAX_LENGTH )
  {
    if( *str == separator )
    {
      words[i++][j] = '\0';
      j = 0;
    }else
    {
      words[i][j++] = *str;
    }
    str++;
  }

  if( i >= max || j >= MAX_LENGTH )
  {
    return ERROR;
  }else
  {
    words[i][j] = '\0';
    return i+1;
  }
}

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
