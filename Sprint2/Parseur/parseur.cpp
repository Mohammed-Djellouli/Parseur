#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

int main() {
    string dossierEntree = "converted_txt";
    string dossierSortie = "parsed_txt";

    // Supprimer le dossier de sortie s’il existe déjà
    if (fs::exists(dossierSortie)) {
        cout << "Suppression de l'ancien dossier '" << dossierSortie << "'..." << endl;
        fs::remove_all(dossierSortie);
    }

    // Créer le dossier parsed_txt
    cout << "Création du dossier '" << dossierSortie << "'..." << endl;
    fs::create_directory(dossierSortie);

    // Parcourir les fichiers dans converted_txt
    cout << "Fichiers trouvés dans '" << dossierEntree << "':" << endl;

    for (const auto& fichier : fs::directory_iterator(dossierEntree)) {
        if (fichier.path().extension() == ".txt") {
            string nomOriginal = fichier.path().filename().string();
            cout << "- " << nomOriginal << endl;

            // Remplacer les espaces par des underscores
            string nomModifie = nomOriginal;
            replace(nomModifie.begin(), nomModifie.end(), ' ', '_');

            //Lire le contenu du fichier
            ifstream fichierEntree(fichier.path());
            if (!fichierEntree) {
                cerr << "Erreur lors de l'ouverture du fichier : " << nomOriginal << endl;
                continue;
            }

            stringstream buffer;
            buffer << fichierEntree.rdbuf();
            string contenu = buffer.str();


        }
    }

    return 0;
}
