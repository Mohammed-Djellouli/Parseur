#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

// === Fonction extraction Abstract ===
string extraireAbstract(const string& contenu) {
    istringstream iss(contenu);
    string ligne, abstract = "";
    bool inAbstract = false;

    regex contientAbstract("\\b(Abstract|Résumé|ABSTRACT|RESUME)\\b", regex_constants::icase);
    regex finPossible(R"(\b((\d+\.?)|(I+\.))?\s*[Ii]\s*N\s*T\s*R\s*O\s*D\s*U\s*C\s*T\s*I\s*O\s*N\b)", regex_constants::icase);
    regex finPossible2(R"(\bIntroduction\b)", regex_constants::icase);
    regex separateurColonnes(" {7,}");

    vector<string> lignes;
    while (getline(iss, ligne)) {
        ligne.erase(remove(ligne.begin(), ligne.end(), '\r'), ligne.end());
        lignes.push_back(ligne);
    }

    for (size_t i = 0; i < lignes.size(); ++i) {
        string currentLine = lignes[i];
        smatch match;

        if (regex_search(currentLine, match, separateurColonnes)) {
            string leftPart = match.prefix().str();
            string rightPart = match.suffix().str();
            leftPart.erase(0, leftPart.find_first_not_of(" \t\n"));
            leftPart.erase(leftPart.find_last_not_of(" \t\n") + 1);
            rightPart.erase(0, rightPart.find_first_not_of(" \t\n"));
            rightPart.erase(rightPart.find_last_not_of(" \t\n") + 1);

            if (regex_search(rightPart, contientAbstract) || regex_search(rightPart, finPossible) || regex_search(rightPart, finPossible2)) {
                currentLine = rightPart;
            } else {
                currentLine = !leftPart.empty() ? leftPart : rightPart;
            }
        }

        currentLine.erase(0, currentLine.find_first_not_of(" \t\n"));
        currentLine.erase(currentLine.find_last_not_of(" \t\n") + 1);

        if (!inAbstract && regex_search(currentLine, contientAbstract)) {
            inAbstract = true;
            continue;
        }

        if (inAbstract) {
            bool startsFarRight = true;
            for (size_t j = 0; j < 30 && j < lignes[i].size(); ++j) {
                if (lignes[i][j] != ' ') {
                    startsFarRight = false;
                    break;
                }
            }

            if (regex_search(currentLine, finPossible) || regex_search(currentLine, finPossible2)) break;
            if (startsFarRight) continue;

            if (!currentLine.empty()) {
                abstract += currentLine + " ";
            }
        }
    }

    if (abstract.empty()) return "Abstract introuvable";

    abstract.erase(0, abstract.find_first_not_of(" \t\n"));
    abstract.erase(abstract.find_last_not_of(" \t\n") + 1);
    return abstract;
}

// === Fonction extraction Titre ===
string extraireTitre(const string& contenu) {
    istringstream iss(contenu);
    string ligne;
    vector<string> lignesTitre;

    while (getline(iss, ligne)) {
        ligne.erase(remove(ligne.begin(), ligne.end(), '\r'), ligne.end());
        ligne.erase(0, ligne.find_first_not_of(" \t\n"));
        ligne.erase(ligne.find_last_not_of(" \t\n") + 1);
        if (ligne.empty()) continue;

        string lower = ligne;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower.find("from:") != string::npos || lower.find("submitted") != string::npos ||
            lower.find("published") != string::npos || lower.find("journal") != string::npos ||
            lower.find("copyright") != string::npos || lower.find("www.") != string::npos ||
            lower.find("http") != string::npos || lower.find(".org") != string::npos ||
            lower.find(".com") != string::npos || lower.find("@") != string::npos ||
            lower.find("author") != string::npos || lower.find("email") != string::npos ||
            lower.find("proceedings") != string::npos) continue;

        lignesTitre.push_back(ligne);
        if (lignesTitre.size() >= 2) break;
    }

    string titre = "";
    for (size_t i = 0; i < lignesTitre.size(); ++i) {
        titre += lignesTitre[i];
        if (i != lignesTitre.size() - 1) titre += " ";
    }

    if (titre.empty()) return "Titre introuvable";

    vector<string> separateurs = {" – ", "—", " –", "— ", " - ", "–", ".", " -- ", "*", "Minh", "David", "Vincent"};
    for (const auto& sep : separateurs) {
        size_t pos = titre.find(sep);
        if (pos != string::npos) {
            titre = titre.substr(0, pos);
            break;
        }
    }

    titre.erase(0, titre.find_first_not_of(" \t\n"));
    titre.erase(titre.find_last_not_of(" \t\n") + 1);
    return titre;
}

string extraireBiblio(const string& contenu) {
    istringstream iss(contenu);
    string ligne, biblio = "";
    bool inBiblio = false;

    regex debutBiblio(R"(\b(References|Bibliography|Références|Référence|Works Cited)\b)", regex_constants::icase);
    regex separateurColonnes(" {7,}");
    regex ligneVide(R"(^\s*$)");

    vector<string> lignes;
    while (getline(iss, ligne)) {
        ligne.erase(remove(ligne.begin(), ligne.end(), '\r'), ligne.end());
        lignes.push_back(ligne);
    }

    for (size_t i = 0; i < lignes.size(); ++i) {
        string currentLine = lignes[i];
        smatch match;
        string selectedColumn = currentLine;

        if (regex_search(currentLine, match, separateurColonnes)) {
            string leftPart = match.prefix().str();
            string rightPart = match.suffix().str();

            leftPart.erase(0, leftPart.find_first_not_of(" \t\n"));
            leftPart.erase(leftPart.find_last_not_of(" \t\n") + 1);
            rightPart.erase(0, rightPart.find_first_not_of(" \t\n"));
            rightPart.erase(rightPart.find_last_not_of(" \t\n") + 1);

            // On garde une seule colonne pour éviter d'ajouter des morceaux parasites
            if (inBiblio) {
                // Pendant la biblio, on prend la colonne gauche uniquement
                selectedColumn = leftPart;
            } else {
                // Avant la biblio, on cherche le mot-clé sur les deux
                if (regex_search(rightPart, debutBiblio)) {
                    selectedColumn = rightPart;
                } else if (regex_search(leftPart, debutBiblio)) {
                    selectedColumn = leftPart;
                } else {
                    continue; // Ligne inutile avant la biblio
                }
            }
        } else {
            // Ligne classique (pas en deux colonnes)
            selectedColumn.erase(0, selectedColumn.find_first_not_of(" \t\n"));
            selectedColumn.erase(selectedColumn.find_last_not_of(" \t\n") + 1);
        }

        // Détection du début de la biblio
        if (!inBiblio && regex_search(selectedColumn, debutBiblio)) {
            inBiblio = true;
            continue;
        }

        if (inBiblio) {
            if (regex_match(selectedColumn, ligneVide)) continue;
            biblio += selectedColumn + "\n";
        }
    }

    if (biblio.empty()) return "Bibliographie introuvable";

    biblio.erase(0, biblio.find_first_not_of(" \t\n"));
    biblio.erase(biblio.find_last_not_of(" \t\n") + 1);
    return biblio;
}




// === Fonction principale ===
int main(int argc, char* argv[]) {
    //  Gestion des arguments -t (texte) ou -x (xml)
    if (argc != 2 || (string(argv[1]) != "-t" && string(argv[1]) != "-x")) {
        cerr << " Utilisation incorrecte.\n";
        cerr << "Usage : ./parseur -t   (sortie texte)\n";
        cerr << "        ./parseur -x   (sortie XML)\n";
        return 1;
    }

    bool modeTexte = (string(argv[1]) == "-t");
    string dossierEntree = "converted_txt";
    string dossierSortie = modeTexte ? "parsed_txt" : "parsed_xml";

    //  Nettoyage et création du dossier de sortie
    if (fs::exists(dossierSortie)) {
        fs::remove_all(dossierSortie);
    }
    fs::create_directory(dossierSortie);

    cout << "\n Traitement des fichiers depuis '" << dossierEntree << "' vers '" << dossierSortie << "'\n";

    //  Parcourir les fichiers .txt
    for (const auto& fichier : fs::directory_iterator(dossierEntree)) {
        if (fichier.path().extension() == ".txt") {
            string nomOriginal = fichier.path().filename().string();
            string nomModifie = nomOriginal;
            replace(nomModifie.begin(), nomModifie.end(), ' ', '_');

            ifstream fichierEntree(fichier.path());
            if (!fichierEntree) {
                cerr << "Erreur ouverture fichier : " << nomOriginal << endl;
                continue;
            }

            stringstream buffer;
            buffer << fichierEntree.rdbuf();
            string contenu = buffer.str();

            //  Extraction
            string titre = extraireTitre(contenu);
            string resume = extraireAbstract(contenu);
            string biblio = extraireBiblio(contenu);

            //  Écriture dans fichier final
            string cheminSortie = dossierSortie + "/" + nomModifie;
            ofstream fichierSortie(cheminSortie);
            if (!fichierSortie) {
                cerr << "Erreur création fichier sortie : " << cheminSortie << endl;
                continue;
            }

            if (modeTexte) {
                fichierSortie << "Titre : " << titre << "\n\n";
                fichierSortie << "Abstract : " << resume << "\n";
            } else {
                fichierSortie << "<article>\n";
                fichierSortie << "  <preamble>" << nomModifie << "</preamble>\n";
                fichierSortie << "  <titre>" << titre << "</titre>\n";
                fichierSortie << "  <auteur>Auteur introuvable</auteur>\n";
                fichierSortie << "  <abstract>" << resume << "</abstract>\n";
                fichierSortie << "  <biblio>"<<biblio<<"</biblio>\n";
                fichierSortie << "</article>\n";
            }

            cout << " Fichier traité : " << nomOriginal << " ➜ " << cheminSortie << endl;
        }
    }

    cout << "\n Tous les fichiers ont été traités avec succès.\n";
    return 0;
}