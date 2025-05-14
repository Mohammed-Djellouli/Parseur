# Parseur d’articles scientifiques – Projet SCRUM LIA 2025

Ce projet vise à développer un **parseur** permettant d’analyser des **articles scientifiques convertis au format texte brut (.txt)** à partir de fichiers **PDF**. Il est réalisé dans le cadre du TP de Conception Logicielle (SCRUM) pour le **Laboratoire 
Informatique d’Avignon (LIA)**.

---

## Objectif

Créer un système qui :
- Convertit des fichiers PDF en '.txt' (via 'pdftotext' ou 'pdf2txt')
- Identifie automatiquement les **sections clés** d’un article :
  - Titre
  - Auteurs
  - Abstract
  - Introduction
  - Développement
  - Conclusion
  - Bibliographie

---

## Technologies utilisées

- Langage : 'Python'
- Outils de conversion :
  - ['pdftotext'](https://manpages.debian.org/testing/poppler-utils/pdftotext.1.en.html)
  - ['pdf2txt.py'](https://github.com/pdfminer/pdfminer.six)
- Environnement : Terminal sous GNU/Linux
- Méthodologie : **SCRUM**
- Contrôle de version : 'Git' et 'GitHub'

---

## Installation

### Prérequis :
- GNU/Linux
- Python 3
- 'pip'

### Installer les outils :

```bash
# Pour pdftotext
sudo apt update
sudo apt install poppler-utils

# Pour pdf2txt (pdfminer.six)
pip install pdfminer.six
