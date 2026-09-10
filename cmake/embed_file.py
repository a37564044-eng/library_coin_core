import sys
from pathlib import Path

src = Path(sys.argv[1])
out = Path(sys.argv[2])
name = sys.argv[3]

data = src.read_bytes()

out.parent.mkdir(parents=True, exist_ok=True)

with out.open("w") as f:
    f.write("#pragma once\n")
    f.write("#include <cstddef>\n")
    f.write("#include <cstdint>\n\n")
    f.write(f"static const std::uint8_t {name}[] = {{\n")

    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        f.write("    ")
        f.write(", ".join(f"0x{x:02x}" for x in chunk))
        f.write(",\n")

    f.write("};\n")
    f.write(f"static const std::size_t {name}_size = {len(data)};\n")

print(f"Embedded {src} -> {out} ({len(data)} bytes)")
