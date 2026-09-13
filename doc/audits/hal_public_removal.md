# Audit de suppression de `hal/public`

Date : 13 septembre 2026

Branche auditée : `codex/remove-hal-public`

Révision de base : `04f0603`

## Objet, périmètre et verdict

Cet audit décrit la feuille de route permettant de supprimer `srcs/hal/public/` au profit de
contrats neutres dans `srcs/interfaces/` et d'une sélection explicite des implémentations au moment
du build. Il couvre les sept en-têtes restants de la façade, leurs consommateurs, autoCode et
les fragments BSD `bmake`. Il ne modifie pas le firmware et ne considère pas les sections
inutilisées par le linker comme une sélection de sources.

La suppression est **faisable sous conditions**, mais ce n'est pas un déplacement mécanique
d'en-têtes. Les contrats de pilotes sont déjà correctement placés dans `interfaces/`. Les éléments
restants mélangent encore trois responsabilités : contrats portables, types propres à l'AVR8 et
sélection d'implémentations par préprocesseur.

L'ordre recommandé est le suivant :

1. rendre la liste des unités de traduction matérielles explicite dans les fragments Make ;
2. extraire les contrats réellement portables vers `interfaces/` ;
3. réduire les primitives de contexte à une opération HAL indivisible et garder l'assembleur privé ;
4. traiter séparément la représentation de pile, le GPIO physique et les chaînes en mémoire flash ;
5. supprimer la génération `hal_define`, les relais et leurs règles d'accès.

Le résultat cible doit suivre ce chemin :

```text
code portable -> interfaces/<capacité>.h -> symbole HAL
                                            ^
                                            |
                         fichier .c/.S choisi par target/board/MCU/arch.mk
```

`interfaces/` décrit le contrat, mais ne choisit ni l'architecture ni le périphérique et n'inclut
aucun en-tête sous `hal/`.

## État actuel de la façade

`hal/public` contient sept en-têtes et possède onze consommateurs directs dans les sources des
deux profils. Six en-têtes choisissent une implémentation avec `ARCH_avr8` ou `MCU_atmega2560`. Le
septième, `define.h`, est une agrégation générée des définitions de la cible, de la carte, du MCU et
de l'architecture.

| En-tête public | Contenu sélectionné | Consommateurs ou rôle | Difficulté principale |
| --- | --- | --- | --- |
| `atomic.h` | `arch/avr8/atomic.h` | `sc_hal.c`, `sc_modules.c` | état d'interruption et coût d'appel |
| `context.h` | `arch/avr8/context.h` | allocation et démarrage des threads | ABI de pile et assembleur inline |
| `define.h` | quatre `define.h` générés | type de pile dans `modules.h` | mélange de types arch, MCU et réglages |
| `gpio.h` | `mcu/atmega2560/gpio.h` | sysCore et câblage des deux cibles | descripteur physique propre au MCU |
| `interrupt.h` | `arch/avr8/interrupt.h` | démarrage de l'ordonnanceur | `sei`, `cli` et `reti` inline |
| `stack.h` | `arch/avr8/stack.h` | démarrage de l'ordonnanceur | changement de `SP` non appelable seul |
| `tmlibc.h` | `mcu/atmega2560/tmlibc.h` | formatage, chaînes et LCD | macros `PROGMEM` et cycle avec USART |

La façade ne contient aucune implémentation `.c`. Son retrait ne sélectionnera donc rien à lui
seul : il faut simultanément rendre le build responsable du choix de l'unique implémentation de
chaque capacité.

## État actuel de la sélection au build

La chaîne `target -> board -> MCU -> architecture` est déjà explicite : les fragments sélectionnés
construisent `VAL_HW_STACK`, ajoutent leurs répertoires à `PATHS_SOURCES` et contribuent aux listes
d'initialisation. Les deux profils actuels aboutissent à :

```text
test1|test_noscli -> arduinoMega -> atmega2560 -> avr8
```

Cette base est utile, mais `mk/sources.mk` transforme ensuite tous les répertoires enregistrés en
sources par un `find` récursif. En outre, `mk/path_files.mk` ajoute globalement `system`,
`tm_libc`, `hal/public`, tous les pilotes externes et toutes les tâches.

Conséquences vérifiées avec `bmake -V FILES_SRC` :

- `test_noscli` compile toujours `scli.c` et ses quatre commandes ;
- les deux cibles compilent systématiquement LCD AMC2004 et RTC ZS042 ;
- tous les fichiers `.c` du MCU et de l'architecture sélectionnés sont compilés, qu'une capacité
  soit demandée ou non ;
- les fichiers `.err` et `.rc` suivent aussi la découverte par répertoire ;
- l'élimination finale par `--gc-sections` masque une partie du problème sans contrôler la
  compatibilité des unités compilées avec la cible.

Le build choisit donc aujourd'hui une **arborescence**, pas une liste exacte d'implémentations.

## Points déjà prêts

- Le contrat `hal_halt()` est dans `interfaces/` et son implémentation est sélectionnée avec AVR8.
- Les six contrats de pilotes sont dans `interfaces/drv_*.h` et leurs implémentations les incluent
  directement.
- autoCode génère les inclusions de pilotes depuis ces contrats neutres.
- Les fragments de cible, carte, MCU et architecture existent déjà et sont inclus dans un ordre
  déterministe.
- `FILES_HALINIT_HEADER` et `VAL_FUNCINIT` décrivent déjà la composition d'initialisation sans
  utiliser les relais de `hal/public`.
- Le contrôle d'architecture autorise `sysCore` et `sysCall` à consommer HAL et `interfaces`, tout
  en interdisant une dépendance inverse.

Ces éléments permettent une migration incrémentale sans introduire de dispatch dynamique, de table
de fonctions en RAM ni de condition matérielle dans le code portable.

## Bloqueurs à résoudre avant suppression

### 1. Démarrage et restauration de contexte

`tm_schedulerStart()` enchaîne actuellement le chargement de `SP`, la restauration des registres,
l'activation des interruptions et `reti`. Ces opérations sont des fonctions `always_inline`
séparées, mais leur séquence forme en pratique une seule transition de contexte. Les transformer en
fonctions C externes indépendantes déposerait des adresses de retour sur une pile en cours de
remplacement et autoriserait des prologues ou épilogues incompatibles.

Le contrat portable doit être réduit à une opération de démarrage qui ne retourne pas. Son
implémentation AVR8, sélectionnée au build, doit posséder toute la séquence et être vérifiée au
désassemblage. Les macros `AVR8_CONTEXT_SAVE` et `AVR8_CONTEXT_RESTORE` restent privées à
l'architecture et au pilote de timer d'ordonnancement ; elles ne doivent pas migrer dans
`interfaces/`.

### 2. Représentation de la pile et état atomique

`hal_stack_word_t` et `hal_atomic_state_t` sont actuellement des `uint8_t` définis par AVR8.
Le premier dimensionne chaque pile statique dans `mod_thread_item_t`, et le second mémorise `SREG`
pour restaurer exactement l'état antérieur des interruptions.

Une définition neutre doit être choisie avant de supprimer `hal/public/define.h`. Utiliser
aveuglément `uintptr_t` doublerait la RAM occupée par les piles sur AVR8. Utiliser toujours
`uint8_t` imposerait une unité et un alignement qui ne sont pas encore validés pour les CPU 16/32
bits. Cette décision exige un prototype avec contrôle de `sizeof`, alignement, `.data`, `.bss`,
pile et code généré. Le contrat atomique doit conserver la restauration imbriquée de l'état ; une
simple paire globale `disable/enable` n'est pas équivalente.

### 3. Frontière GPIO

`hal_pin_t`, `hal_port_list_t`, `hal_port_t` et `hal_signal_t` exposent actuellement dans sysCore
des détails de port propres à l'ATmega2560. Les déplacer tels quels vers `interfaces/` violerait la
règle qui réserve cette couche aux concepts valables sur toutes les plates-formes.

La cible recommandée est une API fondée sur `gpio_signal_t` : le code portable demande
l'initialisation, la lecture ou l'écriture d'un signal logique ; le fichier cible sélectionné au
build possède la table logique-vers-physique et les types MCU privés. Ainsi sysCore ne stocke plus
de descripteur matériel et `targetWireSignal()` n'est plus exposé par un en-tête généré.

### 4. Backend des chaînes

`hal/public/tmlibc.h` ne contient pas seulement deux prototypes : il définit `TM_STR*` avec
`PSTR` et `PROGMEM`. Cette représentation économise la RAM AVR et ne peut pas être remplacée par des
littéraux C ordinaires sans mesure. Le backend contient encore le cycle déjà identifié entre lecture
de chaîne et USART. Le remplacement de `panic()` par `hal_halt()` a déjà retiré de ce chemin les
chaînes, le formatage et la sortie USART.

Il faut séparer la représentation portable de `tm_string_t` de l'accès sélectionné à la mémoire
programme. Déplacer les macros AVR dans `interfaces/` serait une fausse suppression de la dépendance
matérielle. La solution retenue devra conserver les chaînes en flash, éviter l'allocation et
supprimer le cycle USART/backend.

## Feuille de route proposée

### Étape 0 — Établir la référence mesurable

1. Construire proprement `test1` et `test_noscli` et conserver ELF, map, `avr-size` et
   désassemblage des chemins de contexte.
2. Relever la taille de `mod_thread_item_t`, de chaque pile, de l'état atomique et la profondeur de
   pile des appels concernés.
3. Valider sur Arduino Mega le boot, la préemption, le yield coopératif, les GPIO, USART, LCD, RTC
   et la panique avant toute modification structurelle.

Cette référence fixe les comportements et coûts à préserver ; le build réussi seul ne valide pas
une transition de contexte.

### Étape 1 — Sélectionner explicitement les unités de traduction

1. Faire contribuer chaque fragment sélectionné à des listes dédiées, par exemple sources de cible,
   carte, MCU, architecture et pilotes externes.
2. Déclarer les fichiers exacts dans ces listes au lieu d'ajouter tout leur répertoire à la liste
   compilée.
3. Faire choisir LCD AMC2004 et RTC ZS042 par `target.mk`, et non par l'ajout global de
   `hal/drivers`.
4. Aligner les listes `.err`, `.rc`, les dépendances autoCode, les objets et le nettoyage sur la
   même composition.
5. Ajouter des erreurs Make précoces pour une capacité sans implémentation, une implémentation en
   double, un fichier absent et une composition incohérente.
6. Conserver éventuellement des listes de répertoires pour la documentation ou l'analyse, mais ne
   plus les utiliser comme source de vérité de `FILES_SRC`.

Cette étape doit précéder la suppression des relais : elle rend le lien symbole-vers-implémentation
observable et empêche qu'un deuxième backend soit compilé par accident.

### Étape 2 — Introduire les contrats neutres simples

1. Prendre le contrat `hal_halt()` comme modèle pour introduire le contrat atomique, puis migrer
   ses consommateurs sans inclure de fichier concret.
2. Déplacer uniquement prototypes, types portables et sémantique documentée ; laisser registres,
   attributs AVR, assembleur et tables physiques dans HAL.
3. Faire compiler exactement une implémentation par contrat via les listes de l'étape 1.
4. Mesurer les fonctions atomiques hors ligne et avec LTO : elles ne doivent ni rouvrir une fenêtre
   d'interruption ni augmenter la pile de façon non maîtrisée.

`hal_halt()` remplace désormais `panic()` sans chaîne, journalisation ni dépendance USART.

### Étape 3 — Fermer les trois contrats structurants

1. Remplacer les quatre primitives publiques de démarrage (`SP`, restore, enable, `reti`) par une
   seule opération HAL `noreturn`, implémentée dans le backend d'architecture sélectionné.
2. Choisir et tester une représentation de pile neutre sans doubler la RAM AVR ; adapter ensuite
   l'allocation générée et `hal_threadContextInit()`.
3. Déplacer le câblage GPIO physique dans le fichier de cible sélectionné et limiter le contrat
   partagé aux signaux logiques.
4. Découpler le cœur de chaînes, l'accès RAM/ROM et USART avant de retirer le dernier relais
   `tmlibc.h`; une future panique enrichie reste hors de cette migration.
5. Garder le contexte d'ISR et les registres volatils dans les sources privées AVR8/ATmega2560.

Cette étape est le principal point de décision. Elle doit être découpée en changements indépendants
et mesurés, pas livrée comme une suppression globale de répertoire.

### Étape 4 — Retirer l'agrégateur généré et autoCode associé

Quand `modules.h` ne dépend plus de `hal_stack_word_t` issu du header concret :

1. supprimer `srcs/hal/public/define.h` de `FILES_PARSE_TAG` ;
2. retirer le tag `hal_define`, l'option `--haldefine` et leur traitement dans autoCode ;
3. supprimer `FILES_HALDEFINE`, ses listes, stamps et dépendances dans les fragments Make ;
4. adapter le corpus `test_autoCode` qui vérifie aujourd'hui ce tag et cette option ;
5. conserver séparément `hal_init` et `hal_fxinit`, qui décrivent l'ordre d'initialisation et ne
   dépendent pas de la façade publique.

Les quatre `define.h` ne doivent pas être déplacés en bloc dans `interfaces/` : les deux fichiers
vides peuvent disparaître, les constantes MCU restent privées et les types réellement partagés
suivent les décisions des étapes 2 et 3.

### Étape 5 — Supprimer la façade et durcir les frontières

1. Migrer les derniers consommateurs, puis supprimer les sept fichiers restants et le répertoire
   `srcs/hal/public`.
2. Retirer ce chemin de `PATHS_SOURCES`, `conf/header_allow.conf` et de toute documentation.
3. Remplacer les autorisations fondées sur les anciens chemins par des gardes attachées aux nouveaux
   contrats critiques.
4. Étendre le contrôle pour interdire tout include concret `hal/arch`, `hal/mcu` ou `hal/board`
   depuis `system`, `tm_libc` et `interfaces`; les sources de cible et HAL restent les seuls
   propriétaires de ces includes.
5. Ajouter des recherches négatives explicites sur `hal/public`, `HAL_PUBLIC_`, `hal_define` et
   `--haldefine`.

## Critères d'acceptation

La migration est terminée seulement si les conditions suivantes sont toutes satisfaites :

- `srcs/hal/public` n'existe plus et aucune référence textuelle active ne subsiste ;
- `interfaces/` n'inclut aucun fichier HAL et ne contient ni registre, ni nom AVR, ni sélection
  `ARCH_*`/`MCU_*` ;
- `bmake -V FILES_SRC` montre uniquement les implémentations demandées pour chaque cible ;
- `test_noscli` ne compile plus SCLI ni ses commandes ;
- chaque capacité possède exactement une implémentation sélectionnée, avec diagnostic précoce si
  elle manque ;
- `hal_define` et `--haldefine` ont disparu du générateur, du build et des tests ;
- `bmake autoCode_alone`, `bmake test_autoCode` et `bmake test_autoCode_sanitize` réussissent avec
  des sorties générées stables ;
- `bmake clean && bmake` et le build propre `VAL_TARGET=test_noscli` se terminent par
  `Build complete` sans dépendance ou objet obsolète ;
- flash, RAM statique et pile restent dans les seuils fixés à l'étape 0 ;
- le désassemblage confirme la séquence de contexte et l'absence de prologue/épilogue parasite ;
- les essais Arduino Mega confirment préemption, atomiques, GPIO, USART, timers, LCD/RTC, boot et
  halt terminal.

## Risques et séquencement des livraisons

Le risque fonctionnel le plus élevé est le contexte, suivi des sections atomiques et de la mémoire
programme. La sélection Make et l'arrêt terminal minimal sont plus faciles à isoler. L'ordre de publication
recommandé est donc : sélection explicite des sources, contrats simples, contexte/pile, GPIO,
chaînes, retrait autoCode, puis suppression finale.

Chaque livraison doit conserver un firmware constructible. Aucun changement ne doit introduire de
dispatch à l'exécution, d'allocation dynamique, de table de callbacks en RAM ou de `#if` matériel
dans le code portable. Faute de seconde architecture implémentée, la portabilité finale restera une
propriété de structure et de contrats jusqu'à validation sur une autre pile matérielle.

## Validation effectuée pour cet audit

- lecture des sept en-têtes restants de `hal/public` et de leurs implémentations sélectionnées ;
- recherche des onze consommateurs directs et des inclusions HAL concrètes ;
- inspection de `target.mk`, des fragments carte/MCU/architecture, de `sources.mk`,
  `path_files.mk`, `autoCode.mk` et des règles d'accès ;
- inspection des contrats de pilotes, du GPIO, de la pile, des atomiques, du contexte et du backend
  de chaînes ;
- évaluation statique de `VAL_HW_STACK`, `PATHS_SOURCES`, `FILES_SRC`, des listes d'initialisation
  et de définition avec BSD `bmake` pour `test1` et `test_noscli`.

Aucun code n'a été modifié, aucun autoCode n'a été régénéré, aucun firmware n'a été construit et
aucun essai matériel n'a été réalisé pendant cet audit.
