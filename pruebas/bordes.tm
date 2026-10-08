// Casos borde de la detención y de la cinta

MAQUINA Rechaza {                           // sin regla para (q0, '1') -> RECHAZA
    ALFABETO: '0', '1', '_'; ESTADOS: q0, qA; INICIAL: q0; FINALES: qA;
    TRANSICIONES { q0, '0' -> q0, '0', DER; q0, '_' -> qA, '_', QUIETO; }
    SIMULAR: "001";
}

MAQUINA CintaCreceIzquierda {               // el cabezal sale por la izquierda: la cinta se extiende
    ALFABETO: '0', '1', '_'; ESTADOS: q0, qA; INICIAL: q0; FINALES: qA;
    TRANSICIONES { q0, '0' -> q0, '0', IZQ; q0, '_' -> qA, '1', QUIETO; }
    SIMULAR: "0";
}
MAQUINA CabezalInicial {                    // el cabezal parte en la celda 2 (SIMULAR ... CABEZAL n)
    ALFABETO: '0', '1', '_'; ESTADOS: q0, qA; INICIAL: q0; FINALES: qA;
    TRANSICIONES { q0, '1' -> qA, '0', QUIETO; }
    SIMULAR: "001" CABEZAL 2;
}
