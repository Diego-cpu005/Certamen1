%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"

void error_lexico(const char* s);
%}

%option noyywrap
%option yylineno

/* ---------- Macros léxicas (ER) ---------- */
DIGITO      [0-9]
LETRA       [a-zA-Z]
ID          {LETRA}({LETRA}|{DIGITO}|"_")*
NUMERO      {DIGITO}+

%%
"MAQUINA"                   { return MAQUINA; }
"SUBRUTINA"                 { return SUBRUTINA; }
"ALFABETO"[ \t]*":"         { return ALFABETO; }
"ESTADOS"[ \t]*":"          { return ESTADOS; }
"INICIAL"[ \t]*":"          { return INICIAL; }
"FINALES"[ \t]*":"          { return FINALES; }
"SIMULAR"[ \t]*":"          { return SIMULAR; }
"TRANSICIONES"              { return TRANSICIONES; }

"CABEZAL"                   { return CABEZAL; }
"LLAMAR"                    { return LLAMAR; }
"DESDE"                     { return DESDE; }
"HACIA"                     { return HACIA; }

"DER"                       { yylval.entero = 1;  return DIRECCION; }
"IZQ"                       { yylval.entero = -1; return DIRECCION; }
"QUIETO"                    { yylval.entero = 0;  return DIRECCION; }

"->"                        { return FLECHA; }
"{"                         { return LLAVE_A; }
"}"                         { return LLAVE_C; }
"("                         { return PAR_A; }
")"                         { return PAR_C; }
","                         { return COMA; }
";"                         { return PUNTO_COMA; }

{NUMERO}                    { yylval.entero = atoi(yytext); return NUMERO; }
{ID}                        { yylval.cadena = strdup(yytext); return ID; }

\'[^\'\n]\'                 { yylval.caracter = yytext[1]; return CARACTER; }
\"[^\"\n]*\"                {
                                yytext[strlen(yytext)-1] = '\0';
                                yylval.cadena = strdup(yytext+1);
                                return CADENA;
                            }

[ \t\r\n]+                  { /* ignorar espacios y saltos de línea */ }
"//".*                      { /* comentario de una línea */ }
.                           { error_lexico(yytext); }
%%
