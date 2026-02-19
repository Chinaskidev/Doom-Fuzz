import { useRef, useCallback } from "react";

export default function Knob({
  label,
  value,
  onChange,
  min = 0,
  max = 1,
  color = "#e8a020",
  size = 56,
}) {
  const dragRef = useRef({ active: false, startY: 0, startVal: 0 });
  const angle = -135 + ((value - min) / (max - min)) * 270;

  const onPointerMove = useCallback(
    (e) => {
      if (!dragRef.current.active) return;
      const dy = dragRef.current.startY - e.clientY;
      const range = max - min;
      const newVal = Math.max(
        min,
        Math.min(max, dragRef.current.startVal + (dy / 200) * range)
      );
      onChange(newVal);
    },
    [min, max, onChange]
  );

  const onPointerUp = useCallback(() => {
    dragRef.current.active = false;
    window.removeEventListener("pointermove", onPointerMove);
    window.removeEventListener("pointerup", onPointerUp);
  }, [onPointerMove]);

  const onPointerDown = (e) => {
    e.preventDefault();
    dragRef.current = { active: true, startY: e.clientY, startVal: value };
    window.addEventListener("pointermove", onPointerMove);
    window.addEventListener("pointerup", onPointerUp);
  };

  return (
    <div
      style={{
        display: "flex",
        flexDirection: "column",
        alignItems: "center",
        gap: "6px",
        userSelect: "none",
      }}
    >
      <div
        onPointerDown={onPointerDown}
        style={{
          width: size,
          height: size,
          borderRadius: "50%",
          cursor: "grab",
          background:
            "radial-gradient(circle at 35% 35%, #555 0%, #222 60%, #111 100%)",
          boxShadow:
            "0 2px 8px rgba(0,0,0,0.7), inset 0 1px 2px rgba(255,255,255,0.1)",
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
          transform: `rotate(${angle}deg)`,
          transition: "transform 0.05s ease-out",
          border: "2px solid #333",
          position: "relative",
        }}
      >
        {/* Indicador de posición */}
        <div
          style={{
            width: 3,
            height: size * 0.32,
            background: color,
            borderRadius: 2,
            position: "absolute",
            top: 5,
            boxShadow: `0 0 6px ${color}88`,
          }}
        />
      </div>
      <span
        style={{
          fontFamily: "'Courier New', monospace",
          fontSize: 10,
          fontWeight: 700,
          color: "#bbb",
          letterSpacing: "1.5px",
          textTransform: "uppercase",
          textShadow: "0 0 4px rgba(232,160,32,0.2)",
        }}
      >
        {label}
      </span>
    </div>
  );
}
