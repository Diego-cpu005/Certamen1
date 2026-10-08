/* semantico.c: tabla de símbolos, validaciones semánticas y ciclo de vida de cada máquina */
#include "turing.h"

const char* NOM_DIR[]  = { "IZQ", "QUIETO", "DER" };   /* índice = dir + 1 */

/* Nombres de máquinas ya definidas (para detectar repetidas) */
static char** nombres_maq = NULL;
static int num_nombres_maq = 0, cap_nombres_maq = 0;

/* Máquina actual */
char* nombre_maq = NULL;
char* alfabeto = NULL;
int num_alf = 0;
static int cap_alf = 0;
Simbolo* tabla = NULL;
int num_tabla = 0, idx_inicial = -1;
static int cap_tabla = 0;
Transicion* trans = NULL;
int num_trans = 0;
static int cap_trans = 0;
int hay_sim = 0, cab_sim = 0;
char* cadena_sim = NULL;

int errores_maq = 0;
int total_errores = 0;

/* ======================= ERRORES ======================= */

void error_semantico(const char* fmt, ...) {
    va_list ap;
    printf("ERROR SEMÁNTICO (línea %d): ", yylineno);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    errores_maq++;
    total_errores++;
}

void error_lexico(const char* s) {
    printf("ERROR LÉXICO (línea %d): carácter no reconocido '%s'\n", yylineno, s);
    errores_maq++;
    total_errores++;
}

/* ======================= TABLA DE SÍMBOLOS Y ALFABETO ======================= */

int buscar_estado(const char* n) {
    for (int i = 0; i < num_tabla; i++)
        if (strcmp(tabla[i].nombre, n) == 0) return i;
    return -1;
}

int insertar_estado(const char* n, int generado) {
    if (num_tabla >= cap_tabla) {
        cap_tabla = cap_tabla ? cap_tabla * 2 : 16;
        tabla = (Simbolo*) realloc(tabla, cap_tabla * sizeof(Simbolo));
    }
    tabla[num_tabla].nombre = strdup(n);
    tabla[num_tabla].es_final = 0;
    tabla[num_tabla].generado = generado;
    return num_tabla++;
}

void declarar_estado(char* n) {
    if (buscar_estado(n) >= 0) error_semantico("el estado '%s' fue declarado más de una vez", n);
    else insertar_estado(n, 0);
    free(n);
}

/* Devuelve el estado generado por una subrutina (lo crea si no existe) */
int obtener_estado_generado(const char* nombre) {
    int i = buscar_estado(nombre);
    if (i < 0) return insertar_estado(nombre, 1);
    if (!tabla[i].generado) {
        error_semantico("el estado generado '%s' colisiona con un estado declarado", nombre);
        return -1;
    }
    return i;
}

int en_alfabeto(char c) {
    for (int i = 0; i < num_alf; i++) if (alfabeto[i] == c) return 1;
    return 0;
}

void agregar_alfabeto(char c) {
    if (en_alfabeto(c)) { error_semantico("el símbolo '%c' está repetido en el ALFABETO", c); return; }
    if (num_alf >= cap_alf) {
        cap_alf = cap_alf ? cap_alf * 2 : 16;
        alfabeto = (char*) realloc(alfabeto, cap_alf);
    }
    alfabeto[num_alf++] = c;
}

void validar_blanco(void) {
    if (!en_alfabeto(BLANCO))
        error_semantico("el ALFABETO debe incluir el símbolo blanco designado '%c'", BLANCO);
}

void definir_inicial(char* n) {
    int i = buscar_estado(n);
    if (i < 0) error_semantico("el estado inicial '%s' no fue declarado en ESTADOS", n);
    else idx_inicial = i;
    free(n);
}

void definir_final(char* n) {
    int i = buscar_estado(n);
    if (i < 0) error_semantico("el estado final '%s' no fue declarado en ESTADOS", n);
    else if (tabla[i].es_final) error_semantico("el estado final '%s' está repetido", n);
    else tabla[i].es_final = 1;
    free(n);
}

/* ======================= TRANSICIONES ======================= */

int guardar_transicion(int o, char l, int d, char e, int dir) {
    if (tabla[o].es_final) {
        error_semantico("el estado '%s' es FINAL y no puede tener transiciones de salida", tabla[o].nombre);
        return -1;
    }
    if (!en_alfabeto(l) || !en_alfabeto(e)) {
        error_semantico("en la transición desde '%s': el símbolo '%c' o '%c' no pertenece al ALFABETO",
               tabla[o].nombre, l, e);
        return -1;
    }
    for (int i = 0; i < num_trans; i++) {
        if (trans[i].origen == o && trans[i].leido == l) {
            error_semantico("determinismo violado: ya existe una regla para (%s, '%c')", tabla[o].nombre, l);
            return -1;
        }
    }
    if (num_trans >= cap_trans) {
        cap_trans = cap_trans ? cap_trans * 2 : 32;
        trans = (Transicion*) realloc(trans, cap_trans * sizeof(Transicion));
    }
    trans[num_trans].origen = o;
    trans[num_trans].leido = l;
    trans[num_trans].destino = d;
    trans[num_trans].escrito = e;
    trans[num_trans].dir = dir;
    num_trans++;
    return 0;
}

void agregar_transicion(char* o, char l, char* d, char e, int dir) {
    int io = buscar_estado(o), id = buscar_estado(d);
    if (io < 0) error_semantico("estado '%s' usado en una transición no fue declarado", o);
    if (id < 0) error_semantico("estado '%s' usado en una transición no fue declarado", d);
    if (io >= 0 && id >= 0) guardar_transicion(io, l, id, e, dir);
    free(o); free(d);
}

/* ======================= CICLO DE VIDA DE UNA MÁQUINA ======================= */

static void limpiar_maquina(void) {
    for (int i = 0; i < num_tabla; i++) free(tabla[i].nombre);
    num_tabla = 0; num_alf = 0; num_trans = 0;
    idx_inicial = -1; hay_sim = 0; cab_sim = 0;
    free(cadena_sim); cadena_sim = NULL;
    free(nombre_maq); nombre_maq = NULL;
    errores_maq = 0;
}

void iniciar_maquina(char* nombre) {
    limpiar_maquina();
    reiniciar_llamadas();
    nombre_maq = nombre;
    printf("\n########## MÁQUINA '%s' ##########\n", nombre);
    for (int i = 0; i < num_nombres_maq; i++)
        if (strcmp(nombres_maq[i], nombre) == 0) {
            error_semantico("la máquina '%s' ya fue definida", nombre);
            return;
        }
    if (num_nombres_maq >= cap_nombres_maq) {
        cap_nombres_maq = cap_nombres_maq ? cap_nombres_maq * 2 : 8;
        nombres_maq = (char**) realloc(nombres_maq, cap_nombres_maq * sizeof(char*));
    }
    nombres_maq[num_nombres_maq++] = strdup(nombre);
}

/* Valida la cinta de entrada al leer SIMULAR (el alfabeto ya fue declarado) */
void definir_simulacion(char* cadena, int cabezal) {
    for (const char* p = cadena; *p; p++)
        if (!en_alfabeto(*p)) {
            error_semantico("la cinta de entrada contiene '%c', que no pertenece al ALFABETO", *p);
            break;
        }
    hay_sim = 1; cadena_sim = cadena; cab_sim = cabezal;
}

void cerrar_maquina(void) {
    if (errores_maq > 0) {
        printf("\n>>> MÁQUINA '%s' INVÁLIDA: %d error(es). No se simula.\n", nombre_maq, errores_maq);
        return;
    }
    printf("\n>>> MÁQUINA '%s' válida.\n", nombre_maq);
    imprimir_tabla_transiciones();
    if (hay_sim) ejecutar_simulacion();
    else printf("\n(La máquina no declara SIMULAR: no se ejecuta.)\n");
}

void liberar_todo(void) {
    limpiar_maquina();
    free(tabla); free(trans); free(alfabeto);
    for (int i = 0; i < num_nombres_maq; i++) free(nombres_maq[i]);
    free(nombres_maq);
    liberar_subrutinas();
}
