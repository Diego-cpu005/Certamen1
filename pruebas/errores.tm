// Pruebas de errores: cada pieza tiene UN error (indicado al lado).
// El intérprete lo informa, descarta esa pieza y sigue con la siguiente.

SUBRUTINA sin_salida(n) { ENTRADA, '1' -> ENTRADA, '1', DER; }   // ERROR: nunca llega a SALIDA
SUBRUTINA mover(n) { ENTRADA, '1' -> SALIDA, '1', DER; }         // correcta (se usa abajo)

MAQUINA E1 {                                // ERROR: estado no declarado
    ALFABETO: '0', '_'; ESTADOS: q0, qA; INICIAL: q0; FINALES: qA;
    TRANSICIONES { q0, '0' -> qX, '0', DER; }
}
MAQUINA E2 {                                // ERROR: inicial y final inexistentes
    ALFABETO: '0', '_'; ESTADOS: q0; INICIAL: qX; FINALES: qZ;
    TRANSICIONES { q0, '0' -> q0, '0', DER; }
}
MAQUINA E3 {                                // ERROR: el alfabeto no incluye el blanco '_'
    ALFABETO: '0', '1'; ESTADOS: q0, qA; INICIAL: q0; FINALES: qA;
    TRANSICIONES { q0, '0' -> qA, '1', DER; }
}
MAQUINA E4 {                                // ERROR: no determinista (dos reglas para q0, '0')
    ALFABETO: '0', '_'; ESTADOS: q0, qA; INICIAL: q0; FINALES: qA;
    TRANSICIONES { q0, '0' -> q0, '0', DER; q0, '0' -> qA, '_', DER; }
}
MAQUINA E5 {                                // ERROR: el estado final qA tiene transición de salida
    ALFABETO: '0', '_'; ESTADOS: q0, qA; INICIAL: q0; FINALES: qA;
    TRANSICIONES { qA, '0' -> q0, '0', DER; }
}
MAQUINA E6 {                                // ERROR: el símbolo '9' no pertenece al alfabeto
    ALFABETO: '0', '_'; ESTADOS: q0, qA; INICIAL: q0; FINALES: qA;
    TRANSICIONES { q0, '9' -> qA, '0', DER; }
}
MAQUINA E7 {                                // ERROR: la subrutina 'inexistente' no está definida
    ALFABETO: '1', '_'; ESTADOS: q0, qA; INICIAL: q0; FINALES: qA;
    TRANSICIONES { LLAMAR inexistente(1) DESDE q0 HACIA qA; }
}
MAQUINA E8 {                                // ERROR: la cinta de entrada tiene un '2' fuera del alfabeto
    ALFABETO: '0', '_'; ESTADOS: q0, qA; INICIAL: q0; FINALES: qA;
    TRANSICIONES { q0, '0' -> qA, '0', DER; }
    SIMULAR: "02";
}

MAQUINA Valida {                            // correcta: se ejecuta aunque haya errores antes
    ALFABETO: '1', '_'; ESTADOS: q0, qA; INICIAL: q0; FINALES: qA;
    TRANSICIONES { LLAMAR mover(2) DESDE q0 HACIA qA; }
    SIMULAR: "11";
}
