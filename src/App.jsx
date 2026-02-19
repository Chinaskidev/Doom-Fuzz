import { useState, useEffect } from "react";
import Knob from "./components/Knob";
import LevelMeter from "./components/LevelMeter";
import useAudioEngine from "./hooks/useAudioEngine";

const DEFAULT_PARAMS = {
  gain: 0.7,
  volume: 0.5,
  tone: 0.35,
  bass: 0.6,
  octave: 0.3,
  gateTh: -60,
  sag: 0.4,
};

export default function App() {
  const [params, setParams] = useState(DEFAULT_PARAMS);
  const [active, setActive] = useState(false);

  const {
    started,
    devices,
    selectedInput,
    setSelectedInput,
    level,
    start,
    stop,
    sendParams,
  } = useAudioEngine();

  // Sincronizar parámetros + bypass con el hilo DSP
  useEffect(() => {
    sendParams({ ...params, bypass: active ? 0 : 1 });
  }, [params, active, sendParams]);

  const setParam = (key) => (val) => {
    setParams((p) => ({
      ...p,
      [key]: typeof val === "number" ? Math.round(val * 100) / 100 : val,
    }));
  };

  const handleStart = async () => {
    const ok = await start();
    if (ok) setActive(true);
  };

  const handleStop = () => {
    stop();
    setActive(false);
  };

  const ledColor = active ? "#00ff44" : "#331a00";
  const ledShadow = active
    ? "0 0 12px #00ff44, 0 0 24px #00ff4488"
    : "none";

  return (
    <div
      style={{
        minHeight: "100vh",
        display: "flex",
        alignItems: "center",
        justifyContent: "center",
        background: "#0a0a0a",
        backgroundImage:
          "radial-gradient(circle at 50% 30%, #1a1510 0%, #0a0a0a 70%)",
        fontFamily: "'Courier New', monospace",
        padding: 20,
      }}
    >
      {/* ── Carcasa del pedal ── */}
      <div
        style={{
          width: 340,
          background: "#1a1a1a",
          borderRadius: 16,
          padding: "28px 24px",
          border: "2px solid #2a2a2a",
          boxShadow:
            "0 8px 32px rgba(0,0,0,0.8), inset 0 1px 0 rgba(255,255,255,0.05)",
          backgroundImage:
            "linear-gradient(180deg, #1e1e1e 0%, #151515 50%, #111 100%)",
          position: "relative",
          overflow: "hidden",
        }}
      >
        {/* Textura metálica superpuesta */}
        <div
          style={{
            position: "absolute",
            inset: 0,
            opacity: 0.03,
            pointerEvents: "none",
            backgroundImage: `url("data:image/svg+xml,%3Csvg width='4' height='4' xmlns='http://www.w3.org/2000/svg'%3E%3Crect width='1' height='1' fill='white'/%3E%3C/svg%3E")`,
            backgroundSize: "4px 4px",
          }}
        />

        {/* ── Encabezado ── */}
        <div style={{ textAlign: "center", marginBottom: 24, position: "relative" }}>
          <div style={{ fontSize: 11, color: "#666", letterSpacing: 4, marginBottom: 4 }}>
            YULTIC DSP
          </div>
          <h1
            style={{
              fontSize: 28,
              fontWeight: 900,
              color: "#e8a020",
              margin: 0,
              textShadow: "0 0 20px rgba(232,160,32,0.3), 0 2px 0 #000",
              letterSpacing: 3,
              fontFamily: "'Georgia', serif",
            }}
          >
            DOOM FUZZ
          </h1>
          <div style={{ fontSize: 9, color: "#555", letterSpacing: 3, marginTop: 4 }}>
            STONER / DOOM DISTORTION
          </div>
        </div>

        {/* ── Selector de dispositivo de entrada ── */}
        {!started && (
          <div style={{ marginBottom: 20 }}>
            <select
              value={selectedInput}
              onChange={(e) => setSelectedInput(e.target.value)}
              style={{
                width: "100%",
                padding: "8px 10px",
                background: "#111",
                color: "#aaa",
                border: "1px solid #333",
                borderRadius: 6,
                fontSize: 11,
                fontFamily: "monospace",
                outline: "none",
                cursor: "pointer",
              }}
            >
              <option value="">Entrada de audio por defecto</option>
              {devices.map((d) => (
                <option key={d.deviceId} value={d.deviceId}>
                  {d.label || `Dispositivo ${d.deviceId.slice(0, 8)}`}
                </option>
              ))}
            </select>
          </div>
        )}

        {/* ── Indicador LED ── */}
        <div style={{ display: "flex", justifyContent: "center", marginBottom: 16 }}>
          <div
            style={{
              width: 10,
              height: 10,
              borderRadius: "50%",
              background: ledColor,
              boxShadow: ledShadow,
              border: "1px solid #333",
              transition: "all 0.15s",
            }}
          />
        </div>

        {/* ── Medidor de nivel ── */}
        {started && <LevelMeter level={level} />}

        {/* ── Perillas principales: Gain / Tone / Volume ── */}
        <div
          style={{
            display: "flex",
            justifyContent: "space-around",
            marginBottom: 20,
          }}
        >
          <Knob
            label="Gain"
            value={params.gain}
            onChange={setParam("gain")}
            color="#ff4444"
          />
          <Knob
            label="Tone"
            value={params.tone}
            onChange={setParam("tone")}
          />
          <Knob
            label="Volume"
            value={params.volume}
            onChange={setParam("volume")}
          />
        </div>

        {/* ── Perillas secundarias: Bass / Octave / Sag ── */}
        <div
          style={{
            display: "flex",
            justifyContent: "space-around",
            marginBottom: 20,
          }}
        >
          <Knob
            label="Bass"
            value={params.bass}
            onChange={setParam("bass")}
            color="#44aaff"
          />
          <Knob
            label="Octave"
            value={params.octave}
            onChange={setParam("octave")}
            color="#aa44ff"
          />
          <Knob
            label="Sag"
            value={params.sag}
            onChange={setParam("sag")}
            color="#44ff88"
          />
        </div>

        {/* ── Perilla de puerta de ruido ── */}
        <div
          style={{
            display: "flex",
            justifyContent: "center",
            marginBottom: 24,
          }}
        >
          <Knob
            label="Gate"
            value={params.gateTh}
            onChange={setParam("gateTh")}
            min={-80}
            max={-20}
            color="#888"
            size={46}
          />
        </div>

        {/* ── Zona del footswitch ── */}
        <div style={{ display: "flex", justifyContent: "center" }}>
          {!started ? (
            <button
              onClick={handleStart}
              style={{
                width: 140,
                height: 48,
                borderRadius: 8,
                border: "2px solid #333",
                background: "linear-gradient(180deg, #2a2a2a, #1a1a1a)",
                color: "#e8a020",
                fontSize: 13,
                fontWeight: 700,
                fontFamily: "monospace",
                letterSpacing: 2,
                cursor: "pointer",
                boxShadow:
                  "0 4px 12px rgba(0,0,0,0.5), inset 0 1px 0 rgba(255,255,255,0.08)",
                transition: "transform 0.1s",
              }}
              onMouseDown={(e) => (e.target.style.transform = "scale(0.97)")}
              onMouseUp={(e) => (e.target.style.transform = "scale(1)")}
            >
              ⚡ CONECTAR
            </button>
          ) : (
            <div style={{ display: "flex", gap: 12, alignItems: "center" }}>
              {/* Interruptor principal de pisada */}
              <button
                onClick={() => setActive(!active)}
                style={{
                  width: 80,
                  height: 80,
                  borderRadius: "50%",
                  cursor: "pointer",
                  border: `3px solid ${active ? "#e8a020" : "#333"}`,
                  background: active
                    ? "radial-gradient(circle at 40% 40%, #444, #222)"
                    : "radial-gradient(circle at 40% 40%, #333, #1a1a1a)",
                  boxShadow: active
                    ? "0 4px 16px rgba(232,160,32,0.3), inset 0 2px 4px rgba(0,0,0,0.5)"
                    : "0 4px 12px rgba(0,0,0,0.5), inset 0 2px 4px rgba(0,0,0,0.5)",
                  display: "flex",
                  alignItems: "center",
                  justifyContent: "center",
                  transition: "all 0.15s",
                }}
              >
                <span
                  style={{
                    fontSize: 10,
                    color: active ? "#e8a020" : "#555",
                    fontWeight: 700,
                    letterSpacing: 1,
                    fontFamily: "monospace",
                  }}
                >
                  {active ? "ON" : "OFF"}
                </span>
              </button>

              {/* Botón de detener */}
              <button
                onClick={handleStop}
                title="Desconectar audio"
                style={{
                  width: 40,
                  height: 40,
                  borderRadius: "50%",
                  cursor: "pointer",
                  border: "1px solid #333",
                  background: "#1a1a1a",
                  color: "#666",
                  fontSize: 14,
                  display: "flex",
                  alignItems: "center",
                  justifyContent: "center",
                  transition: "color 0.15s",
                }}
                onMouseEnter={(e) => (e.target.style.color = "#ff4444")}
                onMouseLeave={(e) => (e.target.style.color = "#666")}
              >
                ■
              </button>
            </div>
          )}
        </div>

        {/* ── Pie de página ── */}
        <div
          style={{
            textAlign: "center",
            marginTop: 20,
            fontSize: 8,
            color: "#333",
            letterSpacing: 3,
          }}
        >
          HANDCRAFTED IN EL SALVADOR
        </div>
      </div>
    </div>
  );
}
