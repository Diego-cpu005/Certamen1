%code {
#include "turing.h"
extern int yylex(void);
extern FILE* yyin;
void yyerror(const char* s);
}

%define parse.error verbose

%union {
    char* cadena;
    int entero;
    char caracter;
}

%token MAQUINA SUBRUTINA ALFABETO ESTADOS INICIAL FINALES TRANSICIONES SIMULAR
%token CABEZAL LLAMAR DESDE HACIA
%token FLECHA LLAVE_A LLAVE_C PAR_A PAR_C COMA PUNTO_COMA
%token <entero> DIRECCION NUMERO
%token <cadena> ID CADENA
%token <caracter> CARACTER

%type <entero> param_opt arg_opt

%%

/* ============ ESTRUCTURA GENERAL: una o más subrutinas y/o máquinas ============ */

programa: lista_decl ;

lista_decl: decl
          | lista_decl decl
          ;

decl: subrutina
    | maquina
    ;

/* ============ SUBRUTINAS: moldes de transiciones con estados ENTRADA y SALIDA ============ */

subrutina: SUBRUTINA ID PAR_A param_opt PAR_C { iniciar_sub($2, $4); }
           LLAVE_A lista_transiciones_sub LLAVE_C { cerrar_sub(); }
         ;

param_opt: /* vacío */  { $$ = 0; }
         | ID           { $$ = 1; free($1); }       /* parámetro entero */
         ;

lista_transiciones_sub: transicion_sub
                      | lista_transiciones_sub transicion_sub
                      ;

transicion_sub: ID COMA CARACTER FLECHA ID COMA CARACTER COMA DIRECCION PUNTO_COMA
                    { agregar_transicion_sub($1, $3, $5, $7, $9); }
              ;

/* ============ MÁQUINAS ============ */

maquina: MAQUINA ID { iniciar_maquina($2); } LLAVE_A cuerpo LLAVE_C { cerrar_maquina(); } ;

cuerpo: def_alfabeto def_estados def_inicial def_finales def_transiciones def_simular ;

def_alfabeto: ALFABETO lista_caracteres PUNTO_COMA { validar_blanco(); } ;

lista_caracteres: CARACTER                       { agregar_alfabeto($1); }
                | lista_caracteres COMA CARACTER { agregar_alfabeto($3); }
                ;

def_estados: ESTADOS lista_estados PUNTO_COMA ;

lista_estados: ID                    { declarar_estado($1); }
             | lista_estados COMA ID { declarar_estado($3); }
             ;

def_inicial: INICIAL ID PUNTO_COMA { definir_inicial($2); } ;

def_finales: FINALES lista_finales PUNTO_COMA ;

lista_finales: ID                    { definir_final($1); }
             | lista_finales COMA ID { definir_final($3); }
             ;

def_transiciones: TRANSICIONES LLAVE_A lista_transiciones LLAVE_C ;

lista_transiciones: transicion_o_llamada
                  | lista_transiciones transicion_o_llamada
                  ;

transicion_o_llamada: transicion
                    | llamada
                    ;

transicion: ID COMA CARACTER FLECHA ID COMA CARACTER COMA DIRECCION PUNTO_COMA
                { agregar_transicion($1, $3, $5, $7, $9); }
          ;

llamada: LLAMAR ID PAR_A arg_opt PAR_C DESDE ID HACIA ID PUNTO_COMA
             { expandir_llamada($2, $4, $7, $9); }
       ;

arg_opt: /* vacío */  { $$ = -1; }
       | NUMERO       { $$ = $1; }
       ;

def_simular: /* vacío */
           | SIMULAR CADENA PUNTO_COMA                    { definir_simulacion($2, 0); }
           | SIMULAR CADENA CABEZAL NUMERO PUNTO_COMA     { definir_simulacion($2, $4); }
           ;

%%

void yyerror(const char* s) {
    printf("ERROR SINTÁCTICO (línea %d): %s\n", yylineno, s);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Uso: %s archivo.tm\n", argv[0]);
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if (!yyin) { perror(argv[1]); return 1; }

    printf("--- Intérprete de Máquinas de Turing (DSL) --- archivo: %s\n", argv[1]);
    int r = yyparse();
    fclose(yyin);
    if (r != 0) printf("\n>>> Análisis detenido por error sintáctico.\n");
    printf("\n--- Fin. Errores totales: %d ---\n", total_errores);
    liberar_todo();
    return (r != 0 || total_errores > 0) ? 1 : 0;
}
