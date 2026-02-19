// ============================================================
// DOOM FUZZ — Procesador DSP en AudioWorklet
// Todo el procesamiento de señal corre aquí en un hilo de audio dedicado
// ============================================================

class DoomFuzzProcessor extends AudioWorkletProcessor {
  constructor() {
    super();
    this.params = {
      gain: 0.7, volume: 0.5, tone: 0.35,
      bass: 0.6, octave: 0.3, gateTh: -60, sag: 0.4, bypass: 0,
    };

    // Estados de filtros biquad
    this.lp250   = { x1: 0, x2: 0, y1: 0, y2: 0 };
    this.lp120   = { x1: 0, x2: 0, y1: 0, y2: 0 };
    this.lpTone  = { x1: 0, x2: 0, y1: 0, y2: 0 };
    this.hpTone  = { x1: 0, y1: 0 };
    this.lpCab   = { x1: 0, x2: 0, y1: 0, y2: 0 };
    this.hpCab   = { x1: 0, y1: 0 };
    this.peak    = { x1: 0, x2: 0, y1: 0, y2: 0 };

    // Seguidores de envolvente
    this.gateEnv = 0;
    this.compEnv = 0;

    // Buffer de retardo para amplitud estéreo (efecto Haas)
    this.delayBuf = new Float32Array(512);
    this.delayIdx = 0;

    // Recibir actualizaciones de parámetros desde el hilo principal
    this.port.onmessage = (e) => {
      Object.assign(this.params, e.data);
    };
  }

  // ── Pasabajos de 2do orden (biquad) ───────────────────────
  biquadLP(x, state, freq, Q = 0.707) {
    const w0 = (2 * Math.PI * freq) / sampleRate;
    const sinw = Math.sin(w0);
    const cosw = Math.cos(w0);
    const alpha = sinw / (2 * Q);

    const b0 = (1 - cosw) / 2;
    const b1 = 1 - cosw;
    const b2 = (1 - cosw) / 2;
    const a0 = 1 + alpha;
    const a1 = -2 * cosw;
    const a2 = 1 - alpha;

    const y =
      (b0 / a0) * x +
      (b1 / a0) * state.x1 +
      (b2 / a0) * state.x2 -
      (a1 / a0) * state.y1 -
      (a2 / a0) * state.y2;

    state.x2 = state.x1;
    state.x1 = x;
    state.y2 = state.y1;
    state.y1 = y;
    return y;
  }

  // ── Pasaaltos de 1er orden ────────────────────────────────
  hp1(x, state, freq) {
    const rc = 1 / (2 * Math.PI * freq);
    const dt = 1 / sampleRate;
    const a = rc / (rc + dt);
    const y = a * (state.y1 + x - state.x1);
    state.x1 = x;
    state.y1 = y;
    return y;
  }

  // ── EQ paramétrico (realce de medios) ─────────────────────
  peakEQ(x, state, freq, gainDb, bw) {
    const A = Math.pow(10, gainDb / 40);
    const w0 = (2 * Math.PI * freq) / sampleRate;
    const sinw = Math.sin(w0);
    const alpha = sinw * Math.sinh((Math.log(2) / 2) * bw * (w0 / sinw));

    const b0 = 1 + alpha * A;
    const b1 = -2 * Math.cos(w0);
    const b2 = 1 - alpha * A;
    const a0 = 1 + alpha / A;
    const a1 = -2 * Math.cos(w0);
    const a2 = 1 - alpha / A;

    const y =
      (b0 / a0) * x +
      (b1 / a0) * state.x1 +
      (b2 / a0) * state.x2 -
      (a1 / a0) * state.y1 -
      (a2 / a0) * state.y2;

    state.x2 = state.x1;
    state.x1 = x;
    state.y2 = state.y1;
    state.y1 = y;
    return y;
  }

  // ── Procesar una muestra a través de toda la cadena ───────
  processSample(x) {
    const p = this.params;

    // Bypass: señal limpia
    if (p.bypass > 0.5) return [x, x];

    // ─ Puerta de ruido ──────────────────────────────────────
    const absX = Math.abs(x);
    const gateAtk = 0.01;   // ataque rápido  (~2ms)
    const gateRel = 0.0003; // liberación lenta (~700ms)
    this.gateEnv += (absX - this.gateEnv) * (absX > this.gateEnv ? gateAtk : gateRel);
    const thresh = Math.pow(10, p.gateTh / 20);
    const gateMul =
      this.gateEnv > thresh ? 1.0 : this.gateEnv / Math.max(thresh, 1e-10);
    x *= gateMul;

    // ─ Refuerzo de graves (mezcla LP en paralelo) ───────────
    const lowPart = this.biquadLP(x, this.lp250, 250) * p.bass * 3.0;
    x = x + lowPart;

    // ─ Etapa de ganancia: 3 etapas de recorte en cascada ────
    // Etapa 1: saturación suave
    x *= 1.0 + p.gain * 40.0;
    x = Math.tanh(x);
    // Etapa 2: empujar más fuerte
    x *= 3.0;
    x = Math.tanh(x);
    // Etapa 3: recorte duro asimétrico (carácter de parlante roto)
    x *= 2.5;
    x = Math.min(0.8, Math.max(-1.0, x));

    // ─ Caída de voltaje (compresor seguidor de envolvente) ───
    const level = Math.abs(x);
    const attCoeff = 0.001 * sampleRate;
    const relCoeff = (0.08 + p.sag * 0.3) * sampleRate;

    if (level > this.compEnv) {
      this.compEnv += (level - this.compEnv) / (attCoeff + 1);
    } else {
      this.compEnv += (level - this.compEnv) / (relCoeff + 1);
    }

    const ratio = 2.0 + p.sag * 6.0;
    const compThresh = 0.316; // ≈ -10dB
    if (this.compEnv > compThresh) {
      const dB = 20 * Math.log10(this.compEnv / compThresh);
      const reduction = dB * (1 - 1 / ratio);
      x *= Math.pow(10, -reduction / 20);
    }

    // ─ Octava abajo (rectificación de onda completa) ────────
    const sub =
      Math.tanh(this.biquadLP(Math.abs(x), this.lp120, 120) * 2.0) * p.octave;
    x = x + sub;

    // ─ Pila de tonos ────────────────────────────────────────
    const lpFreq = 400 + p.tone * 3000;
    const hpFreq = 80 + (1 - p.tone) * 200;
    const lpSig = this.biquadLP(x, this.lpTone, lpFreq);
    const hpSig = this.hp1(x, this.hpTone, hpFreq);
    x = lpSig * (1 - p.tone * 0.6) + hpSig * (p.tone * 0.6);

    // ─ Simulación de cabina (2x12 cerrada) ──────────────────
    x = this.biquadLP(x, this.lpCab, 4500);
    x = this.peakEQ(x, this.peak, 1500, 3.0, 1.5);
    x = this.hp1(x, this.hpCab, 60);

    // ─ Volumen de salida ────────────────────────────────────
    x *= p.volume;

    // ─ Amplitud estéreo (retardo Haas de 3ms en canal R) ────
    const delaySamples = Math.min(
      Math.floor(sampleRate * 0.003),
      this.delayBuf.length - 1
    );
    this.delayBuf[this.delayIdx] = x;
    const readIdx =
      (this.delayIdx - delaySamples + this.delayBuf.length) %
      this.delayBuf.length;
    const xR = this.delayBuf[readIdx];
    this.delayIdx = (this.delayIdx + 1) % this.delayBuf.length;

    return [x, xR];
  }

  // ── Callback de procesamiento del AudioWorklet ────────────
  process(inputs, outputs) {
    const input = inputs[0];
    const output = outputs[0];
    if (!input || !input[0]) return true;

    const len = input[0].length;
    for (let i = 0; i < len; i++) {
      const mono = input[0][i] + (input[1] ? input[1][i] : 0);
      const [L, R] = this.processSample(mono);
      if (output[0]) output[0][i] = L;
      if (output[1]) output[1][i] = R;
    }
    return true;
  }
}

registerProcessor("doom-fuzz", DoomFuzzProcessor);
