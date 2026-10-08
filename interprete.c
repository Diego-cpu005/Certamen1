/* interprete.c: intérprete (cinta dinámica, traza paso a paso) */
#include "turing.h"

void imprimir_tabla_transiciones(void) {
    printf("\n--- Representación interna (tabla de transiciones) ---\n");
    printf("alfabeto = {");
    for (int i = 0; i < num_alf; i++) printf(" %c", alfabeto[i]);
    printf(" }\n");
    printf("inicial = %s   finales = {", tabla[idx_inicial].nombre);
    for (int i = 0; i < num_tabla; i++) if (tabla[i].es_final) printf(" %s", tabla[i].nombre);
    printf(" }\n");
    int gen = 0;
    for (int i = 0; i < num_tabla; i++) gen += tabla[i].generado;
    printf("estados = %d (%d generados por subrutinas)   transiciones = %d\n", num_tabla, gen, num_trans);
    for (int i = 0; i < num_trans; i++)
        printf("(%s, %c) -> (%s, %c, %s)\n", tabla[trans[i].origen].nombre, trans[i].leido,
               tabla[trans[i].destino].nombre, trans[i].escrito, NOM_DIR[trans[i].dir + 1]);
}

/* ======================= SIMULACIÓN (INTÉRPRETE) ======================= */

/* Cinta dinámicamente extensible hacia ambos lados. Posición lógica = índice - off,
   con la celda 0 = primer carácter de la entrada. */
typedef struct { char* d; long cap; long off; } Cinta;

static int cinta_asegurar(Cinta* c, long* idx) {
    while (*idx < 0) {
        long nueva = c->cap * 2;
        if (nueva > MAX_CINTA) return -1;
        long desp = nueva - c->cap;
        char* nd = (char*) malloc(nueva);
        memset(nd, BLANCO, desp);
        memcpy(nd + desp, c->d, c->cap);
        free(c->d);
        c->d = nd; c->cap = nueva; c->off += desp; *idx += desp;
    }
    while (*idx >= c->cap) {
        long nueva = c->cap * 2;
        if (nueva > MAX_CINTA) return -1;
        c->d = (char*) realloc(c->d, nueva);
        memset(c->d + c->cap, BLANCO, nueva - c->cap);
        c->cap = nueva;
    }
    return 0;
}

/* marcar=1: celdas separadas por espacio y cabezal entre [ ]; marcar=0: cinta final compacta */
static char* cinta_texto(const Cinta* c, long head, int marcar) {
    long lo = -1, hi = -1;
    for (long i = 0; i < c->cap; i++)
        if (c->d[i] != BLANCO) { if (lo < 0) lo = i; hi = i; }
    if (marcar) {
        if (lo < 0) { lo = hi = head; }
        else { if (head < lo) lo = head; if (head > hi) hi = head; }
    }
    if (lo < 0) return strdup("(vacía)");
    char* s = (char*) malloc((hi - lo + 1) * 2 + 8);
    long k = 0;
    for (long i = lo; i <= hi; i++) {
        if (marcar && i == head) { s[k++] = '['; s[k++] = c->d[i]; s[k++] = ']'; }
        else s[k++] = c->d[i];
        if (marcar && i < hi) s[k++] = ' ';
    }
    s[k] = '\0';
    return s;
}

static int buscar_trans(int estado, char leido) {
    for (int i = 0; i < num_trans; i++)
        if (trans[i].origen == estado && trans[i].leido == leido) return i;
    return -1;
}

void ejecutar_simulacion(void) {
    long len = (long) strlen(cadena_sim);
    for (long i = 0; i < len; i++) {
        if (!en_alfabeto(cadena_sim[i])) {
            error_semantico("la cinta de entrada contiene '%c', que no pertenece al ALFABETO", cadena_sim[i]);
            return;
        }
    }
    if (cab_sim < 0) { error_semantico("la posición inicial del cabezal no puede ser negativa"); return; }

    Cinta c;
    c.cap = 64 + len; c.off = 32;
    c.d = (char*) malloc(c.cap);
    memset(c.d, BLANCO, c.cap);
    memcpy(c.d + c.off, cadena_sim, len);
    long idx = c.off + cab_sim;
    if (cinta_asegurar(&c, &idx)) { printf("Cabezal inicial fuera del límite de cinta.\n"); free(c.d); return; }

    int estado = idx_inicial;
    long paso = 0;
    const char* resultado = NULL;
    char* txt;

    printf("\n=== SIMULACIÓN DE '%s' ===\n", nombre_maq);
    txt = cinta_texto(&c, idx, 1);
    printf("Cinta inicial : %s\nCabezal inicial: celda %ld   Estado inicial: %s\n\n", txt, idx - c.off, tabla[estado].nombre);
    free(txt);
    printf("%-6s %-18s %-4s %-50s %-26s %s\n", "paso", "estado", "lee", "acción", "cinta después", "cabezal");

    if (tabla[estado].es_final) resultado = "ACEPTA (el estado inicial ya es final)";

    while (!resultado) {
        char leido = c.d[idx];
        int t = buscar_trans(estado, leido);
        if (t < 0) {
            txt = cinta_texto(&c, idx, 1);
            printf("%-6ld %-18s %-4c %-50s %-26s celda %ld\n", paso, tabla[estado].nombre, leido,
                   "sin transición aplicable", txt, idx - c.off);
            free(txt);
            resultado = "RECHAZA (no existe transición para el par estado-símbolo)";
            break;
        }
        Transicion* tr = &trans[t];
        int estado_prev = estado;
        c.d[idx] = tr->escrito;
        idx += tr->dir;
        estado = tr->destino;
        if (cinta_asegurar(&c, &idx)) {
            printf("%-6ld %-16s %-4c %s\n", paso, tabla[estado_prev].nombre, leido, "el cabezal superó el límite de la cinta");
            resultado = "DETENIDA (límite de cinta alcanzado)";
            break;
        }
        char accion[96];
        int n = snprintf(accion, sizeof accion, "escribe %c, mueve %s", tr->escrito, NOM_DIR[tr->dir + 1]);
        if (estado != estado_prev) n += snprintf(accion + n, sizeof accion - n, ", pasa a %s", tabla[estado].nombre);
        if (tabla[estado].es_final) snprintf(accion + n, sizeof accion - n, " (acepta)");
        txt = cinta_texto(&c, idx, 1);
        printf("%-6ld %-18s %-4c %-50s %-26s celda %ld\n", paso, tabla[estado_prev].nombre, leido, accion, txt, idx - c.off);
        free(txt);
        paso++;
        if (tabla[estado].es_final) resultado = "ACEPTA (estado final alcanzado)";
    }

    txt = cinta_texto(&c, idx, 0);
    printf("\nRESULTADO     : %s\n", resultado);
    printf("Pasos         : %ld\n", paso);
    printf("Estado final  : %s\n", tabla[estado].nombre);
    printf("Cinta final   : %s\n", txt);
    printf("===================================================\n");
    free(txt);
    free(c.d);
}
