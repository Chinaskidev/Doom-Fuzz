export default function LevelMeter({ level }) {
  const color =
    level > 0.8 ? "#ff3333" : level > 0.5 ? "#e8a020" : "#00ff44";

  return (
    <div
      style={{
        height: 3,
        background: "#222",
        borderRadius: 2,
        marginBottom: 16,
        overflow: "hidden",
      }}
    >
      <div
        style={{
          height: "100%",
          borderRadius: 2,
          transition: "width 0.05s",
          width: `${Math.min(level * 100, 100)}%`,
          background: color,
          boxShadow: `0 0 6px ${color}66`,
        }}
      />
    </div>
  );
}
