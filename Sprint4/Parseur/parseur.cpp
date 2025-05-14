#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <set>
#include <iterator>

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


string extraireCorps(const string& contenu) {
    istringstream iss(contenu);
    string ligne, corps = "";
    bool inCorps = false;
    bool debutTrouve = false; 

regex debutClassique(R"(^\s*(2|II)(\.\d+)?\s*[\.\-\–]?\s+(Related Work|From Full Sentence to Compressed
Sentence|Performance Measure, Corpora for Evaluation|Support Vector Machine|THE IMPORTANCE OF LINGUISTIC|Previous Work|Model Architectures|RELATED WORK|Sentence Compression)\b)", regex_constants::icase);

    regex abstractFin(R"(^\s*abstract\s*$)", std::regex_constants::icase); 
    bool apresAbstract = false;

    regex finCorps(R"(\b(Conclusion|Conclusions|Discussion|Discussions|Summary|Future Work|Acknowledgments|Acknowledgements|References|Bibliography)\b)", std::regex_constants::icase);
    regex ligneVide(R"(^\s*$)");


    while (getline(iss, ligne)) {

        ligne.erase(std::remove(ligne.begin(), ligne.end(), '\r'), ligne.end());

        if (!debutTrouve) {
            if (regex_search(ligne, debutClassique)) {
                inCorps = true;
                debutTrouve = true;
                continue; 
            }
        }

        if (inCorps) {
            if (regex_search(ligne, finCorps)) {
                break; 
            }
            if (!regex_match(ligne, ligneVide)) {
                corps += ligne + "\n";
            }
        }
    }

    if (corps.empty()) {
        return "Corps introuvable";
    }

    size_t first = corps.find_first_not_of(" \t\n\r");
    if (string::npos == first) {
        return "Corps introuvable (que des espaces)";
    }
    size_t last = corps.find_last_not_of(" \t\n\r");
    corps = corps.substr(first, (last - first + 1));

    return corps;
}

    

vector<string> extraireAuteurs(const string& contenu) {
    istringstream iss(contenu);
    string ligne;
    vector<string> lignes;
    regex emailPattern(R"(([\w\.-]+,?[\w\.-]*)\s*@[\w\.-]+\.\w+)");
    regex ignorerPattern(R"(University|Universiteit|Way|Technologies|Laboratory|Space|Institut|Department|School|Laboratoire|Centre|College|Inc|Place|Parkway|France|Canada|Belgium|JAPAN|USA|UK|arXiv|SRI|Google|Submitted|published|École|Polytechnique)", regex_constants::icase);
    regex ligneVide(R"(^\s*$)");
    regex ligneNom(R"(([A-Z][a-z]+([ -][A-Z][a-z]+)+))");
    regex separateurColonnes(" {7,}");

    while (getline(iss, ligne)) {
        ligne.erase(remove(ligne.begin(), ligne.end(), '\r'), ligne.end());
        lignes.push_back(ligne);
    }

    set<string> auteursTrouves;

    // 1. Recherche autour des emails
    for (size_t i = 0; i < lignes.size(); ++i) {
    smatch emailMatch;
    if (regex_search(lignes[i], emailMatch, emailPattern)) {
        // Ligne contenant un email : extraire la partie gauche (avant l'email)
        string ligneAvecEmail = lignes[i];
        size_t posEmail = ligneAvecEmail.find(emailMatch[0].str());
        string partieGauche = ligneAvecEmail.substr(0, posEmail);

        // Nettoyage
        partieGauche.erase(0, partieGauche.find_first_not_of(" \t"));
        partieGauche.erase(partieGauche.find_last_not_of(" \t") + 1);
        partieGauche = regex_replace(partieGauche, regex(R"([0-9]+)"), "");
        partieGauche = regex_replace(partieGauche, regex(R"([,;])"), "");

        // Vérifie si la partie gauche ressemble à un nom
        if (!partieGauche.empty() && !regex_search(partieGauche, ignorerPattern) && regex_search(partieGauche, ligneNom)) {
            auteursTrouves.insert(partieGauche);
        }
    }
}


    // 2. Fallback haut du fichier (colonnes)
    if (auteursTrouves.empty()) {
        for (size_t i = 0; i < min((size_t)15, lignes.size()); ++i) {
            string ligneCourante = lignes[i];
            if (regex_match(ligneCourante, ligneVide)) continue;

            if (regex_search(ligneCourante, regex(R"(Integer|compression|Global|approach|inference|sentence|summary|document|scoring|method|evaluation|update|reduction|system|learning|representation|model)", regex_constants::icase))) {
                continue;
            }

            if (regex_search(ligneCourante, ligneNom) && !regex_search(ligneCourante, ignorerPattern)) {
                auteursTrouves.insert(ligneCourante);
                continue;
            }

            smatch match;
            if (regex_search(ligneCourante, match, separateurColonnes)) {
                string left = match.prefix().str();
                string right = match.suffix().str();

                for (string part : {left, right}) {
                    part.erase(0, part.find_first_not_of(" \t"));
                    part.erase(part.find_last_not_of(" \t") + 1);
                    part = regex_replace(part, regex(R"([\\,\[\]]+)"), "");

                    stringstream ss(part);
                    string nom;
                    while (getline(ss, nom, ',')) {
                        nom.erase(0, nom.find_first_not_of(" \t"));
                        nom.erase(nom.find_last_not_of(" \t") + 1);

                        int majCount = count_if(nom.begin(), nom.end(), ::isupper);
                        if (majCount >= 2 && !regex_search(nom, ignorerPattern) && regex_search(nom, ligneNom)) {
                            auteursTrouves.insert(nom);
                        }
                    }
                }
            }
        }
    }

    return vector<string>(auteursTrouves.begin(), auteursTrouves.end());
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
            string corps = extraireCorps(contenu);
            //  Extraction
            string titre = extraireTitre(contenu);
            string resume = extraireAbstract(contenu);
            string biblio = extraireBiblio(contenu);
            vector<string> auteurs = extraireAuteurs(contenu);


            //  Écriture dans fichier final
            string extension = modeTexte ? ".txt" : ".xml";
            string cheminSortie = dossierSortie + "/" + fs::path(nomModifie).stem().string() + extension;

            ofstream fichierSortie(cheminSortie);
            if (!fichierSortie) {
                cerr << "Erreur création fichier sortie : " << cheminSortie << endl;
                continue;
            }

            if (modeTexte) {
                fichierSortie << "Titre : " << titre << "\n\n";
                fichierSortie << "Abstract : " << resume << "\n";
                fichierSortie <<"Preamble : "<<nomModifie<< "\n";
                for (const string& a : auteurs) {
                    fichierSortie << "Aueturs : " << a << "\n";
                }
                fichierSortie << "Biblio : "<<biblio<< "\n";
            } else {
                fichierSortie << "<article>\n";
                fichierSortie << "  <preamble>" << nomModifie << "</preamble>\n";
                fichierSortie << "  <titre>" << titre << "</titre>\n";
                for (const string& a : auteurs) {
                    fichierSortie << "  <auteur>" << a << "</auteur>\n";
                }
                fichierSortie << "  <abstract>" << resume << "</abstract>\n";
                fichierSortie << "  <corps>" << corps << "</corps>\n";
                fichierSortie << "  <biblio>"<<biblio<<"</biblio>\n";
                fichierSortie << "</article>\n";
            }

            cout << " Fichier traité : " << nomOriginal << " ➜ " << cheminSortie << endl;
        }
    }

    cout << "\n Tous les fichiers ont été traités avec succès.\n";
    return 0;
}