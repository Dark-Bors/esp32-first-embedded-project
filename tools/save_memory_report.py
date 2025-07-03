import sys
import shutil
from datetime import datetime
from pathlib import Path

BUILD_DIR = Path("build")
REPORT_DIR = Path("memory_reports")
REPORT_DIR.mkdir(exist_ok=True)

timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
output_file = REPORT_DIR / f"memory_{timestamp}.txt"

# Run the size tool
idf_size_cmd = (
    Path("C:/Espressif/python_env/idf5.4_py3.11_env/Scripts/python.exe"),
    Path("C:/Espressif/frameworks/esp-idf-v5.4.2/tools/idf_size.py"),
    str(BUILD_DIR / "blink_led.map"),
)

import subprocess
result = subprocess.run(idf_size_cmd, capture_output=True, text=True)

if result.returncode == 0:
    with open(output_file, "w", encoding="utf-8") as f:
        f.write(result.stdout)
    print(f"Saved memory report to: {output_file}")
else:
    print("Failed to generate memory report:")
    print(result.stderr)
    sys.exit(1)
