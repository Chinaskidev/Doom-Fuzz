// ============================================================
// DOOM FUZZ — Unidad de Distorsion Stoner/Doom
// Faust DSP · 2026
// ============================================================
// Cadena de señal:
//   Entrada → Puerta de ruido → Pre-EQ (refuerzo de graves) →
//   Etapa de ganancia → Recorte duro asimetrico →
//   Mezcla de octava abajo → Pila de tonos (LP/HP) →
//   Simulacion de cabina → Salida
// ============================================================

import("stdfaust.lib");

// --- Controles de interfaz -----------------------------------------

gain    = hslider("[0] Gain", 0.7, 0.0, 1.0, 0.01) : si.smoo;
volume  = hslider("[1] Volume", 0.5, 0.0, 1.0, 0.01) : si.smoo;
tone    = hslider("[2] Tone", 0.35, 0.0, 1.0, 0.01) : si.smoo;
bass    = hslider("[3] Bass Boost", 0.6, 0.0, 1.0, 0.01) : si.smoo;
octave  = hslider("[4] Octave Down", 0.3, 0.0, 1.0, 0.01) : si.smoo;
gate_th = hslider("[5] Gate Threshold", -60, -80, -20, 1) : si.smoo;
sag     = hslider("[6] Sag (compression)", 0.4, 0.0, 1.0, 0.01) : si.smoo;

// --- Puerta de ruido -----------------------------------------------

noise_gate = _ <: (_, amp_detect) : *
with {
    amp_detect = an.amp_follower(0.01) : >(ba.db2linear(gate_th)) : si.smoo;
};

// --- Pre-EQ: Refuerzo de graves ------------------------------------
// Refuerzo de bajos en paralelo: extraer graves, amplificar y mezclar

bass_boost = _ <: (_, low_boost) : +
with {
    low_boost = fi.lowpass(2, 250) : *(bass * 3.0);
};

// --- Etapa de ganancia ---------------------------------------------

soft_clip(x) = ma.tanh(x);

// Recorte duro asimetrico — el positivo recorta mas que el negativo
asym_clip(x) = min(0.8, max(-1.0, x));

gain_stage = *(1.0 + gain * 40.0)       // ganancia de entrada: hasta 41x
           : soft_clip                    // 1ra etapa: saturacion calida
           : *(3.0) : soft_clip           // 2da etapa: empujar mas fuerte
           : *(2.5) : asym_clip;          // 3ra etapa: recorte duro asimetrico

// --- Simulacion de caida de voltaje --------------------------------

voltage_sag = co.compressor_mono(
    2.0 + sag * 6.0,    // ratio: 2:1 a 8:1
    -10,                 // umbral (dB)
    0.001,               // ataque (s)
    0.08 + sag * 0.3     // liberacion (s) — lento = mas caida
);

// --- Octava abajo --------------------------------------------------
// Rectificacion de onda completa para generar sub-octava

octave_down = _ <: (_, sub_gen) : +
with {
    sub_gen = abs : fi.lowpass(2, 120) : *(2.0) : soft_clip : *(octave);
};

// --- Pila de tonos -------------------------------------------------
// tone = 0 → fango doom | tone = 1 → fuzz brillante y cortante

tone_stack = _ <: (lp, hp) : mix
with {
    lp_freq = 400 + tone * 3000;
    hp_freq = 80 + (1.0 - tone) * 200;
    lp = fi.lowpass(2, lp_freq);
    hp = fi.highpass(1, hp_freq);
    mix(l, h) = l * (1.0 - tone * 0.6) + h * (tone * 0.6);
};

// --- Simulacion de cabina ------------------------------------------

cabinet_sim = fi.lowpass(3, 4500)              // caida del parlante
            : fi.peak_eq(3.0, 1500, 300)       // presencia en medios
            : fi.highpass(1, 60);               // apretar sub-graves

// --- Cadena de señal completa --------------------------------------

fuzz_chain = noise_gate
           : bass_boost
           : gain_stage
           : voltage_sag
           : octave_down
           : tone_stack
           : cabinet_sim
           : *(volume);

// --- Salida estereo ------------------------------------------------

stereo_spread = _ <: (_ , @(int(ma.SR * 0.003)));

process = _ : fuzz_chain : stereo_spread;
