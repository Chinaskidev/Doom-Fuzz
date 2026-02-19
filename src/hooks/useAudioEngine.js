import { useState, useEffect, useRef, useCallback } from "react";

export default function useAudioEngine() {
  const [started, setStarted] = useState(false);
  const [devices, setDevices] = useState([]);
  const [selectedInput, setSelectedInput] = useState("");
  const [level, setLevel] = useState(0);

  const ctxRef = useRef(null);
  const nodeRef = useRef(null);
  const streamRef = useRef(null);
  const analyserRef = useRef(null);
  const rafRef = useRef(null);

  // Enumerar dispositivos de entrada de audio
  useEffect(() => {
    navigator.mediaDevices?.enumerateDevices().then((devs) => {
      setDevices(devs.filter((d) => d.kind === "audioinput"));
    });
  }, []);

  // Bucle de medición de nivel
  useEffect(() => {
    if (!analyserRef.current) return;
    const buf = new Uint8Array(64);
    const tick = () => {
      analyserRef.current.getByteTimeDomainData(buf);
      let peak = 0;
      for (let i = 0; i < buf.length; i++) {
        peak = Math.max(peak, Math.abs(buf[i] - 128));
      }
      setLevel(peak / 128);
      rafRef.current = requestAnimationFrame(tick);
    };
    tick();
    return () => cancelAnimationFrame(rafRef.current);
  }, [started]);

  // Enviar parámetros al worklet
  const sendParams = useCallback((params) => {
    if (nodeRef.current) {
      nodeRef.current.port.postMessage(params);
    }
  }, []);

  // Iniciar motor de audio
  const start = useCallback(async () => {
    try {
      const constraints = {
        audio: selectedInput
          ? { deviceId: { exact: selectedInput } }
          : true,
        video: false,
      };
      const stream = await navigator.mediaDevices.getUserMedia(constraints);
      streamRef.current = stream;

      const ctx = new AudioContext({
        latencyHint: "interactive",
        sampleRate: 48000,
      });
      ctxRef.current = ctx;

      // Reanudar contexto explícitamente — los navegadores pueden iniciarlo en estado
      // "suspended" cuando el AudioContext se crea después de un await (fuera del gesto de usuario)
      if (ctx.state === "suspended") {
        await ctx.resume();
      }

      // Cargar procesador AudioWorklet desde la carpeta public
      await ctx.audioWorklet.addModule("/doom-fuzz-processor.js");

      const source = ctx.createMediaStreamSource(stream);
      const worklet = new AudioWorkletNode(ctx, "doom-fuzz", {
        numberOfInputs: 1,
        numberOfOutputs: 1,
        outputChannelCount: [2],
      });
      nodeRef.current = worklet;

      const analyser = ctx.createAnalyser();
      analyser.fftSize = 128;
      analyserRef.current = analyser;

      // Conectar: micrófono → fuzz → analizador → parlantes
      source.connect(worklet);
      worklet.connect(analyser);
      analyser.connect(ctx.destination);

      setStarted(true);
      return true;
    } catch (err) {
      console.error("Fallo al iniciar audio:", err);
      alert("Error al iniciar audio: " + err.message);
      return false;
    }
  }, [selectedInput]);

  // Detener motor de audio
  const stop = useCallback(() => {
    cancelAnimationFrame(rafRef.current);
    streamRef.current?.getTracks().forEach((t) => t.stop());
    ctxRef.current?.close();
    nodeRef.current = null;
    analyserRef.current = null;
    ctxRef.current = null;
    setStarted(false);
    setLevel(0);
  }, []);

  return {
    started,
    devices,
    selectedInput,
    setSelectedInput,
    level,
    start,
    stop,
    sendParams,
  };
}
