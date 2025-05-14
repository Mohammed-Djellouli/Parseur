import os

# Dossiers d'entrée et de sortie
input_folder = "pdf"
output_folder = "output_pdf2text"

# Créer le dossier de sortie s’il n’existe pas
os.makedirs(output_folder, exist_ok=True)

def convertir_pdf_en_txt(pdf_path, txt_path):
    commande = f"pdf2txt.py \"{pdf_path}\" > \"{txt_path}\""
    print(f"Conversion de {pdf_path} en {txt_path}...")
    os.system(commande)
    print("Conversion terminée.")

# Parcourir tous les fichiers dans le dossier "pdf"
for filename in os.listdir(input_folder):
    if filename.endswith(".pdf"):
        pdf_path = os.path.join(input_folder, filename)
        txt_filename = filename.replace(".pdf", ".txt")
        txt_path = os.path.join(output_folder, txt_filename)
        convertir_pdf_en_txt(pdf_path, txt_path)
