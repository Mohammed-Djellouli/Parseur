#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

int main() {
    auto start = high_resolution_clock::now();

    ifstream fichier("LoremIpsum.txt");
    if (!fichier) {
        cerr << "Erreur ouverture du fichier !" << endl;
        return 1;
    }

    string contenu((istreambuf_iterator<char>(fichier)), istreambuf_iterator<char>());
    fichier.close();

    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;

    cout << fixed << setprecision(6);
    cout << "Temps ecoule pour lire le fichier: " << elapsed.count() << " secondes" << endl;

    return 0;
}
