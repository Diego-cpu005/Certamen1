# Certamen I – Lenguajes de Programación II: DSL para Máquinas de Turing

**Integrantes:** Anais Muñoz, Amalia Toledo, Diego Valenzuela
**Herramientas:** Flex, Bison, GCC, GNU Make

## Compilación y ejecución
```
make                              # compila (genera ./turing)
./turing pruebas/certamen.tm      # ejecuta subrutinas + 4 máquinas
./turing pruebas/errores.tm       # una máquina por cada error de validación
./turing pruebas/bordes.tm        # casos borde
make clean                        # borra lo generado
```

## Estructura del código
| Archivo | Contenido | Líneas |
|---|---|---|
| `lexico.l` | Tokens (ER): palabras clave, símbolos, identificadores, números, caracteres y cadenas | ~60 |
| `sintactico.y` | **GLC** y acciones semánticas (cada acción llama a una función) | ~130 |
| `semantico.c` | Tabla de símbolos, alfabeto, transiciones y validaciones semánticas, ciclo de vida de cada máquina | ~210 |
| `subrutinas.c` | Biblioteca de subrutinas (moldes) y su expansión: duplicación física de nodos | ~120 |
| `interprete.c` | Intérprete: cinta dinámica, traza paso a paso, resultado y cinta final | ~145 |
| `turing.h` | Estructuras de datos y prototipos compartidos | ~65 |

## Flujo
`archivo.tm → Flex (tokens/ER) → Bison (GLC) → acciones semánticas → tabla de símbolos + tabla de transiciones → intérprete → traza paso a paso + cinta final`

## Sintaxis del DSL
```
SUBRUTINA nombre(n) {                 // molde con un parámetro entero (o nombre() sin parámetro)
    ENTRADA, '1' -> SALIDA, '1', DER; // ENTRADA y SALIDA son los estados de interfaz;
    ENTRADA, '_' -> SALIDA, '_', DER; // cualquier otro estado del molde es local
}
MAQUINA Nombre {
    ALFABETO: '0', '1', '_';          // debe incluir el blanco '_'
    ESTADOS: q0, qA;
    INICIAL: q0;
    FINALES: qA;
    TRANSICIONES {
        q0, '0' -> q0, '1', DER;      // (estado, lee) -> (estado nuevo, escribe, mueve)
        LLAMAR nombre(3) DESDE q0 HACIA qA;   // copia el molde 3 veces entre q0 y qA
    }
    SIMULAR: "010";                   // opcional; SIMULAR: "010" CABEZAL 2; fija la posición inicial
}
```
Movimientos: `IZQ`, `DER`, `QUIETO`.

## Gramática (producciones principales)
```
programa              -> lista_decl
lista_decl            -> decl | lista_decl decl
decl                  -> subrutina | maquina
subrutina             -> SUBRUTINA ID ( param_opt ) { lista_transiciones_sub }
transicion_sub        -> ID , CARACTER -> ID , CARACTER , DIRECCION ;
maquina               -> MAQUINA ID { cuerpo }
cuerpo                -> def_alfabeto def_estados def_inicial def_finales def_transiciones def_simular
def_transiciones      -> TRANSICIONES { lista_transiciones }
lista_transiciones    -> transicion_o_llamada | lista_transiciones transicion_o_llamada
transicion_o_llamada  -> transicion | llamada
transicion            -> ID , CARACTER -> ID , CARACTER , DIRECCION ;
llamada               -> LLAMAR ID ( arg_opt ) DESDE ID HACIA ID ;
def_simular           -> ε | SIMULAR CADENA ; | SIMULAR CADENA CABEZAL NUMERO ;
```
Las ER (en `lexico.l`) solo definen tokens; la estructura (varias subrutinas y máquinas, listas de estados y de transiciones, llamadas) está en la GLC.

## Validaciones semánticas (por máquina; un error invalida solo esa máquina)
determinismo · estados de transiciones declarados · inicial y finales existen · símbolos leídos/escritos y cinta de entrada dentro del alfabeto · alfabeto incluye blanco · sin duplicados (estados, símbolos, máquinas, subrutinas) · **un estado final (sumidero) no tiene transiciones de salida**, tampoco `SALIDA` dentro de una subrutina · la subrutina existe y recibe el argumento correcto · el molde solo usa símbolos del alfabeto de la máquina que lo llama.

## Subrutinas
Una subrutina es un **molde** de transiciones con los estados de interfaz `ENTRADA` y `SALIDA`. Al analizar un `LLAMAR nombre(n) DESDE a HACIA b`, se **copia físicamente** el molde n veces: cada copia crea estados nuevos en la tabla de símbolos (memoria dinámica con `realloc`) y sus transiciones en la tabla de transiciones. La `SALIDA` de cada copia es la `ENTRADA` de la siguiente; la primera `ENTRADA` es `a` y la última `SALIDA` es `b`. Los estados locales se renombran por copia (`<estado>_<subrutina><llamada>_<copia>`). Hay subrutinas con parámetro entero (`desplazar(n)`, `escribir_unos(n)`) y sin parámetro (`mover_hasta_blanco()`); una máquina se compone encadenando varias llamadas.

## Cinta y supuestos
- La cinta es **dinámicamente extensible a ambos lados** (se duplica con `realloc`, rellena de blancos). Límite de seguridad: 10 000 000 celdas → la simulación se detiene con "límite de cinta".
- Celda 0 = primer símbolo de la entrada; el cabezal puede tomar posiciones negativas.
- Resultado: ACEPTA (estado final) / RECHAZA (sin transición aplicable) / DETENIDA (límite de cinta).
- Orden fijo de los bloques de una máquina: alfabeto, estados, inicial, finales, transiciones, simular.
- Supuesto propio: un estado final no tiene transiciones de salida (detención absoluta).

## Uso de IA (autorizado por el profesor)
- LLM utilizado: Gemini (Google).
- Uso: consultas de orientación y comprensión conceptual. No se solicitó ni se recibió código como respuesta.
- Prompts utilizados:

**Prompt 1 (orientación sobre el Problema de la Parada y bucles infinitos):**
> *"Estoy desarrollando un intérprete de Máquinas de Turing como proyecto universitario en C (usando Flex y Bison). Mi simulador ejecuta paso a paso las transiciones sobre una cinta dinámica. El problema es que algunas máquinas pueden entrar en un bucle infinito (por ejemplo, una transición que se queda en QUIETO leyendo el mismo símbolo para siempre). Cuáles son las estrategias teóricas y prácticas para manejar esta situación? ¿La única forma de detener un bucle infinito es que el usuario presione Ctrl+C? Existe alguna forma de detectar automáticamente que la máquina entró en un ciclo, o eso contradice el Problema de la Parada de Turing? Necesito orientación conceptual."*

**Prompt 2 (orientación sobre debugging y validación semántica):**
> *"En mi proyecto de compilador para un DSL de Máquinas de Turing (Flex + Bison + C), necesito implementar validaciones semánticas que detecten errores como: estados no declarados usados en transiciones, violación del determinismo (dos reglas para el mismo par estado-símbolo), estados finales con transiciones de salida, símbolos fuera del alfabeto, y subrutinas inexistentes o con argumentos incorrectos. Cuál es la mejor estrategia para organizar estas validaciones? ¿Debería detener el análisis en el primer error o acumular todos los errores y reportarlos al final? Cómo puedo hacer debugging eficiente cuando el output del compilador no coincide con lo esperado? Necesito orientación sobre el enfoque"*

