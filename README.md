# DOOM FUZZ — Yultic DSP

Simulador web de pedal de distorsion/fuzz inspirado en el **Blackhawk Amplification Balrog V3 High Gain Fuzzstortion**. Construido con Web Audio API y React, replica la cadena de señal caracteristica de un fuzz de alta ganancia orientado a generos stoner, doom y sludge metal.

## Descripcion

DOOM FUZZ captura audio en tiempo real desde un dispositivo de entrada (interfaz de audio o microfono) y lo procesa a traves de una cadena DSP que emula las etapas de un pedal de fuzz analogico de alta ganancia. Todo el procesamiento corre en un AudioWorklet dedicado a 48kHz, separado del hilo principal del navegador.

## Cadena de señal

El procesamiento sigue el orden clasico de un fuzz de alta ganancia:

1. **Puerta de ruido** — Noise gate con ataque rapido y liberacion lenta para cortar el ruido entre notas sin afectar el sustain.
2. **Refuerzo de graves** — Filtro pasabajos a 250Hz en paralelo para añadir peso a la señal antes de la distorsion.
3. **Etapa de ganancia** — Tres etapas de recorte en cascada: saturacion suave (tanh), empuje adicional y recorte duro asimetrico que simula el caracter de un parlante roto.
4. **Caida de voltaje (Voltage Sag)** — Compresor seguidor de envolvente que emula el comportamiento de una bateria agotandose, añadiendo compresion dinamica natural.
5. **Octava abajo** — Rectificacion de onda completa filtrada a 120Hz para generar un sub-armonico sintetico.
6. **Pila de tonos** — Mezcla entre filtros pasabajos y pasaaltos con frecuencias de corte ajustables.
7. **Simulacion de cabina** — Emulacion de gabinete 2x12 cerrado con pasabajos a 4500Hz, realce parametrico de medios a 1500Hz y pasaaltos a 60Hz.
8. **Amplitud estereo** — Efecto Haas con retardo de 3ms en el canal derecho para crear sensacion de amplitud.

## Controles

| Control | Rango | Funcion |
|---------|-------|---------|
| Gain | 0 - 1 | Cantidad de distorsion aplicada |
| Tone | 0 - 1 | Balance entre graves y agudos en la salida |
| Volume | 0 - 1 | Nivel de salida final |
| Bass | 0 - 1 | Refuerzo de frecuencias bajas antes de la distorsion |
| Octave | 0 - 1 | Mezcla del sub-armonico de octava inferior |
| Sag | 0 - 1 | Intensidad de la compresion por caida de voltaje |
| Gate | -80 a -20 dB | Umbral de la puerta de ruido |

## Proyecto
Aun en desarrollo... to be continue.