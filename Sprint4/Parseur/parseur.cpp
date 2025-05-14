#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <set>
#include <iterator>
#include <vector>
#include <iomanip>
#include <cstdlib> // Pour la fonction system

using namespace std;
namespace fs = std::filesystem;

// Définition de la fonction d'affichage du menu
int afficherMenuFichiers(const vector<string>& fichiers) {
    cout << "\n=== Sélection du fichier à traiter ===\n\n";

    // Afficher la liste des fichiers
    for (size_t i = 0; i < fichiers.size(); ++i) {
        cout << setw(3) << i+1 << ". " << fichiers[i] << endl;
    }

    cout << setw(3) << 0 << ". Traiter tous les fichiers" << endl;
    cout << setw(3) << fichiers.size()+1 << ". Quitter" << endl;

    cout << "\nVotre choix : ";
    int choix;
    cin >> choix;

    // Validation du choix
    while (choix < 0 || choix > static_cast<int>(fichiers.size()+1)) {
        cout << "Choix invalide. Réessayez : ";
        cin >> choix;
    }

    return choix;
}

// Définition de la fonction de conversion PDF → TXT
bool convertirPDFenTXT(const string& cheminPDF, const string& cheminTXT) {
    string commande = "pdftotext -layout -enc UTF-8 -nopgbrk \"" + cheminPDF + "\" \"" + cheminTXT + "\"";
    int result = system(commande.c_str());
    return result == 0;
}

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
    regex ignorerPattern(R"(University|Universiteit|Way|Technologies|Laboratory|Space|Institut|Department|School|Laboratoire|Centre|College|Inc|Place|Parkway|France|Canada|Belgium|JAPAN|USA|UK|arXiv|SRI|Google|Submitted|published|École|Polytechnique)", regex_constants::icase);
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

string extraireDiscussion(const string& contenu) {
    istringstream iss(contenu);
    string ligne, discussion = "";
    bool inDiscussion = false;

    regex debutDiscussion(R"(\b(Discussion|DISCUSSION|Discussion and Conclusion)\b)", regex_constants::icase);
    regex finDiscussion(R"(\b(Conclusion|CONCLUSION|References|Bibliography|Références|Works Cited|Acknowledgements?)\b)", regex_constants::icase);
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
            string left = match.prefix().str();
            string right = match.suffix().str();
            left.erase(0, left.find_first_not_of(" \t\n"));
            left.erase(left.find_last_not_of(" \t\n") + 1);
            right.erase(0, right.find_first_not_of(" \t\n"));
            right.erase(right.find_last_not_of(" \t\n") + 1);
            currentLine = (regex_search(right, debutDiscussion) || regex_search(right, finDiscussion)) ? right : left;
        }

        currentLine.erase(0, currentLine.find_first_not_of(" \t\n"));
        currentLine.erase(currentLine.find_last_not_of(" \t\n") + 1);

        if (!inDiscussion && regex_search(currentLine, debutDiscussion)) {
            inDiscussion = true;
            continue;
        }

        if (inDiscussion) {
            if (regex_search(currentLine, finDiscussion)) break;
            if (!currentLine.empty()) {
                discussion += currentLine + " ";
            }
        }
    }

    if (discussion.empty()) return "Discussion introuvable";

    discussion.erase(0, discussion.find_first_not_of(" \t\n"));
    discussion.erase(discussion.find_last_not_of(" \t\n") + 1);
    return discussion;
}

string extraireConclusion(const string& contenu) {
    istringstream iss(contenu);
    string ligne, conclusion = "";
    bool inConclusion = false;
    // Regex très souple pour toutes les variantes vues
    regex debutConclusion(R"((^|\s)\d{1,2}(\.|)?\s+(Conclusion|Conclusions)(\s+and\s+Future\s+Work)?\b)", regex_constants::icase);
    regex finConclusion(R"(^\s*(Acknowledgments?|Appendix|References?|Biography|Biosketch|Thanks))", regex_constants::icase);
    while (getline(iss, ligne)) {
        ligne.erase(remove(ligne.begin(), ligne.end(), '\r'), ligne.end());
        // Début détecté
        if (!inConclusion && regex_search(ligne, debutConclusion)) {
            inConclusion = true;
            continue; // skip title line
        }
        // Fin détectée
        if (inConclusion && regex_search(ligne, finConclusion)) break;

        if (inConclusion && !ligne.empty()) {
            conclusion += ligne + " ";
        }
    }
    // Nettoyage
    conclusion.erase(remove_if(conclusion.begin(), conclusion.end(), [](char c) {
        return static_cast<unsigned char>(c) < 32 && c != '\n' && c != '\t';
    }), conclusion.end());
    conclusion.erase(0, conclusion.find_first_not_of(" \t\n"));
    conclusion.erase(conclusion.find_last_not_of(" \t\n") + 1);
    return conclusion.empty() ? "Conclusion introuvable" : conclusion;
}


string extraireIntroduction(const string& contenu) {
    istringstream iss(contenu);
    string ligne;
    vector<string> lignes;

    // Regex pour séparer les colonnes dans les documents en deux colonnes
    regex separateurColonnes(" {7,}");

    //  Amélioration : détection plus souple du titre "Introduction"
    regex debutIntro(R"((^|\s)((\d+|[IVXLC]+)?[\.\s]*)?(Introduction)\b)", regex_constants::icase);


    // Détection d’une nouvelle section : numéro suivi d’un titre (ex. "2 Related Work")
    regex nouvelleSection(R"(^\s*$|^\s*(\d+|[IVXLC]+)[\.\s]+[A-Z].*|^\s*((\d+|[IVXLC]+)(\.\d+)?\.?)\s{1,}(Sentence|[A-Z][^\n]*)|^\s*Figure\s+\d+)", regex_constants::icase);


    // Mots-clés pour l’abstract ou résumé
    regex motCleAbstract(R"(\b(Abstract|Résumé)\b)", regex_constants::icase);

    // Étape 1 : charger toutes les lignes
    while (getline(iss, ligne)) {
        ligne.erase(remove(ligne.begin(), ligne.end(), '\r'), ligne.end()); // Nettoyer les retours chariot
        lignes.push_back(ligne);
    }

    int indexIntro = -1;

    // Étape 2 : repérer la ligne "Introduction"
    for (size_t i = 0; i < lignes.size(); ++i) {
        string l = lignes[i];
        l.erase(0, l.find_first_not_of(" \t\n"));
        l.erase(l.find_last_not_of(" \t\n") + 1);

        if (regex_search(l, debutIntro)) {
            indexIntro = i;
            break;
        }
    }

    if (indexIntro == -1) {
        // Fallback : essayer d'extraire le premier gros paragraphe avant la première section numérotée
        string tentativeIntro = "";
        for (size_t i = 0; i < lignes.size(); ++i) {
            string test = lignes[i];
            test.erase(0, test.find_first_not_of(" \t\n"));
            test.erase(test.find_last_not_of(" \t\n") + 1);

            // Arrêt si on tombe sur une vraie section (comme "1 Related Work", "2 Methodology", etc.)
            if (regex_match(test, regex(R"(^\s*(\d+|[IVXLC]+)[\.\s]+[A-Z].*)"))) {
                break;
            }

            if (!test.empty()) {
                tentativeIntro += test + " ";
            }
        }

        if (!tentativeIntro.empty()) {
            // Nettoyage
            tentativeIntro.erase(0, tentativeIntro.find_first_not_of(" \t\n"));
            tentativeIntro.erase(tentativeIntro.find_last_not_of(" \t\n") + 1);
            return tentativeIntro;
        }

        return "Introduction introuvable";
    }


    string introduction = "";

    // Étape 3 : remonter les lignes avant "Introduction" si elles sont liées (ex. résumé structuré en colonnes)
    for (int i = indexIntro - 1; i >= 0; --i) {
        string line = lignes[i];
        smatch match;

        if (regex_search(line, motCleAbstract)) break;
        if (!regex_search(line, match, separateurColonnes)) break;

        string left = match.prefix().str();
        string right = match.suffix().str();

        left.erase(0, left.find_first_not_of(" \t\n"));
        left.erase(left.find_last_not_of(" \t\n") + 1);
        right.erase(0, right.find_first_not_of(" \t\n"));
        right.erase(right.find_last_not_of(" \t\n") + 1);

        if (left.empty() && right.empty()) break;

        introduction = (left + " " + right + " ") + introduction;
    }

    // Étape 4 : concaténer les lignes de l’introduction en descendant
    string introLigne = lignes[indexIntro];
    string introTexte = introLigne;
    introTexte.erase(0, introTexte.find_first_not_of(" \t\n"));
    introTexte.erase(introTexte.find_last_not_of(" \t\n") + 1);

    // Si la ligne contient autre chose que juste "Introduction", ne pas l’ajouter brute
    if (!regex_match(introTexte, debutIntro)) {
        introduction += introLigne + " ";
    }


    for (size_t i = indexIntro + 1; i < lignes.size(); ++i) {
        string line = lignes[i];

        // Nettoyer la ligne
        string test = line;
        test.erase(0, test.find_first_not_of(" \t\n"));
        test.erase(test.find_last_not_of(" \t\n") + 1);

        // Stop si nouvelle section OU si ligne complètement vide
        if (test.empty() || regex_search(test, nouvelleSection)) break;

        smatch match;
        string left = line, right = "";

        if (regex_search(line, match, separateurColonnes)) {
            left = match.prefix().str();
            right = match.suffix().str();
        }

        left.erase(0, left.find_first_not_of(" \t\n"));
        left.erase(left.find_last_not_of(" \t\n") + 1);
        right.erase(0, right.find_first_not_of(" \t\n"));
        right.erase(right.find_last_not_of(" \t\n") + 1);

        string merged = left;
        if (!right.empty()) merged += " " + right;

        if (!merged.empty()) introduction += merged + " ";
    }


    // Étape 5 : nettoyage final
    introduction.erase(0, introduction.find_first_not_of(" \t\n"));
    introduction.erase(introduction.find_last_not_of(" \t\n") + 1);

    return introduction;
}

// Fonction pour traiter un fichier spécifique
void traiterFichier(const string& cheminFichier, const string& dossierSortie, bool modeTexte) {
    string nomOriginal = fs::path(cheminFichier).filename().string();
    string nomModifie = nomOriginal;
    replace(nomModifie.begin(), nomModifie.end(), ' ', '_');

    ifstream fichierEntree(cheminFichier);
    if (!fichierEntree) {
        cerr << "Erreur ouverture fichier : " << nomOriginal << endl;
        return;
    }

    stringstream buffer;
    buffer << fichierEntree.rdbuf();
    string contenu = buffer.str();

    // Extraction
    string titre = extraireTitre(contenu);
    string resume = extraireAbstract(contenu);
    string biblio = extraireBiblio(contenu);
    vector<string> auteurs = extraireAuteurs(contenu);
    string discussion = extraireDiscussion(contenu);
    string conclusion = extraireConclusion(contenu);
    string introduction = extraireIntroduction(contenu);

    // Écriture dans fichier final
    string extension = modeTexte ? ".txt" : ".xml";
    string cheminSortie = dossierSortie + "/" + fs::path(nomModifie).stem().string() + extension;

    ofstream fichierSortie(cheminSortie);
    if (!fichierSortie) {
        cerr << "Erreur création fichier sortie : " << cheminSortie << endl;
        return;
    }

    if (modeTexte) {
        fichierSortie << "Titre : " << titre << "\n\n";
        fichierSortie << "Introduction : " << introduction << "\n\n";
        fichierSortie << "Abstract : " << resume << "\n";
        fichierSortie << "Discussion : " << discussion << "\n";
        fichierSortie << "Preamble : " << nomModifie << "\n";
        for (const string& a : auteurs) {
            fichierSortie << "Auteurs : " << a << "\n";
        }
        fichierSortie << "Conclusion : " << conclusion << "\n\n";
        fichierSortie << "Biblio : " << biblio << "\n";
    } else {
        fichierSortie << "<article>\n";
        fichierSortie << "  <preamble>" << nomModifie << "</preamble>\n";
        fichierSortie << "  <titre>" << titre << "</titre>\n";
        for (const string& a : auteurs) {
            fichierSortie << "  <auteur>" << a << "</auteur>\n";
        }
        fichierSortie << "  <introduction>" << introduction << "</introduction>\n";
        fichierSortie << "  <abstract>" << resume << "</abstract>\n";
        fichierSortie << "  <discussion>" << discussion << "</discussion>\n";
        fichierSortie << "  <conclusion>" << conclusion << "</conclusion>\n";
        fichierSortie << "  <biblio>" << biblio << "</biblio>\n";
        fichierSortie << "</article>\n";
    }

    cout << " Fichier traité : " << nomOriginal << " ➜ " << cheminSortie << endl;
}

// === Fonction principale ===
int main(int argc, char* argv[]) {
    if (argc != 2 || (string(argv[1]) != "-t" && string(argv[1]) != "-x")) {
        cerr << " Utilisation incorrecte.\n";
        cerr << "Usage : ./parseur -t   (sortie texte)\n";
        cerr << "        ./parseur -x   (sortie XML)\n";
        return 1;
    }

    bool modeTexte = (string(argv[1]) == "-t");
    string dossierPDF = "pdf";
    string dossierSortie = modeTexte ? "parsed_txt" : "parsed_xml";

    // Créer dossier de sortie s'il n'existe pas
    if (!fs::exists(dossierSortie)) {
        fs::create_directory(dossierSortie);
    }

<<<<<<< HEAD
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
=======
    // Lister les fichiers PDF
    vector<string> fichiersPDF;
    vector<string> cheminsPDF;
    for (const auto& fichier : fs::directory_iterator(dossierPDF)) {
        if (fichier.path().extension() == ".pdf") {
            fichiersPDF.push_back(fichier.path().filename().string());
            cheminsPDF.push_back(fichier.path().string());
>>>>>>> 761d2fe2eb9e89ebbfaad0f314fa9e6ea5d97ed3
        }
    }

    if (fichiersPDF.empty()) {
        cerr << "Aucun fichier PDF trouvé dans '" << dossierPDF << "'\n";
        return 1;
    }

    // Afficher menu
    int choix = afficherMenuFichiers(fichiersPDF);
    if (choix == fichiersPDF.size() + 1) {
        cout << "Programme terminé.\n";
        return 0;
    }

    vector<string> fichiersATraiter;
    if (choix == 0) {
        fichiersATraiter = cheminsPDF;
    } else {
        fichiersATraiter.push_back(cheminsPDF[choix - 1]);
    }

    for (const auto& cheminPDF : fichiersATraiter) {
        string nomSansExtension = fs::path(cheminPDF).stem().string();
        string cheminTempTxt = "temp_" + nomSansExtension + ".txt";

        cout << "Conversion de : " << cheminPDF << endl;
        if (!convertirPDFenTXT(cheminPDF, cheminTempTxt)) {
            cerr << "Erreur lors de la conversion du PDF.\n";
            continue;
        }

        // Parser le fichier temporaire converti
        traiterFichier(cheminTempTxt, dossierSortie, modeTexte);

        // Nettoyer le fichier temporaire
        fs::remove(cheminTempTxt);
    }

    cout << "\nTous les fichiers ont été traités.\n";
    return 0;
}
