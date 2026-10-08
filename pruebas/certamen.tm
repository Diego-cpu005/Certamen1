// ==========================================
// 1. PRIMERA MÁQUINA: SUMADOR UNARIO
// ==========================================

SUBRUTINA ir_al_final() {
    ENTRADA, '1' -> ENTRADA, '1', DER;
    ENTRADA, '_' -> SALIDA, '_', IZQ;
}

MAQUINA SumadorUnario {
    ALFABETO: '1', '+', '_';
    ESTADOS: q_inicio, q_cambia, q_borra_ultimo, q_fin;
    INICIAL: q_inicio;
    FINALES: q_fin;
    
    TRANSICIONES {
        q_inicio, '1' -> q_inicio, '1', DER;
        q_inicio, '+' -> q_cambia, '1', DER;
        LLAMAR ir_al_final() DESDE q_cambia HACIA q_borra_ultimo;
        q_borra_ultimo, '1' -> q_fin, '_', QUIETO;
    }
    SIMULAR: "111+11" CABEZAL 0;
}

// ==========================================
// 2. SEGUNDA MÁQUINA: RELLENADOR DE CEROS
// ==========================================

SUBRUTINA escribir_cero(n) {
    ENTRADA, '_' -> SALIDA, '0', DER;
}

MAQUINA PaddingCeros {
    ALFABETO: '0', '1', '_';
    ESTADOS: q_busca_fin, q_escribe_relleno, q_fin;
    INICIAL: q_busca_fin;
    FINALES: q_fin;
    
    TRANSICIONES {
        q_busca_fin, '0' -> q_busca_fin, '0', DER;
        q_busca_fin, '1' -> q_busca_fin, '1', DER;
        q_busca_fin, '_' -> q_escribe_relleno, '_', QUIETO;
        LLAMAR escribir_cero(5) DESDE q_escribe_relleno HACIA q_fin;
    }
    SIMULAR: "1011" CABEZAL 0;
}