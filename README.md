# PATADOM — Wiki Tecnica del DSP

## Indice

1. [Arquitectura General](#arquitectura-general)
2. [Orden de Construccion](#orden-de-construccion)
3. [Bloques Base](#bloques-base)
   - [Parameters.h](#1-parametersh--los-controles-del-pedal)
   - [Biquad](#2-biquad--filtros-iir)
   - [EnvelopeFollower](#3-envelopefollower--detector-de-nivel)
   - [DelayLine](#4-delayline--buffer-circular)
4. [Modulos DSP](#modulos-dsp)
   - [NoiseGate](#5-noisegate--puerta-de-ruido)
   - [GainStage](#6-gainstage--el-corazon-de-la-distorsion)
   - [VoltageSag](#7-voltagesag--compresion-dinamica)
   - [OctaveDown](#8-octavedown--sub-bajo-sintetico)
   - [ToneStack](#9-tonestack--control-de-tono)
   - [CabinetSim](#10-cabinetsim--simulacion-de-speaker)
   - [StereoWidth](#11-stereowidth--efecto-haas)
5. [DoomFuzzEngine](#12-doomfuzzengine--el-director-de-orquesta)
6. [Cadena de Senal Completa](#cadena-de-senal-completa)
7. [Decisiones de Diseno](#decisiones-de-diseno)

---

## Arquitectura General

El proyecto tiene 3 capas bien separadas:

```
doom-fuzz-plugin/
├── CMakeLists.txt          <- Sistema de build (orquesta todo)
├── dsp/                    <- CAPA 1: DSP puro en C++ (sin JUCE)
│   ├── include/doomfuzz/   <- Headers publicos
│   └── src/                <- Implementaciones .cpp
└── plugin/                 <- CAPA 2 y 3: Wrapper JUCE + UI
```

**Principio clave**: El DSP core (`dsp/`) es C++ puro, sin ninguna dependencia de JUCE. Esto permite portarlo a hardware embebido (Daisy, Teensy) sin cambios.

---

## Orden de Construccion

Los archivos se construyen de abajo hacia arriba por dependencias:

| Orden | Archivo           | Tipo         | Depende de               |
|-------|-------------------|--------------|--------------------------|
| 1     | `Parameters.h`    | Struct       | Nada                     |
| 2     | `Biquad`          | Bloque base  | Nada                     |
| 3     | `EnvelopeFollower`| Bloque base  | Nada                     |
| 4     | `DelayLine`       | Bloque base  | Nada                     |
| 5     | `NoiseGate`       | Modulo DSP   | EnvelopeFollower         |
| 6     | `GainStage`       | Modulo DSP   | Biquad                   |
| 7     | `VoltageSag`      | Modulo DSP   | Nada (envelope propio)   |
| 8     | `OctaveDown`      | Modulo DSP   | Biquad                   |
| 9     | `ToneStack`       | Modulo DSP   | Biquad + OnePoleHP       |
| 10    | `CabinetSim`      | Modulo DSP   | Biquad + OnePoleHP       |
| 11    | `StereoWidth`     | Modulo DSP   | DelayLine                |
| 12    | `DoomFuzzEngine`  | Orquestador  | Todos los anteriores     |

Primero los **ladrillos** (1-4), luego los **modulos** que los usan (5-11), y al final el **engine** que conecta todo (12).

---

## Bloques Base

Estos archivos no dependen de ningun otro modulo del proyecto. Son las piezas fundamentales que los modulos DSP reutilizan.

---

### 1. Parameters.h — Los controles del pedal

**Archivo**: `dsp/include/doomfuzz/Parameters.h`

```cpp
struct Parameters {
    float gain    = 0.7f;      // Cantidad de distorsion [0.0-1.0]
    float volume  = 0.5f;      // Nivel de salida [0.0-1.0]
    float tone    = 0.35f;     // Posicion del tono [0.0-1.0]
    float bass    = 0.6f;      // Cantidad de bass boost [0.0-1.0]
    float octave  = 0.3f;      // Mezcla de octava abajo [0.0-1.0]
    float gateTh  = -60.0f;    // Umbral del noise gate en dB
    float sag     = 0.4f;      // Compresion voltage sag [0.0-1.0]
    bool  bypass  = false;     // Activar/desactivar efecto
};
```

**Que hace**: Es solo un `struct` con valores por defecto. No hace nada por si solo — es el "contrato" que dice: "estos son los 8 controles que tiene el pedal". Todos los demas modulos reciben estos valores.

**Nota**: `#pragma once` al inicio evita que el archivo se incluya multiples veces. `namespace doomfuzz` agrupa todo para evitar conflictos de nombres.

---

### 2. Biquad — Filtros IIR

**Archivos**: `dsp/include/doomfuzz/Biquad.h` + `dsp/src/Biquad.cpp`

Es el "ladrillo" mas reutilizado de todo el proyecto. Contiene dos clases:

#### Clase Biquad (filtro de 2do orden)

```cpp
class Biquad {
    void setLowpass(float freq, float sampleRate, float Q = 0.707f);
    void setHighpass(float freq, float sampleRate, float Q = 0.707f);
    void setPeakEQ(float freq, float sampleRate, float gainDb, float bw);
    float process(float x);

private:
    float b0_, b1_, b2_;        // Coeficientes del numerador (feedforward)
    float a1_, a2_;             // Coeficientes del denominador (feedback)
    float x1_, x2_, y1_, y2_;   // Variables de estado (muestras anteriores)
};
```

Un biquad es un filtro digital recursivo de 2 polos y 2 ceros.

**Formula de procesamiento** (Direct Form II):

```
y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
```

Esta es la ecuacion de diferencias del filtro:
- Suma ponderada de entradas actuales y anteriores (feedforward)
- Resta la salida anterior ponderada (feedback)
- Actualiza los valores historicos para la siguiente muestra

**Tipos de filtro disponibles**:

- **Lowpass** (`setLowpass`): Deja pasar frecuencias bajas, corta agudos. Diseno Butterworth usando `w0 = 2*pi*freq/sampleRate`, `alpha = sin(w0) / (2*Q)`.
- **Highpass** (`setHighpass`): Deja pasar frecuencias altas, corta graves. Espejo del lowpass con numerador invertido.
- **PeakEQ** (`setPeakEQ`): Ecualizacion parametrica. `A = 10^(gainDb/40)` convierte ganancia dB a escala lineal. Permite realzar o atenuar una frecuencia especifica con ancho de banda configurable.

**Lo usan 4 modulos**: GainStage (LP 250Hz), OctaveDown (LP 120Hz), ToneStack (LP variable), CabinetSim (LP 4500Hz + PeakEQ 1500Hz).

#### Clase OnePoleHP (highpass de 1er orden)

```cpp
class OnePoleHP {
    void setFreq(float freq, float sampleRate);
    float process(float x);

private:
    float alpha_, x1_, y1_;
};
```

**Formula**:

```
rc = 1 / (2 * pi * freq)          // Constante RC del filtro analogo
dt = 1 / sampleRate                // Periodo de muestreo
alpha = rc / (rc + dt)             // Coeficiente del filtro digital

y[n] = alpha * (y[n-1] + x[n] - x[n-1])
```

Mas barato computacionalmente que el Biquad (1 polo vs 2 polos). Lo usan ToneStack y CabinetSim.

**Resumen**:

| Clase     | Orden | Uso                                |
|-----------|-------|------------------------------------|
| Biquad    | 2do   | Ecualizacion, filtros resonantes   |
| OnePoleHP | 1er   | Highpass simple, DC offset removal |

---

### 3. EnvelopeFollower — Detector de nivel

**Archivos**: `dsp/include/doomfuzz/EnvelopeFollower.h` + `dsp/src/EnvelopeFollower.cpp`

```cpp
class EnvelopeFollower {
    void setCoeffs(float attackCoeff, float releaseCoeff);
    float process(float x);
    float getEnvelope() const { return env_; }

private:
    float attackCoeff_  = 0.01f;    // ~2 ms de subida
    float releaseCoeff_ = 0.0003f;  // ~700 ms de bajada
    float env_ = 0.0f;
};
```

**Que hace**: Sigue el volumen de la senal en tiempo real. Piensa en el como un "medidor de VU" interno.

**Logica central**:

```cpp
float process(float x) {
    const float absX = std::fabs(x);                              // Valor absoluto
    const float coeff = (absX > env_) ? attackCoeff_ : releaseCoeff_;  // Elige velocidad
    env_ += (absX - env_) * coeff;                                // Se acerca al nivel
    return env_;
}
```

- Si la senal **sube** → usa `attackCoeff_` (rapido, 0.01 = ~2ms). Reacciona rapido a notas nuevas.
- Si la senal **baja** → usa `releaseCoeff_` (lento, 0.0003 = ~700ms). Cae gradualmente.
- `env_ += (absX - env_) * coeff` — interpolacion lineal hacia el nivel actual.

**Lo usa**: NoiseGate (para saber si hay senal o silencio).

---

### 4. DelayLine — Buffer circular

**Archivos**: `dsp/include/doomfuzz/DelayLine.h` + `dsp/src/DelayLine.cpp`

```cpp
class DelayLine {
    void reset();
    void write(float x);
    float read(int delaySamples) const;

private:
    static constexpr int kMaxSize = 512;
    float buffer_[kMaxSize] = {};   // Array fijo en el stack
    int writeIdx_ = 0;
};
```

**Que hace**: Un array fijo de 512 floats que funciona como cinta circular. Escribis en una posicion, avanza, y cuando llega al final vuelve al inicio.

**Logica**:

```cpp
void write(float x) {
    buffer_[writeIdx_] = x;
    writeIdx_ = (writeIdx_ + 1) % kMaxSize;  // Vuelve a 0 al llegar a 512
}

float read(int delaySamples) const {
    if (delaySamples >= kMaxSize) delaySamples = kMaxSize - 1;  // Proteccion
    if (delaySamples < 0) delaySamples = 0;
    const int readIdx = (writeIdx_ - 1 - delaySamples + kMaxSize * 2) % kMaxSize;
    return buffer_[readIdx];   // Lee N muestras atras
}
```

**Capacidad**: 512 muestras a 44100 Hz = ~11.6 ms de delay maximo.

**Dato importante**: `float buffer_[512]` esta en el stack, no en el heap → **cero allocacion dinamica**, ideal para hardware embebido.

**Lo usa**: StereoWidth (para el efecto Haas de 3ms).

---

## Modulos DSP

Estos modulos implementan cada etapa de la cadena de audio. Cada uno usa uno o mas bloques base.

---

### 5. NoiseGate — Puerta de ruido

**Archivos**: `dsp/include/doomfuzz/NoiseGate.h` + `dsp/src/NoiseGate.cpp`

**Posicion en la cadena**: Etapa 1 (primera)

**Depende de**: EnvelopeFollower

```cpp
class NoiseGate {
    float process(float x, float thresholdDb);

private:
    EnvelopeFollower env_;
};
```

**Logica**:

```cpp
void reset() {
    env_.reset();
    env_.setCoeffs(0.01f, 0.0003f);  // Attack ~2ms, Release ~700ms
}

float process(float x, float thresholdDb) {
    const float env = env_.process(x);                          // Nivel actual
    const float thresh = std::pow(10.0f, thresholdDb / 20.0f);  // dB a lineal
    const float gateMul = (env > thresh)
        ? 1.0f                                    // Arriba del umbral: deja pasar
        : env / std::max(thresh, 1e-10f);         // Abajo: atenua gradualmente
    return x * gateMul;
}
```

**Que hace**: Elimina ruido ambiental y hum cuando no estas tocando.

- Convierte el threshold de dB a lineal (ej: -60 dB → 0.001)
- Si el envelope esta **arriba** del umbral → deja pasar todo (x * 1.0)
- Si esta **abajo** → atenua gradualmente (fade suave, no corta de golpe)
- `std::max(thresh, 1e-10f)` evita division por cero

**Parametro**: `gateTh` (-80 dB a -20 dB, default -60 dB)

---

### 6. GainStage — El corazon de la distorsion

**Archivos**: `dsp/include/doomfuzz/GainStage.h` + `dsp/src/GainStage.cpp`

**Posicion en la cadena**: Etapa 2

**Depende de**: Biquad (LP 250 Hz para bass boost)

```cpp
class GainStage {
    void prepare(float sampleRate);
    float process(float x, float gain, float bass);

private:
    Biquad lp250_;   // Lowpass a 250 Hz para aislar graves
};
```

**Este es el modulo mas importante — donde se crea el sonido de distorsion.** Tres etapas en cascada:

```cpp
void prepare(float sampleRate) {
    lp250_.setLowpass(250.0f, sampleRate);    // Configura el filtro de graves
}

float process(float x, float gain, float bass) {
    // ETAPA 0: Bass boost paralelo
    const float lowPart = lp250_.process(x) * bass * 3.0f;
    x = x + lowPart;

    // ETAPA 1: Saturacion suave via tanh
    x *= 1.0f + gain * 40.0f;      // Multiplicador: [1x a 41x]
    x = std::tanh(x);               // Aplasta suavemente entre -1 y +1

    // ETAPA 2: Empuja mas fuerte
    x *= 3.0f;                       // Multiplicador fijo 3x
    x = std::tanh(x);               // Segunda saturacion

    // ETAPA 3: Clip asimetrico (caracter de speaker roto)
    x *= 2.5f;
    x = std::min(0.8f, std::max(-1.0f, x));   // Corta en +0.8 arriba, -1.0 abajo

    return x;
}
```

**Detalle de cada etapa**:

1. **Bass Boost** (Etapa 0): Filtra solo los graves (< 250 Hz) con el Biquad y los suma de vuelta a la senal original. `bass * 3.0` controla la cantidad. Es procesamiento "paralelo" — no reemplaza la senal, la engrosa.

2. **Stage 1** (Saturacion suave): Multiplica la senal por hasta 41x (con gain=1.0) y luego `tanh()` que aplasta la senal suavemente entre -1 y +1. Con gain=0.7 (default): `1 + 0.7 * 40 = 29x`. La funcion `tanh()` redondea los picos en vez de cortarlos, creando distorsion "calida".

3. **Stage 2** (Saturacion agresiva): Empuja 3x mas y otro `tanh()`. Genera mas armonicos porque la senal ya estaba saturada del Stage 1.

4. **Stage 3** (Clip asimetrico): Corta en **+0.8 arriba pero -1.0 abajo**. Esto simula un **parlante roto**: los picos positivos y negativos no son iguales, generando armonicos pares (sonido mas "gordo" y organico). Los armonicos pares son los que hacen que los amps de tubos suenen "musicales".

**Parametros**: `gain` (intensidad de distorsion), `bass` (boost de graves)

---

### 7. VoltageSag — Compresion dinamica

**Archivos**: `dsp/include/doomfuzz/VoltageSag.h` + `dsp/src/VoltageSag.cpp`

**Posicion en la cadena**: Etapa 3

**Depende de**: Nada (tiene su propio envelope interno)

```cpp
class VoltageSag {
    void prepare(float sampleRate);
    float process(float x, float sag);

private:
    float sampleRate_ = 44100.0f;
    float compEnv_ = 0.0f;     // Envelope de compresion
};
```

**Que hace**: Simula la caida de voltaje de un amplificador vintage. Cuando tocas fuerte, la fuente de poder "se cansa" y comprime la senal.

```cpp
float process(float x, float sag) {
    const float level = std::fabs(x);

    // Velocidades de seguimiento (escaladas por sample rate)
    const float attCoeff = 0.001f * sampleRate_;               // Attack muy rapido
    const float relCoeff = (0.08f + sag * 0.3f) * sampleRate_; // Release depende de sag

    // Seguimiento de envelope (detector de pico)
    if (level > compEnv_)
        compEnv_ += (level - compEnv_) / (attCoeff + 1.0f);   // Sube rapido
    else
        compEnv_ += (level - compEnv_) / (relCoeff + 1.0f);   // Baja lento

    // Compresion sobre el umbral
    const float compThresh = 0.316f;   // ~-10 dB en lineal
    if (compEnv_ > compThresh) {
        const float ratio = 2.0f + sag * 6.0f;                // [2:1 a 8:1]
        const float dB = 20.0f * std::log10(compEnv_ / compThresh);  // Cuantos dB sobre el umbral
        const float reduction = dB * (1.0f - 1.0f / ratio);          // Formula de compresion
        x *= std::pow(10.0f, -reduction / 20.0f);                    // Aplica reduccion
    }

    return x;
}
```

**Caracteristicas del compresor**:

| Parametro | Valor |
|-----------|-------|
| Threshold | -10 dB (0.316 lineal) |
| Ratio     | 2:1 (sag=0, suave) a 8:1 (sag=1, aplasta) |
| Attack    | Muy rapido (~23 us) |
| Release   | Depende del parametro sag |

**Efecto**: Notas fuertes se "comprimen", dando la sensacion de que el amplificador **respira**. Es lo que hace que los amps vintage suenen "vivos" — cuando tocas un acorde fuerte, el volumen baja un poco y luego se recupera.

**Parametro**: `sag` (cuanta compresion aplicar)

---

### 8. OctaveDown — Sub-bajo sintetico

**Archivos**: `dsp/include/doomfuzz/OctaveDown.h` + `dsp/src/OctaveDown.cpp`

**Posicion en la cadena**: Etapa 4

**Depende de**: Biquad (LP 120 Hz)

```cpp
class OctaveDown {
    void prepare(float sampleRate);
    float process(float x, float octaveMix);

private:
    Biquad lp120_;   // Lowpass a 120 Hz
};
```

**Que hace**: Genera un sub-bajo una octava por debajo de la nota que tocas. Todo en una linea:

```cpp
void prepare(float sampleRate) {
    lp120_.setLowpass(120.0f, sampleRate);   // Solo deja pasar <120 Hz
}

float process(float x, float octaveMix) {
    const float sub = std::tanh(lp120_.process(std::fabs(x)) * 2.0f) * octaveMix;
    return x + sub;
}
```

**Desglose paso a paso**:

1. `std::fabs(x)` — Rectificacion de onda completa (voltea la parte negativa hacia arriba). Esto "dobla" la frecuencia, pero combinado con el lowpass...
2. `lp120_.process(...)` — El lowpass a 120 Hz extrae solo la fundamental grave, eliminando los armonicos altos de la rectificacion
3. `* 2.0f` → `std::tanh()` — Satura para darle cuerpo y armonicos al sub-bajo
4. `* octaveMix` — Controla cuanto sub-bajo se mezcla (0 = nada, 1 = maximo)
5. `return x + sub` — Se suma a la senal original (procesamiento paralelo)

**Parametro**: `octave` (cuanto sub-bajo agregar)

---

### 9. ToneStack — Control de tono

**Archivos**: `dsp/include/doomfuzz/ToneStack.h` + `dsp/src/ToneStack.cpp`

**Posicion en la cadena**: Etapa 5

**Depende de**: Biquad + OnePoleHP

```cpp
class ToneStack {
    void prepare(float sampleRate, float tone);
    float process(float x, float tone);

private:
    Biquad lpTone_;           // Lowpass variable
    OnePoleHP hpTone_;        // Highpass variable
    float sampleRate_ = 44100.0f;
    float lastTone_ = -1.0f;  // Cache del ultimo valor para optimizacion
};
```

**Que hace**: Crossfade entre sonido oscuro y brillante.

```cpp
float process(float x, float tone) {
    // Solo recalcula coeficientes si tone cambio (optimizacion)
    if (tone != lastTone_) {
        lastTone_ = tone;
        const float lpFreq = 400.0f + tone * 3000.0f;         // [400 - 3400 Hz]
        const float hpFreq = 80.0f + (1.0f - tone) * 200.0f;  // [280 - 80 Hz]
        lpTone_.setLowpass(lpFreq, sampleRate_);
        hpTone_.setFreq(hpFreq, sampleRate_);
    }

    const float lpSig = lpTone_.process(x);     // Senal filtrada por lowpass
    const float hpSig = hpTone_.process(x);     // Senal filtrada por highpass
    return lpSig * (1.0f - tone * 0.6f) + hpSig * (tone * 0.6f);  // Crossfade
}
```

**Comportamiento del knob**:

| Tone | LP Freq | HP Freq | Peso LP | Peso HP | Caracter |
|------|---------|---------|---------|---------|----------|
| 0.0  | 400 Hz  | 280 Hz  | 1.0     | 0.0     | Oscuro, solo graves |
| 0.5  | 1900 Hz | 180 Hz  | 0.7     | 0.3     | Balanceado |
| 1.0  | 3400 Hz | 80 Hz   | 0.4     | 0.6     | Brillante, agudos presentes |

**Optimizacion** (linea `if (tone != lastTone_)`): Solo recalcula los coeficientes de filtro cuando `tone` cambia. Los calculos de `sin()`/`cos()` dentro de `setLowpass()` son costosos, asi que esto ahorra CPU — no recalcula 44100 veces por segundo si el usuario no mueve el knob.

**Parametro**: `tone` (oscuro ↔ brillante)

---

### 10. CabinetSim — Simulacion de speaker

**Archivos**: `dsp/include/doomfuzz/CabinetSim.h` + `dsp/src/CabinetSim.cpp`

**Posicion en la cadena**: Etapa 6

**Depende de**: Biquad + OnePoleHP

```cpp
class CabinetSim {
    void prepare(float sampleRate);
    float process(float x);

private:
    Biquad lpCab_;      // LP 4500 Hz
    Biquad peakEq_;     // Peak EQ 1500 Hz
    OnePoleHP hpCab_;   // HP 60 Hz
};
```

**Que hace**: Simula un gabinete de speaker 1x12". Tres filtros en serie:

```cpp
void prepare(float sampleRate) {
    lpCab_.setLowpass(4500.0f, sampleRate);              // Limite de agudos
    peakEq_.setPeakEQ(1500.0f, sampleRate, 3.0f, 1.5f); // +3dB presencia, BW=1.5 octavas
    hpCab_.setFreq(60.0f, sampleRate);                   // Corte de rumble
}

float process(float x) {
    x = lpCab_.process(x);     // 1. Corta agudos asperos
    x = peakEq_.process(x);    // 2. Boost de presencia
    x = hpCab_.process(x);     // 3. Elimina rumble subsonico
    return x;
}
```

**Detalle de cada filtro**:

| Filtro   | Frecuencia | Funcion |
|----------|-----------|---------|
| Lowpass  | 4500 Hz   | Simula el rolloff natural del speaker (un cono real no reproduce >5kHz) |
| Peak EQ  | 1500 Hz, +3dB | Agrega presencia/medios — la "voz" de la guitarra vive aqui |
| Highpass | 60 Hz     | Remueve DC offset y rumble que el speaker no reproduciria |

**Sin parametros del usuario** — siempre fija. Es lo que hace que suene a "amplificador real" y no a "distorsion digital cruda". Sin este modulo, la distorsion suena metalica y artificial.

---

### 11. StereoWidth — Efecto Haas

**Archivos**: `dsp/include/doomfuzz/StereoWidth.h` + `dsp/src/StereoWidth.cpp`

**Posicion en la cadena**: Etapa 8 (ultima)

**Depende de**: DelayLine

```cpp
class StereoWidth {
    void prepare(float sampleRate);
    void process(float monoIn, float& outL, float& outR);

private:
    DelayLine delay_;
    int delaySamples_ = 0;
};
```

**Que hace**: Crea imagen estereo a partir de una senal mono.

```cpp
void prepare(float sampleRate) {
    // 3ms de delay = 132 muestras a 44100 Hz
    delaySamples_ = std::min(static_cast<int>(sampleRate * 0.003f), 511);
}

void process(float monoIn, float& outL, float& outR) {
    delay_.write(monoIn);
    outL = monoIn;                          // Izquierdo: directo (0 ms)
    outR = delay_.read(delaySamples_);      // Derecho: retrasado (3 ms)
}
```

**Como funciona el efecto Haas**:

- Canal izquierdo = senal directa (0 ms)
- Canal derecho = senal retrasada 3 ms
- Tu cerebro escucha ambos canales y percibe un sonido "ancho"
- 3ms es suficiente para dar espacialidad pero no tanto como para escuchar un eco
- No hay cancelacion de fase en frecuencias bajas (a diferencia de otros metodos de stereo)

---

## 12. DoomFuzzEngine — El director de orquesta

**Archivos**: `dsp/include/doomfuzz/DoomFuzzEngine.h` + `dsp/src/DoomFuzzEngine.cpp`

**Depende de**: Todos los modulos anteriores

```cpp
class DoomFuzzEngine {
public:
    void prepare(float sampleRate);
    void processBlock(const float* inputL, const float* inputR,
                      float* outputL, float* outputR, int numSamples);
    void setParameters(const Parameters& params);

private:
    void processSample(float x, float& outL, float& outR);

    Parameters params_;
    float sampleRate_ = 44100.0f;

    NoiseGate   gate_;
    GainStage   gainStage_;
    VoltageSag  voltageSag_;
    OctaveDown  octaveDown_;
    ToneStack   toneStack_;
    CabinetSim  cabinetSim_;
    StereoWidth stereoWidth_;
};
```

**Este archivo une todo.** Contiene los 7 modulos como miembros y los conecta.

### prepare() — Inicializacion

```cpp
void prepare(float sampleRate) {
    sampleRate_ = sampleRate;
    gate_.reset();
    gainStage_.prepare(sampleRate);
    voltageSag_.prepare(sampleRate);
    octaveDown_.prepare(sampleRate);
    toneStack_.prepare(sampleRate, params_.tone);
    cabinetSim_.prepare(sampleRate);
    stereoWidth_.prepare(sampleRate);
}
```

Se llama una vez cuando el DAW inicia la reproduccion. Cada modulo calcula sus coeficientes segun el sample rate (tipicamente 44100 o 48000 Hz).

### setParameters() — Actualizar controles

```cpp
void setParameters(const Parameters& params) {
    params_ = params;   // Copia simple del struct
}
```

Se llama antes de cada bloque de audio. El PluginProcessor lee los valores de la UI y los pasa aqui.

### processBlock() — Procesamiento por bloque

```cpp
void processBlock(const float* inputL, const float* inputR,
                   float* outputL, float* outputR, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        float mono = inputL[i];
        if (inputR != nullptr)
            mono += inputR[i];       // Suma stereo a mono

        float L, R;
        processSample(mono, L, R);   // Procesa muestra por muestra
        outputL[i] = L;
        outputR[i] = R;
    }
}
```

Recibe un bloque de audio del host (tipicamente 256-1024 muestras) y lo procesa **una muestra a la vez** en el loop `for`.

### processSample() — La cadena completa

```cpp
void processSample(float x, float& outL, float& outR) {
    // Bypass: pasa la senal limpia sin procesar
    if (params_.bypass) {
        outL = x;
        outR = x;
        return;
    }

    x = gate_.process(x, params_.gateTh);                    // 1. Noise Gate
    x = gainStage_.process(x, params_.gain, params_.bass);   // 2. Distorsion
    x = voltageSag_.process(x, params_.sag);                 // 3. Compresion
    x = octaveDown_.process(x, params_.octave);              // 4. Sub-bajo
    x = toneStack_.process(x, params_.tone);                 // 5. Tono
    x = cabinetSim_.process(x);                              // 6. Cabinet
    x *= params_.volume;                                      // 7. Volumen
    stereoWidth_.process(x, outL, outR);                     // 8. Stereo
}
```

La variable `x` se va transformando en cada paso. Entra como una muestra mono limpia y sale como dos muestras stereo con distorsion.

---

## Cadena de Senal Completa

```
Entrada Stereo (del DAW o microfono)
        |
        v
  [Suma a Mono]
        |
        v
  1. [NOISE GATE]          threshold: -60 dB
     Elimina ruido          EnvelopeFollower -> atenuacion suave
        |
        v
  2. [GAIN STAGE]           gain: 0.7, bass: 0.6
     Bass boost LP 250Hz
     + 3 etapas:            Stage 1: x * 29 -> tanh (suave)
                            Stage 2: x * 3  -> tanh (agresivo)
                            Stage 3: x * 2.5 -> clip [+0.8, -1.0]
        |
        v
  3. [VOLTAGE SAG]          sag: 0.4
     Compresor              Threshold -10dB, ratio 4.4:1
     Simula amp vintage     El amp "respira"
        |
        v
  4. [OCTAVE DOWN]          octave: 0.3
     Sub-bajo sintetico     abs() -> LP 120Hz -> tanh -> mix
        |
        v
  5. [TONE STACK]           tone: 0.35
     Control de tono        LP [400-3400Hz] / HP [80-280Hz] crossfade
        |
        v
  6. [CABINET SIM]          (sin parametros)
     Simulacion speaker     LP 4500Hz + Peak 1500Hz +3dB + HP 60Hz
        |
        v
  7. [VOLUME]               volume: 0.5
     Nivel de salida        Simple multiplicacion: x *= volume
        |
        v
  8. [STEREO WIDTH]         (sin parametros)
     Efecto Haas            L = directo, R = delay 3ms
        |
        v
  Salida Stereo (al DAW o speakers)
```

---

## Decisiones de Diseno

### 1. DSP separado de JUCE
La libreria `DoomFuzzDSP` es C++ puro. No incluye ningun header de JUCE. Esto permite compilarla para:
- Plugin VST3/AU (via JUCE wrapper)
- Hardware embebido (Daisy Seed, Teensy)
- Cualquier otra plataforma con compilador C++17

### 2. Procesamiento sample-by-sample
Cada muestra se procesa individualmente, no en bloques vectorizados (SIMD). Esto:
- Simplifica la logica de distorsion (cada etapa depende de la anterior)
- Es fiel al prototipo JavaScript original
- Facilita la portabilidad a microcontroladores

### 3. Arrays de tamano fijo
`DelayLine` usa `float buffer_[512]` en el stack. Sin `new`, sin `malloc`, sin `std::vector`. Esto:
- Elimina allocacion dinamica en el audio thread (donde `new` puede causar glitches)
- Es requisito para hardware embebido en tiempo real
- Garantiza latencia predecible

### 4. Coeficientes cacheados
`ToneStack` solo recalcula filtros si el parametro `tone` cambio. Las funciones trigonometricas (`sin`, `cos`) son costosas computacionalmente, asi que se evita recalcularlas 44100 veces por segundo si el usuario no mueve el knob.

### 5. Mono in -> Stereo out
La entrada se suma a mono para procesar la distorsion (los pedales reales son mono). La salida se expande a stereo con el efecto Haas, creando amplitud espacial sin perder compatibilidad mono.

### 6. Clip asimetrico en GainStage
Los limites de clip son +0.8 y -1.0 (no simetricos). Esto genera armonicos pares que suenan mas "musicales" y organicos, similar a un amplificador de tubos o un speaker roto. Los armonicos pares son la razon por la que los amps de tubos suenan diferentes a los transistorizados.

### 7. Metering atomico
El nivel de salida se comunica del audio thread al UI thread via `std::atomic<float>`. Sin mutex, sin locks — solo una escritura atomica. Esto evita bloqueos en el thread de audio, que es critico en tiempo real y no puede esperar por un lock.
