# Audit de la structure et des noms du projet

Date : 12 septembre 2026
Branche auditée : `codex/files-directories`
Révision auditée : `8e7c02f`

## Périmètre et verdict

L'audit couvre l'arborescence suivie par Git, les fichiers ignorés présents dans le checkout, les
règles de nommage, les dépendances entre couches et la sélection des sources par BSD `bmake`.

La structure générale est saine et correspond au découpage annoncé : `user`, `system`, `hal`,
`interfaces`, `tm_libc` et `autoCode` ont des responsabilités identifiables. La chaîne matérielle
`target -> board -> MCU -> architecture` est également construite explicitement par le build.

La cohérence reste partielle. Les écarts principaux concernent un cycle entre `tm_libc` et le
système, la présence de dépendances ascendantes générées dans `sysCore`, une sélection des sources
trop globale et l'absence de convention uniforme pour les noms de fichiers et de répertoires.

## Structure conforme à l'architecture

- Les tâches utilisent les API `sysCall` et n'incluent pas directement le HAL.
- Les services et leurs commandes utilisent `sysCall` sans inclure le HAL.
- `interfaces/` ne dépend que de la bibliothèque C et de ses propres en-têtes.
- Le HAL ne dépend ni des services, ni de `sysCall`, ni des tâches.
- `hal/arch`, `hal/mcu`, `hal/board` et `hal/drivers` matérialisent correctement les niveaux de
  spécialisation matérielle.
- Les cibles `test1` et `test_noscli` sélectionnent toutes deux la chaîne
  `arduinoMega -> atmega2560 -> avr8`.
- Les sorties de compilation et les journaux ordinaires sont placés sous `build/`.

## Écarts architecturaux

### 1. Cycle entre `tm_libc`, `sysCall` et le HAL

`srcs/tm_libc/tm_snprintf.c` inclut `system/sysCall/sc_modules.h` et appelle `sc_coopYield()`.
Le même fichier dépend aussi de `hal/public/tmlibc.h`. En sens inverse,
`srcs/system/sysCall/sc_modules.c` inclut `tm_libc/tm_string.h`.

Le graphe contient donc notamment le cycle suivant :

```text
tm_libc -> sysCall -> sysCore -> tm_libc
```

Ce cycle ne bloque pas la construction actuelle, mais il affaiblit le statut transversal de
`tm_libc`, complique l'initialisation et rend une future composition matérielle plus difficile.

### 2. Dépendances ascendantes générées dans `sysCore`

La région générée de `srcs/system/sysCore/modules_list.h` inclut directement les en-têtes des
services et des tâches. Cette solution permet au registre statique de référencer leurs points
d'entrée, mais elle fait dépendre le cœur de ses consommateurs.

La composition générée devrait à terme être isolée dans une unité dédiée, distincte des composants
génériques de `sysCore`, sans ajouter d'allocation dynamique ni de coût d'exécution.

### 3. Sélection des sources trop globale

`mk/path_files.mk` ajoute globalement `system`, `hal/drivers` et `user/tasks` à `PATHS_SOURCES`.
`mk/sources.mk` découvre ensuite récursivement tous leurs fichiers `.c`.

Le profil `test_noscli` retire bien SCLI de l'enregistrement autoCode, mais `scli.c`, ses commandes
et toutes les tâches sont encore compilés. L'élimination des sections inutilisées limite le contenu
du firmware lié ; elle ne constitue toutefois pas une véritable sélection des unités de traduction.

Cette stratégie est acceptable avec la cible unique actuelle, mais elle ne garantit pas que les
drivers et services non sélectionnés restent compilables avec une future chaîne matérielle.

### 4. Contrôle de frontières incomplet

`conf/system_header_allow.conf` protège cinq en-têtes critiques. Le contrôle passe et apporte une
défense utile, mais il ne représente pas l'ensemble des règles de couches. Il ne détecterait pas,
par exemple, toute nouvelle dépendance `HAL -> services` ou `interfaces -> HAL`.

Le message « Checking forbidden system critical includes » doit donc être compris comme un contrôle
ciblé, et non comme une validation exhaustive du graphe architectural.

## Cohérence des noms

Les symboles publics respectent globalement les préfixes documentés et le style camelCase. Les noms
de fichiers et de répertoires mélangent cependant plusieurs conventions :

| Domaine | Exemples actuels | Observation |
| --- | --- | --- |
| Couches | `autoCode`, `sysCall`, `sysCore`, `tm_libc` | camelCase et snake_case mélangés |
| Matériel | `avr8`, `atmega2560`, `arduinoMega` | identifiants sans règle commune explicite |
| Drivers externes | `lcd_AMC2004`, `rtc_ZS042` | type en minuscules, modèle en majuscules |
| Timers | `timerSTC`, `timerSched` | acronymes et mots concaténés |
| Initialisation HAL | `avr8_init.c`, `arduinoMega_init.c` | suffixe de fichier en snake_case |
| Initialisation cible | `test1Init.c`, `test_noscliInit.c` | suffixe de fichier en camelCase |

Les écarts concrets les plus nets sont les suivants :

- le répertoire `test_noscli`, le fichier `test-noscli_init.rc` et le symbole
  `test_noscliInit()` utilisent trois compositions différentes ;
- le commentaire de `test-noscli_init.rc` désigne encore la cible `test1` ;
- `tmlibc.h` et `tmlibc.c` ne reprennent pas la forme `tm_libc` du nom de couche ;
- `TaskMate.c` est le seul fichier C principal en PascalCase ;
- `doc/architecture/interface.md` est au singulier alors que la couche et la règle sont nommées
  `interfaces` ;
- `test1Init.h` et `test_noscliInit.h` n'ont pas de garde d'inclusion ;
- les deux fichiers `target/*/define.h` déclarent `@file target_define.h`, qui n'est pas leur nom.

Il n'existe pas de règle générale documentée pour les noms de fichiers et de répertoires. Avant une
campagne de renommage, il faut donc fixer une convention et ses exceptions : noms de composants,
identifiants matériels issus des constructeurs, acronymes, fichiers d'initialisation et profils de
cible. Une normalisation mécanique sans cette décision déplacerait seulement les incohérences.

## Documentation et noms obsolètes

- `AGENTS.md` référence `doc/arch_boundary_enforcement.md`, fichier absent.
- `AGENTS.md` et `doc/rules/style.md` référence `doc/licence_header.txt`, fichier absent.
- `doc/rules/style.md` référence `make_variable_prefixes.md`; le fichier réel est
  `make_prefixes.md`.
- `AGENTS.md` documente `bmake HWT=<target>`; le sélecteur réel est `VAL_TARGET`.
- `doc/audits/firmware_flash.md` référence encore `srcs/system/services/services_init.rc`, remplacé
  par la composition actuelle entre `system_init.rc` et les fichiers de cible.

Ces références sont plus problématiques que les variations purement esthétiques : elles peuvent
faire exécuter une mauvaise commande ou rechercher une source de vérité inexistante.

## Hygiène du checkout et build

Le dépôt Git est propre, mais le checkout contient des fichiers ignorés hors de `build/` :

- 23 fichiers `*.orig` et un fichier `*.rej.orig` ;
- `i2c.d` à la racine ;
- `branch.sh`, `current.geany` et le répertoire `prompt/`.

Ils ne polluent pas l'historique Git et ne sont pas pris comme sources C par le build. Ils rendent
néanmoins l'arborescence locale moins lisible et peuvent perturber certains inventaires génériques.

La cible `clean` supprime effectivement les artefacts visés, mais son affichage contient les chaînes
littérales `{FILE_AUTOCODE_TARGET}` et `{PATH_BUILD_TARGET}` à cause de substitutions sans `$` dans
`mk/utils.mk`.

## Priorités recommandées

1. Corriger les chemins et commandes obsolètes de la documentation active.
2. Documenter une convention unique pour les noms de fichiers, répertoires, acronymes et matériels.
3. Corriger `test-noscli_init.rc`, les blocs `@file` et les gardes d'inclusion des cibles.
4. Étendre le contrôle de frontières aux dépendances de couches complètes.
5. Découpler l'attente coopérative et le backend matériel du noyau de `tm_libc`.
6. Isoler la composition générée de `sysCore` et sélectionner réellement les sources par cible.
7. Nettoyer les sauvegardes locales ignorées et corriger l'affichage de `bmake clean`.

## Validation effectuée

- `bmake clean && bmake` pour `test1` : `Build complete`.
- `bmake VAL_TARGET=test_noscli clean && bmake VAL_TARGET=test_noscli` : `Build complete`.
- restauration et reconstruction du profil par défaut : `Build complete`.
- contrôle des en-têtes critiques : succès pour les cinq motifs configurés.
- autoCode final sur `test1` : neuf fichiers générés inchangés.
- état Git avant export du présent rapport : propre.
- aucun essai sur matériel physique.
