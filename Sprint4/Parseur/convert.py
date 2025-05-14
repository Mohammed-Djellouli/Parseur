import subprocess
import os


input_dir = "pdf"
output_dir = "output"


os.makedirs(output_dir, exist_ok=True)


for filename in os.listdir(input_dir):
    if filename.endswith(".pdf"):
        input_path = os.path.join(input_dir, filename)
        output_path = os.path.join(output_dir, filename.replace(".pdf", ".txt"))

        
        command = [
            "pdftotext",
            "-layout",
            "-enc", "UTF-8",
            "-nopgbrk",
            input_path,
            output_path
        ]

        
        print(f"Conversion de {filename}...")
        result = subprocess.run(command, capture_output=True, text=True)


