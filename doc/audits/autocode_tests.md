# Audit des tests et de la traçabilité autoCode

## Objet et périmètre

Cet audit développe la dernière faiblesse relevée dans la note d'architecture autoCode : absence de
corpus de parseur, de tests par injection de panne et de manifeste des entrées et outils
([autoCode.md](../architecture/autoCode.md#L32)).

L'analyse porte sur l'outil hôte, son intégration à BSD `bmake`, ses parseurs et le remplacement des
fichiers générés. Elle correspond à la révision `8261d21`. Aucun code n'a été modifié et aucune
génération ou compilation n'a été exécutée pendant l'audit.

## Conclusion

La correction recommandée tient en trois lots complémentaires : caractériser le comportement actuel
avec un corpus exécutant le vrai binaire, rendre les pannes hôte déterministes dans une variante de
test, puis enregistrer les entrées exactes de chaque génération dans un manifeste stable.

Le corpus doit précéder les refactorisations. Il donnera une référence observable aux changements du
tokenizer, des parseurs et des opérations de fichiers, sans dupliquer leur logique dans les tests.

## État actuel

`autoCode` est compilé par Clang puis exécuté par le build à partir d'un fichier de configuration
propre à la cible ([autoCode.mk](../../mk/autoCode.mk#L48)). Le build suit les listes dynamiques de
fichiers et leurs chemins, mais ne conserve ni empreinte de contenu ni version des outils ayant
produit les sorties.

Le programme traite successivement les options, les erreurs, les modules et les fichiers portant les
balises, puis remplace toutes les destinations à la fin
([autoCode.c](../../srcs/autoCode/autoCode.c#L54)). La majorité des erreurs appelle directement
`exit(1)`. Des tests unitaires isolés demanderaient donc une refactorisation préalable, tandis qu'un
test boîte noire peut déjà vérifier le contrat réel du programme.

Deux comportements justifient particulièrement cette couverture :

- une balise ayant un mauvais nombre de tokens interrompt seulement la lecture locale avec `break`,
  sans imposer systématiquement un échec global
  ([parseTag.c](../../srcs/autoCode/parseTag.c#L165)) ;
- le remplacement supprime la destination avant un `rename()` dont le résultat n'est pas vérifié,
  ce qui peut perdre le fichier original en cas de panne
  ([fileUtility.c](../../srcs/autoCode/fileUtility.c#L119)).

## Lot 1 — Corpus de régression

Créer `tests/autoCode/` et une cible explicite `bmake test_autoCode`. Le lanceur doit copier chaque
fixture dans un répertoire isolé sous `build/`, créer sa configuration, exécuter le binaire
construit par le projet et comparer les résultats attendus.

Le corpus minimal doit couvrir :

- une génération complète valide et sa seconde exécution sans changement ;
- les options absentes, dupliquées, inconnues et mal formées ;
- les commandes, types, niveaux d'exécution, adresses et noms de modules invalides ;
- les erreurs dupliquées, niveaux inconnus, messages trop longs et mauvais nombres de champs ;
- les balises absentes, dupliquées, inconnues, imbriquées, non terminées ou mal formées ;
- les entrées GPIO invalides et les limites maximales de chaque collection ;
- les lignes aux limites du buffer, puis les lignes trop longues après ajout de leur détection.

Chaque cas positif doit comparer les fichiers générés octet par octet avec des références
versionnées. Chaque cas négatif doit vérifier un code de sortie non nul, un diagnostic distinctif,
l'absence de modification des destinations et le nettoyage des fichiers temporaires.

Une seconde cible `bmake test_autoCode_sanitize` doit compiler l'outil avec AddressSanitizer et
UndefinedBehaviorSanitizer, puis rejouer le même corpus. Les fixtures et les assertions restent donc
identiques entre l'exécution normale et l'exécution instrumentée.

## Lot 2 — Injection déterministe des pannes

Les permissions du système de fichiers suffisent à tester quelques erreurs d'ouverture, mais pas à
déclencher de façon fiable la N-ième panne d'écriture, de fermeture, d'allocation ou de renommage.

Une petite couche privée à `srcs/autoCode/` doit centraliser les opérations hôte. La production
utilise les fonctions standard ; le binaire de test lie une implémentation capable d'échouer sur une
opération et un rang choisis. Cette couche reste interne à autoCode et ne devient ni une interface
du firmware ni un objectif de portabilité générale.

Les points injectés doivent inclure au minimum l'ouverture, la lecture, l'écriture, la fermeture,
l'allocation, la réallocation, la suppression et le renommage. Pour chaque point, le test exige :

- un arrêt en erreur sans création du stamp de génération ;
- des destinations originales strictement inchangées ;
- aucun fichier temporaire résiduel ;
- un diagnostic indiquant l'opération et le fichier concernés.

Avant ces tests, le remplacement doit renommer le temporaire directement sur la destination et
contrôler le résultat. L'ancien fichier ne doit jamais être supprimé avant que le remplacement
puisse réussir. Les erreurs de lecture, d'écriture, de vidage et de fermeture doivent également être
distinguées d'une fin de fichier normale.

## Lot 3 — Manifeste reproductible

Après une génération réussie, le build doit produire un manifeste sous le répertoire de la cible. Il
doit contenir :

- la cible matérielle et la révision Git, avec indication d'un arbre modifié ;
- l'empreinte du binaire autoCode et les versions de Clang et de BSD `bmake` ;
- les chemins triés et les empreintes des sources autoCode et des interfaces compilées avec lui ;
- les chemins triés et les empreintes des fichiers de configuration effectivement consommés ;
- les empreintes finales des destinations générées.

Le manifeste ne doit pas incorporer de date dans son identité : deux générations issues des mêmes
entrées doivent produire le même contenu. Il doit être écrit dans un temporaire, comparé à sa
version précédente puis remplacé seulement s'il a changé. Le stamp autoCode ne doit être créé
qu'après cette étape.

Ce manifeste complète les fichiers `*.deps` actuels : ceux-ci déclenchent correctement la génération
quand une liste change, mais ne constituent pas une preuve autonome des contenus employés
([autoCode.mk](../../mk/autoCode.mk#L89)).

## Ordre de réalisation et validation

1. Ajouter le corpus boîte noire et figer les sorties actuellement valides.
2. Corriger les erreurs découvertes, notamment les balises mal formées et les remplacements risqués.
3. Centraliser les opérations hôte et ajouter les scénarios d'injection de panne.
4. Générer le manifeste déterministe et tester sa stabilité ainsi que son invalidation.
5. Rejouer la génération réelle et construire intégralement le firmware AVR.

Validation finale proposée :

```sh
bmake test_autoCode
bmake test_autoCode_sanitize
bmake autoCode_alone
bmake autoCode_alone
bmake clean && bmake
git diff --check
```

Les deux exécutions d'`autoCode_alone` doivent laisser les mêmes sorties et le même manifeste. La
construction AVR valide leur intégration au firmware, mais elle ne remplace ni le corpus négatif ni
les scénarios de panne du programme hôte.
