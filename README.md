Parseur de PDF Scientifiques - Projet C++
Description
Ce programme en C++ permet d'extraire automatiquement les principales sections d’un article scientifique au format PDF : titre, auteurs, résumé (abstract), introduction, discussion, conclusion et bibliographie.

Il convertit d'abord le PDF en texte brut, puis analyse ce texte pour générer un fichier de sortie au format .txt ou .xml.

Prérequis
Un compilateur C++ compatible C++17 (ou plus récent)

Le programme pdftotext installé sur votre machine (inclus dans le paquet poppler-utils)

Installation de pdftotext (sous Ubuntu/Debian) :
sudo apt install poppler-utils

Compilation
Utilisez la commande suivante pour compiler le programme :
g++ -std=c++17 parseur.cpp -o parseur

Structure du projet
Le dossier du projet doit contenir :

parseur.cpp (le code source du programme)

pdf/ (un dossier contenant les fichiers PDF à traiter)

parsed_txt/ (sera créé automatiquement si vous choisissez le format texte)

parsed_xml/ (sera créé automatiquement si vous choisissez le format XML)

Utilisation
Lancez le programme avec l'une des deux options suivantes : 
./parseur -t     → pour une sortie au format texte (.txt)  
./parseur -x     → pour une sortie au format XML (.xml)
Fonctionnement :
Le programme recherche tous les fichiers PDF dans le dossier pdf/.

Il affiche un menu pour vous permettre de choisir un fichier ou de tous les traiter.

Chaque PDF est converti temporairement en fichier texte avec pdftotext.

Le programme analyse le contenu texte pour en extraire les sections.

Un fichier de sortie est créé dans parsed_txt/ ou parsed_xml/.

Le fichier texte temporaire est automatiquement supprimé après traitement.

Informations extraites
Le programme tente d'extraire les éléments suivants :

Titre de l’article

Auteurs (noms détectés autour des adresses email ou en haut du fichier)

Résumé (abstract ou résumé)

Introduction

Discussion

Conclusion

Bibliographie (References, Bibliography, etc.)

Exemple de sortie XML
<article> <preamble>Nom_Article.pdf</preamble> <titre>Exemple de titre</titre> <auteur>Jean Dupont</auteur> <auteur>Marie Curie</auteur> <introduction>...</introduction> <abstract>...</abstract> <discussion>...</discussion> <conclusion>...</conclusion> <biblio>...</biblio> </article>
Limitations
Le programme repose sur des règles heuristiques (regex) : il peut ne pas fonctionner parfaitement sur des PDF très mal structurés.

Il ne traite pas les figures, images ou équations mathématiques.

Les résultats dépendent fortement de la mise en page du PDF.

Auteur
Développé dans le cadre d’un projet universitaire en C++ à l’Université d’Avignon.
