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
unsigned char cellBorn(char cells[ROWS][COLS], int row, int col);
unsigned char cellSurvive(char cells[ROWS][COLS], int row, int col);
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
