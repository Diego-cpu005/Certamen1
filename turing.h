#ifndef TURING_H
#define TURING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define BLANCO '_'
#define MAX_REPETIR 10000
#define MAX_CINTA 10000000L

/* Tabla de símbolos: un registro por estado de la máquina */
typedef struct { char* nombre; int es_final; int generado; } Simbolo;

/* Transición (representación interna): usa índices de la tabla de símbolos */
typedef struct { int origen; char leido; int destino; char escrito; int dir; } Transicion;

/* ---- Estado de la máquina actual (definido en semantico.c) ---- */
extern const char* NOM_DIR[];
extern int yylineno;
extern char* nombre_maq;
extern char* alfabeto;
extern int num_alf;
extern Simbolo* tabla;
extern int num_tabla, idx_inicial;
extern Transicion* trans;
extern int num_trans;
extern int hay_sim, cab_sim;
extern char* cadena_sim;
extern int errores_maq, total_errores;

/* ---- semantico.c: tabla de símbolos, validaciones y ciclo de vida ---- */
void error_semantico(const char* fmt, ...);
void error_lexico(const char* s);
int buscar_estado(const char* n);
int insertar_estado(const char* n, int generado);
int obtener_estado_generado(const char* nombre);
void declarar_estado(char* n);
int en_alfabeto(char c);
void agregar_alfabeto(char c);
void validar_blanco(void);
void definir_inicial(char* n);
void definir_final(char* n);
int guardar_transicion(int o, char l, int d, char e, int dir);
void agregar_transicion(char* o, char l, char* d, char e, int dir);
void iniciar_maquina(char* nombre);
void definir_simulacion(char* cadena, int cabezal);
void cerrar_maquina(void);
void liberar_todo(void);

/* ---- subrutinas.c: biblioteca de subrutinas y expansión ---- */
void iniciar_sub(char* nombre, int con_param);
void agregar_transicion_sub(char* o, char l, char* d, char e, int dir);
void cerrar_sub(void);
void expandir_llamada(char* nombre, int n, char* desde, char* hacia);
void reiniciar_llamadas(void);
void liberar_subrutinas(void);

/* ---- interprete.c: simulación paso a paso ---- */
void imprimir_tabla_transiciones(void);
void ejecutar_simulacion(void);

#endif
