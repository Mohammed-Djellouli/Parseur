#include <iostream>
#include <filesystem>
#include <string>

using namespace std;
namespace fs = std::filesystem;

int main() {
    string dossierEntree = "converted_txt";
    string dossierSortie = "parsed_txt";

    //Supprimer le dossier de sortie s�il existe d�j�
    if (fs::exists(dossierSortie)) {
        cout << "Suppression de l'ancien dossier '" << dossierSortie << "'..." << endl;
        fs::remove_all(dossierSortie);
    }

    //Cr�er le dossier parsed_txt
    cout << "Cr�ation du dossier '" << dossierSortie << "'..." << endl;
    fs::create_directory(dossierSortie);

    //Parcourir les fichiers dans converted_txt
    cout << "Fichiers trouv�s dans '" << dossierEntree << "':" << endl;
    for (const auto& fichier : fs::directory_iterator(dossierEntree)) {
        if (fichier.path().extension() == ".txt") {
            cout << "- " << fichier.path().filename() << endl;
            //Ici tu pourras appeler d�autres fonctions sur chaque fichier
        }
    }

    return 0;
}
