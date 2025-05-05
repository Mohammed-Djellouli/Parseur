#include <iostream>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

int main() {
    auto start = high_resolution_clock::now();

    for (long long i = 0; i < 100000000; ++i) {
        // juste une boucle vide
    }

    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;

    cout << fixed << setprecision(6);
    cout << "Temps ecoule pour la boucle: " << elapsed.count() << " secondes" << endl;

    return 0;
}
