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
#define CMD_START   1
#define CMD_RESTART 2
#define CMD_EXIT    3
#define CMD_CHANGES 4
#define CMD_AUTO    5

/* Identificadores de matriz 2D en la 3D */
#define CURRENT_CELLS  0
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

/* Funciones control de cambio entre generaciones */
unsigned char isInside(int row, int col, int maxRow, int maxCol);
unsigned char cellsAround(unsigned char cells[ROWS][COLS], int row, int col);
unsigned char cellBorn(unsigned char cells[ROWS][COLS], int row, int col);
unsigned char cellDies(unsigned char cells[ROWS][COLS], int row, int col);
void cellsStateUpdate(unsigned char CURRENT[ROWS][COLS], unsigned char new[ROWS][COLS], unsigned char changes[ROWS][COLS]);

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
  unsigned char finish = FALSE, showChanges = TRUE, newGeneration = FALSE, autoMode = FALSE;

  /* Inicializacion de parametros para el programa */
  seed = time(NULL);
  srandom(seed);
  cellsInit(cells[FUTURE_CELLS]);
  cellsInit(cells[CHANGED_CELLS]);
  cellsInit(cells[CURRENT_CELLS]);
  cellsInit(cellBoard);

  while( !finish )
  {
    clearScreen();                    // limpio la pantalla
    printScreen(cellBoard, stage);    // e imprimo el nuevo estado de ella
    readConsole(&cmd_id, &arg);       // espero recibir ingreso de teclado
    clearBuffer();                    // limpio el buffer

    do
    {
      if( autoMode && !arg )
      {
        autoMode = FALSE;             // si repeti todo, apago auto mode
      }else if( autoMode )
      {
        arg--;                       // en auto mode, decremento la cantidad de repeticiones
      }

      switch( cmd_id ){           // segun cual sea el comando ingresado
        case CMD_AUTO:
          cmd_id = CMD_NONE;
          arg = arg*2 - 1;
          autoMode = TRUE;      // activo el modo automatico
          break;
        case CMD_CHANGES:
          if( arg )
          {
            showChanges = TRUE;     // activo mostrar los cambios
            copyArray(cells[CURRENT_CELLS], cells[CHANGED_CELLS]);  //CURRENTizo esa matriz
          }else
          {
            showChanges = FALSE;
          }
          break;
        case CMD_EXIT:
          finish = TRUE;
          break;
        case CMD_START: case CMD_RESTART:
          if( cmd_id == CMD_RESTART || (cmd_id == CMD_START && !stage) )
          {
            createNewCells(cells[CURRENT_CELLS]);  // creo al azar un nuevo estado inicial
            cellsInit(cells[FUTURE_CELLS]);   // inicio la proxima generacion
            copyArray(cells[CURRENT_CELLS], cells[CHANGED_CELLS]);
            copyArray(cells[CURRENT_CELLS], cellBoard);  // mando a pantalla la CURRENT
            stage = 1;  // inicializo numero de generacion
          }
          break;
        default:
          if( stage )
          {
            if( !newGeneration || !showChanges )    // si hay que generar proxima generacion
            {
              cellsStateUpdate(cells[CURRENT_CELLS], cells[FUTURE_CELLS], cells[CHANGED_CELLS]); // cambios de generacion
              copyArray(cells[FUTURE_CELLS], cells[CURRENT_CELLS]);  // CURRENTizo el estado CURRENT
              if( showChanges ) // si estoy en modo mostrar los cambios
              {
                copyArray(cells[CHANGED_CELLS], cellBoard);
              }else
              {
                copyArray(cells[CURRENT_CELLS], cellBoard);
              }
              stage++;
            }else
            {
              fixChanges(cells[CHANGED_CELLS]);   // arreglo para no pisar entre generaciones los cambios
              copyArray(cells[CURRENT_CELLS], cellBoard);
            }
            newGeneration = ~newGeneration;
          }
          break;
      }
    }while( autoMode );
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

  while( getchar() != '\n' ); //limpio el buffer
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
      c = *str++ - '0'; // convierto a numero el ascii
      number *= 10;   // empujo el numero a la izquierda
      number += c;    // y agrego el nuevo entero
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
  char commands[][MAX_LENGTH] = {"", "start", "restart", "exit", "changes", "auto"};
  char argExpected[] = {0, 0, 0, 0, 1, 1};
  int numberOfWords;

  scanf("%[^\n]", input);   // Espero texto hasta que aprete enter

  numberOfWords = splitStr(cmd_input, input, ' ', MAX_CMD); // Lo separo en partes por espacios
  if( numberOfWords != ERROR )
  {
    if( onlyLetters(&cmd_input[0][0]) )     // me fijo que el comando tenga solo letras
    {
      *cmd_id = commandFinder(&cmd_input[0][0], commands, 6); // identifico el comando
      if( *cmd_id != CMD_NONE )
      {
        if( numberOfWords > 1 && argExpected[*cmd_id] ) // si espera argumento, y los recibe
        {
          if( onlyNumbers(&cmd_input[1][0]) )   // me fijo que sea numerico
          {
            *arg = toNumber(&cmd_input[1][0]);    // y lo guardo
          }else
          {
            *cmd_id = CMD_NONE;
          }
        }else if( numberOfWords > 1 && !argExpected[*cmd_id] )  // si recibe y no esperaba argumentos
        {
          *cmd_id = CMD_NONE;
        }else if( numberOfWords == 1 && argExpected[*cmd_id]  ) // o si no recibe, pero esperaba
        {
          *cmd_id = CMD_NONE;   // se toma como que no hubo comando, para que siga a otra generacion
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
    while( str[j] && cmdList[i][j] && str[j] == cmdList[i][j] )   // comparo caracter a caracter
    {                                                             // el string recibido con los strings
      j++;                                                          // de la lista de comandos
    }
    if( !str[j] && !cmdList[i][j] )
    {
      found = TRUE;
    }
  }

  if( i > cmd_length )
  {
    return CMD_NONE;
  }else
  {
    return i-1;                           // devuelvo el indice del comando en la lista, para identificarlo
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
    check = isNumber( *str++ );   // me fijo que sea numero
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
    check = isLetter( *str++ ); // me fijo que sea letra
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
  while( *str && i < max && j < MAX_LENGTH )    // Mientras estoy en el string
  {                                         // y la palabra la cantidad de palabras no pase el limite
    if( *str == separator )                 // si encuentro un separador de palabras
    {
      words[i++][j] = '\0';               // cierro palabra y tomo la proxima
      j = 0;
    }else
    {
      words[i][j++] = *str;               // llega caracter, lo guardo en palabra
    }
    str++;
  }

  if( i >= max || j >= MAX_LENGTH )        // si me pase de largo en palabra o cantidad de palabras
  {                                         // hay error
    return ERROR;
  }else
  {
    words[i][j] = '\0';                     // si no hubo error, cierro la ultima palabra
    return i+1;                               // y devuelvo la cantidad de palabras separadas
  }
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
        cells[i][j] = CELL_DEAD;  // CURRENTizo a muerta o viva los cambios
      }                           // Para que entre varias generaciones no se pisen
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
      cells[i][j] = CELL_DEAD;    // Inicio todos los elementos de la matriz
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
      to[i][j] = from[i][j];    // Voy elemento a elemento copiando
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
      if( rand() % 2 ){             // Decide de forma aleatorioa
        cells[i][j] = CELL_ALIVE;   // Si la celula inicialmente vive o muere
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

  printf(ANSI_COLOR_RESET "\t\t\t\t[EL JUEGO DE LA VIDA]\n");
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

  printf("\n\n| Guia de comandos |\n");
  printf("\texit: Salir del juego\n");
  printf("\trestart: Reiniciar el juego\n");
  printf("\tstart: Iniciar el juego\n");
  printf("\tauto n: Saltar n generaciones\n");
  printf("\tchanges 0/1: Desactivar/Activar mostrar cambios entre generacion\n");
  printf("\n\n[COMANDO] -> ");
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

void cellsStateUpdate(unsigned char CURRENT[ROWS][COLS], unsigned char future[ROWS][COLS], unsigned char changes[ROWS][COLS])
{
	int row, col;			//row y col se usan como indices para recorrer la matriz

	for(row = 0 ; row < ROWS ; row++)					//Mediante este bucle for me muevo por las filas
	{
		for(col = 0 ; col < COLS ; col++)				//Mediante este bucle for me muevo por las columnas
		{
			if( CURRENT[row][col] == CELL_ALIVE )
			{
				if( cellDies(CURRENT, row, col) )
				{
					future[row][col] = CELL_DEAD;    //Si celula muere, cambio proxima generacion
					changes[row][col] = CELL_DYING;  //Y lo muestro en cambios
				}else
        {
					future[row][col] = CELL_ALIVE;
        }
			}else{
				if( cellBorn(CURRENT, row, col) )
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
