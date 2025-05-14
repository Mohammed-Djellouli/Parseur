#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm> 

using namespace std;
namespace fs = std::filesystem;
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

    cout << "\n[DEBUG] Lignes chargées: " << lignes.size() << "\n";

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
            cout << "[DEBUG] Mot-clé 'Abstract' détecté !" << endl;
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
                if (regex_search(currentLine, finPossible) || regex_search(currentLine, finPossible2)) {
                    
                    break;
                }
            if (startsFarRight) {
                
                continue;
            }

            
            if (regex_search(currentLine, finPossible)) {
               
                break;
            }
            if (regex_search(currentLine, finPossible2)) {
                
                break;
            }

            if (!currentLine.empty()) {
                abstract += currentLine + " ";
                
            } else {
                
            }
        }
    }

    
    if (abstract.empty()) {
        cout << "[DEBUG] Aucun abstract trouvé." << endl;
        return "Abstract introuvable";
    }

    abstract.erase(0, abstract.find_first_not_of(" \t\n"));
    abstract.erase(abstract.find_last_not_of(" \t\n") + 1);

    cout << "\n[DEBUG] Résultat final de l'abstract:\n" << abstract << "\n";
    return abstract;
}
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

        if (lower.find("from:") != string::npos ||
            lower.find("submitted") != string::npos ||
            lower.find("published") != string::npos ||
            lower.find("journal") != string::npos ||
            lower.find("copyright") != string::npos ||
            lower.find("www.") != string::npos ||
            lower.find("http") != string::npos ||
            lower.find(".org") != string::npos ||
            lower.find(".com") != string::npos ||
            lower.find("@") != string::npos ||
            lower.find("author") != string::npos ||
            lower.find("email") != string::npos ||
            lower.find("proceedings") != string::npos) {
            continue;
        }

        lignesTitre.push_back(ligne);
        if (lignesTitre.size() >= 2) break;
    }

    string titre = "";
    for (size_t i = 0; i < lignesTitre.size(); ++i) {
        titre += lignesTitre[i];
        if (i != lignesTitre.size() - 1)
            titre += " ";
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


int main() {
    string dossierEntree = "converted_txt";
    string dossierSortie = "parsed_txt";

    // Nettoyage et création dossier sortie (Collègue 1)
    if (fs::exists(dossierSortie)) {
        fs::remove_all(dossierSortie);
    }
    fs::create_directory(dossierSortie);

    cout << "Fichiers traités du dossier '" << dossierEntree << "':" << endl;

    // Parcourir les fichiers .txt (Collègue 2)
    for (const auto& fichier : fs::directory_iterator(dossierEntree)) {
        if (fichier.path().extension() == ".txt") {
            string nomOriginal = fichier.path().filename().string();

            
            ifstream fichierEntree(fichier.path());
            if (!fichierEntree) {
                cerr << "Erreur ouverture fichier: " << nomOriginal << endl;
                continue;
            }

            stringstream buffer;
            buffer << fichierEntree.rdbuf();
            string contenu = buffer.str();

            // Extraction des données (Collègue 3)
            string titre = extraireTitre(contenu);
            string resume = extraireAbstract(contenu);

            //  Écriture dans fichier structuré (Collègue 4)
            string nomModifie = nomOriginal;
            replace(nomModifie.begin(), nomModifie.end(), ' ', '_');  

            string cheminSortie = dossierSortie + "/" + nomModifie;
            ofstream fichierSortie(cheminSortie);
            if (!fichierSortie) {
                cerr << "Erreur création fichier sortie : " << cheminSortie << endl;
                continue;
            }

            fichierSortie << "Titre : " << titre << "\n\n";
            fichierSortie << "Abstract : " << resume << "\n";
            fichierSortie.close();

            
            cout << "\n==============================================\n";
            cout << "Fichier traité : " << nomOriginal << "\n\n";
            cout << "Titre extrait : " << titre << "\n\n";
            cout << "Abstract extrait : " << resume << endl;
            cout << "Résultat sauvegardé dans : " << cheminSortie << endl;
            cout << "==============================================\n";
        }
    }

    cout << "\n Tous les fichiers ont été traités avec succès.\n";
    return 0;
}
