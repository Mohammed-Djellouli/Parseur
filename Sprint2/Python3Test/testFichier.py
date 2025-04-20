import time

start = time.time()

with open("IpsumLorem.txt", "r") as fichier:
    contenu = fichier.read()

end = time.time()
print("Temps ecoule pour lire le fichier: {:.6f} secondes".format(end - start))