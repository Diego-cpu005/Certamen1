/* subrutinas.c: biblioteca de subrutinas (moldes) y su expansión por duplicación física de nodos */
#include "turing.h"

/* Una subrutina es un molde de transiciones con dos estados de interfaz: ENTRADA y SALIDA.
   Cualquier otro estado del molde es local y se renombra en cada copia. */
typedef struct { char* origen; char leido; char* destino; char escrito; int dir; } PlantillaTrans;
typedef struct { char* nombre; int con_param; PlantillaTrans* t; int nt, cap; } Subrutina;

static Subrutina* lib = NULL;                 /* biblioteca (persiste entre máquinas) */
static int num_lib = 0, cap_lib = 0;
static Subrutina actual;                      /* subrutina que se está definiendo */
static int errores_previos = 0, usa_entrada = 0, usa_salida = 0;
static int contador_llamadas = 0;

static int buscar_sub(const char* n) {
    for (int i = 0; i < num_lib; i++)
        if (strcmp(lib[i].nombre, n) == 0) return i;
    return -1;
}

/* ---------- Definición ---------- */

void iniciar_sub(char* nombre, int con_param) {
    memset(&actual, 0, sizeof actual);
    actual.nombre = nombre;
    actual.con_param = con_param;
    errores_previos = errores_maq;
    usa_entrada = usa_salida = 0;
    if (buscar_sub(nombre) >= 0) error_semantico("la subrutina '%s' ya fue definida", nombre);
}

void agregar_transicion_sub(char* o, char l, char* d, char e, int dir) {
    if (strcmp(o, "SALIDA") == 0)
        error_semantico("en '%s': SALIDA es el sumidero de la subrutina y no puede tener transiciones de salida", actual.nombre);
    if (strcmp(o, "ENTRADA") == 0) usa_entrada = 1;
    if (strcmp(d, "SALIDA") == 0) usa_salida = 1;
    if (actual.nt >= actual.cap) {
        actual.cap = actual.cap ? actual.cap * 2 : 8;
        actual.t = (PlantillaTrans*) realloc(actual.t, actual.cap * sizeof(PlantillaTrans));
    }
    actual.t[actual.nt++] = (PlantillaTrans){ o, l, d, e, dir };
}

void cerrar_sub(void) {
    if (!usa_entrada || !usa_salida)
        error_semantico("la subrutina '%s' debe tener transiciones desde ENTRADA y hacia SALIDA", actual.nombre);
    if (errores_maq == errores_previos) {
        if (num_lib >= cap_lib) {
            cap_lib = cap_lib ? cap_lib * 2 : 8;
            lib = (Subrutina*) realloc(lib, cap_lib * sizeof(Subrutina));
        }
        lib[num_lib++] = actual;
        printf("SUBRUTINA '%s' definida (%d transición(es) en el molde%s).\n", actual.nombre, actual.nt,
               actual.con_param ? ", parametrizada por un entero" : "");
    } else {
        printf("SUBRUTINA '%s' descartada por errores.\n", actual.nombre);
        for (int i = 0; i < actual.nt; i++) { free(actual.t[i].origen); free(actual.t[i].destino); }
        free(actual.t); free(actual.nombre);
    }
}

/* ---------- Expansión: LLAMAR nombre(n) DESDE a HACIA b ---------- */

/* Estado que corresponde a un nombre del molde dentro de la copia c */
static int estado_copia(const char* nom, const char* sub, int llamada, int c, int entrada, int salida) {
    char buf[200];
    if (strcmp(nom, "ENTRADA") == 0) return entrada;
    if (strcmp(nom, "SALIDA") == 0) return salida;
    snprintf(buf, sizeof buf, "%s_%s%d_%d", nom, sub, llamada, c);   /* estado local, nombre único */
    return obtener_estado_generado(buf);
}

/* Copia el molde n veces. La SALIDA de cada copia es la ENTRADA de la siguiente; la primera
   ENTRADA es el estado DESDE y la última SALIDA es el estado HACIA. */
void expandir_llamada(char* nombre, int n, char* desde, char* hacia) {
    int k = buscar_sub(nombre);
    int origen = buscar_estado(desde), destino = buscar_estado(hacia);
    int ok = 1;
    if (k < 0) { error_semantico("la subrutina '%s' no está definida", nombre); ok = 0; }
    else if (lib[k].con_param && n < 0) { error_semantico("la subrutina '%s' requiere un argumento entero", nombre); ok = 0; }
    else if (!lib[k].con_param && n >= 0) { error_semantico("la subrutina '%s' no recibe argumentos", nombre); ok = 0; }
    else if (lib[k].con_param && (n < 1 || n > MAX_REPETIR)) {
        error_semantico("el argumento de '%s' debe estar entre 1 y %d", nombre, MAX_REPETIR); ok = 0;
    }
    if (origen < 0) { error_semantico("estado '%s' (DESDE) no fue declarado", desde); ok = 0; }
    if (destino < 0) { error_semantico("estado '%s' (HACIA) no fue declarado", hacia); ok = 0; }

    if (ok) {
        Subrutina* s = &lib[k];
        int copias = s->con_param ? n : 1, llamada = ++contador_llamadas;
        int t0 = num_trans, s0 = num_tabla, entrada = origen;
        for (int c = 1; ok && c <= copias; c++) {
            int salida = destino;
            if (c < copias) {
                char buf[200];
                snprintf(buf, sizeof buf, "q_%s%d_%d", s->nombre, llamada, c);
                salida = obtener_estado_generado(buf);
            }
            for (int i = 0; ok && i < s->nt; i++) {
                PlantillaTrans* p = &s->t[i];
                int o = estado_copia(p->origen, s->nombre, llamada, c, entrada, salida);
                int d = estado_copia(p->destino, s->nombre, llamada, c, entrada, salida);
                ok = (o >= 0 && d >= 0 && guardar_transicion(o, p->leido, d, p->escrito, p->dir) == 0);
            }
            entrada = salida;
        }
        if (ok) printf(" -> SUBRUTINA '%s' expandida %d vez/veces entre %s y %s: %d estado(s) nuevo(s), %d transición(es).\n",
                       s->nombre, copias, desde, hacia, num_tabla - s0, num_trans - t0);
    }
    free(nombre); free(desde); free(hacia);
}

/* Cada máquina numera sus llamadas desde 1 */
void reiniciar_llamadas(void) { contador_llamadas = 0; }

void liberar_subrutinas(void) {
    for (int i = 0; i < num_lib; i++) {
        for (int j = 0; j < lib[i].nt; j++) { free(lib[i].t[j].origen); free(lib[i].t[j].destino); }
        free(lib[i].t); free(lib[i].nombre);
    }
    free(lib);
}
