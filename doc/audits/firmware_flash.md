# Audit de réduction de la taille flash du firmware

## Objet et périmètre

Cet audit recherche des réductions sensibles de la flash du firmware pour la cible actuelle
`test1 / arduinoMega / atmega2560 / avr8`. Il couvre les options de compilation, les services,
les diagnostics, le catalogue d'erreurs, le formateur et le protocole commun des pilotes.

L'analyse correspond au commit `400ac8b`. Aucun changement de comportement du firmware n'est
inclus dans cet audit.

## Méthode

La référence a été reconstruite avec :

```sh
bmake clean && bmake
avr-size -G -d build/test1_arduinoMega_atmega2560_avr8/TaskMate.elf
avr-nm --format=posix --print-size --size-sort -r \
	build/test1_arduinoMega_atmega2560_avr8/TaskMate.elf
```

La mesure de flash utilisée est `.text + .data`. La section `.data` occupe de la RAM à
l'exécution, mais son image initiale est aussi stockée en flash.

Les variantes de compilation ont été reconstruites intégralement avec les mêmes sources. Les
variantes fonctionnelles ont été reliées avec des substituts minimaux afin de laisser LTO et
`--gc-sections` éliminer les chemins devenus inaccessibles. Ces liens mesurent la contribution
des fonctionnalités, mais ne sont pas des firmwares exécutables validés.

## Référence

Le build propre se termine par `Build complete` et produit :

| Section | Taille |
|---|---:|
| `.text` | 15 678 octets |
| `.data` | 454 octets |
| Flash totale | 16 132 octets |
| `.bss` | 1 364 octets |

Les principaux symboles de code sont :

| Symbole | Taille |
|---|---:|
| `main` | 984 octets |
| `system` | 642 octets |
| `scli` | 542 octets |
| `tm_vsnprintf` | 538 octets |
| `sc_i2cScan` | 528 octets |
| `hal_lcdControl` | 404 octets |
| `sc_rtcRead` | 372 octets |

Le projet active déjà `-Os`, `-mrelax`, `-fshort-enums`, LTO, les sections séparées et
`--gc-sections`. Une simple activation de LTO ou du ramasse-miettes de sections n'est donc pas
une piste nouvelle.

## Résultats mesurés

| Variante | Flash | Gain | Nature de la mesure |
|---|---:|---:|---|
| Référence | 16 132 octets | - | build complet |
| `-mcall-prologues` | 15 480 octets | 652 octets, 4,04 % | build complet |
| Deux options AVR | 15 398 octets | 734 octets, 4,55 % | build complet |
| Textes du catalogue d'erreurs absents | 15 196 octets | 936 octets, 5,80 % | lien expérimental |
| Logs retirés | 11 150 octets | 4 982 octets, 30,88 % | lien expérimental |
| Chaîne SCLI retirée | 8 990 octets | 7 142 octets, 44,27 % | lien expérimental |

`-Oz` ne modifie pas le résultat obtenu avec cet AVR-GCC 14.2.0. `-maccumulate-args` et
`-fno-jump-tables` augmentent la taille et doivent être écartés.

Les gains ne sont pas additifs : LTO transforme et mutualise le programme entier après chaque
variation.

## Priorité 1 - profils développement et production

La réduction la plus importante consiste à ne pas embarquer les fonctions de diagnostic dans un
firmware qui ne les utilise pas. Deux profils sont recommandés :

- `development` : SCLI, commandes, textes complets et logs actuels ;
- `production` : SCLI absente, logs ordinaires supprimés à la compilation, codes numériques et
  chemin `panic()` conservés.

SCLI est enregistrée dans `srcs/system/services/services_init.rc`. Le profil doit sélectionner
l'entrée autoCode appropriée afin que la base des threads, les includes générés et le graphe
d'appel restent cohérents. Exclure seulement `scli.c` du Makefile laisserait une référence générée
invalide et contournerait la source de vérité.

Le retrait expérimental de toute la chaîne SCLI économise 7 142 octets. Il élimine les commandes,
leurs textes, leurs tables, les syscalls exclusivement diagnostiques et le catalogue textuel qui
n'est alors plus référencé.

La contribution marginale de chaque famille de commandes a aussi été mesurée :

| Commande absente | Gain flash |
|---|---:|
| `date` | 1 970 octets |
| `driver` | 1 650 octets |
| `thread` | 1 472 octets |
| `i2c` | 598 octets |

Ces valeurs incluent ce que LTO peut supprimer avec la commande. La somme diffère du retrait total
de SCLI à cause des fonctions partagées, de son lecteur USART, de son tokenizer et de son dispatch.

Si SCLI doit rester disponible, un profil intermédiaire peut sélectionner seulement les commandes
nécessaires. La commande `date`, puis les commandes `driver` et `thread`, sont les premières à
retirer d'une image compacte.

## Priorité 2 - options de compilation AVR

Ajouter `-mcall-prologues` aux options de l'architecture économise 652 octets sans variation de
`.data` ou `.bss`. AVR-GCC remplace certains prologues et épilogues répétés par des routines
partagées.

L'ajout de `-fno-inline-functions-called-once` porte le gain à 734 octets. Cette option matérialise
davantage de fonctions et réduit plusieurs gros corps issus de LTO, notamment `main`, `system` et
`tm_vsnprintf`.

Le premier drapeau est le changement le moins intrusif. Le second doit être accepté seulement après
mesure des cycles et de la profondeur de pile sur les chemins sensibles. Les ISR nues et le
changement de contexte doivent être contrôlés dans le désassemblage même si le compilateur ne doit
pas leur ajouter de prologue ordinaire.

## Priorité 3 - politique de logs

Les littéraux créés par `TM_STR()` représentent 1 600 octets de flash dans le binaire de référence,
hors messages du catalogue d'erreurs. Les textes sont déjà placés correctement en mémoire programme
par le HAL AVR ; les déplacer vers `PROGMEM` une seconde fois ne réduirait pas la flash.

Un substitut de `tm_syslog()` sans effet laisse LTO éliminer 4 982 octets. Le gain dépasse la taille
des chaînes parce que disparaissent aussi les appels, la préparation des arguments, la consultation
des erreurs et plusieurs branches de présentation.

Une vraie politique doit utiliser des macros de niveau qui retirent l'expression complète au
préprocesseur. Appeler une fonction vide sans LTO, ou continuer à évaluer ses arguments, ne garantit
pas ce résultat. Les drivers doivent continuer à exposer leurs états et erreurs plutôt que produire
directement des messages.

Le profil production peut conserver une sortie minimale et bornée pour le démarrage critique et
`panic()`, séparée des logs ordinaires et sans dépendre de SCLI.

## Priorité 4 - catalogue d'erreurs compact

Le catalogue actuel représente directement :

| Élément | Taille flash |
|---|---:|
| 18 textes | 517 octets |
| 18 descripteurs `tm_string_t` | 54 octets |
| table de 23 entrées | 69 octets |
| Total des données identifiées | 640 octets |

Le lien expérimental sans textes économise 936 octets une fois les fonctions et branches devenues
inutiles éliminées. Les codes `err_codes_t` doivent rester stables ; seule leur représentation
humaine doit devenir optionnelle.

Cette évolution appartient aux fichiers `*.err` et à autoCode. Les régions générées de
`srcs/system/sysCall/error.c` et `srcs/interfaces/error_catalog.h` ne doivent pas être modifiées
manuellement. Les messages provisoires `ERR_UNKNOWN` et `ERR_RUNTIME` sont les premiers candidats à
supprimer indépendamment du profil.

## Priorité 5 - contrôleurs de pilotes

Les six fonctions `hal_*Control()` occupent ensemble 1 752 octets :

| Contrôleur | Taille |
|---|---:|
| LCD | 404 octets |
| timer STC | 312 octets |
| I2C | 304 octets |
| RTC | 282 octets |
| timer d'ordonnancement | 234 octets |
| USART | 216 octets |

Elles répètent les traitements `RLSET`, `RLGET`, `SETBIT`, `CLEARBIT`, `GETBIT`, `GETSTATUS` et
`GETLASTERROR`. L'audit `doc/audits/driver_control_factorisation.md` recommande déjà un helper HAL
compilé une seule fois, sans descripteur persistant. Son gain estimé de 350 à 650 octets reste
plausible au vu des 1 752 octets maintenant mesurés.

Cette factorisation doit préserver l'ordre des erreurs, la dépendance I2C de LCD et RTC, et le
caractère `volatile` de la dernière erreur USART. Un prototype binaire est indispensable : des
callbacks indirects ou une ré-inlining LTO peuvent réduire le bénéfice.

## Priorité 6 - formateur compact

`tm_vsnprintf()` occupe 538 octets. Le firmware utilise `%i`, `%s`, `%x` et le remplissage par zéro,
mais aucun appel actuel n'utilise `%c`, `%b` ou `%%`. La conversion générique conserve pourtant les
trois bases et effectue division et modulo sur 16 bits.

Un profil de formateur limité aux conversions réellement requises peut retirer les branches mortes
que le compilateur ne peut pas déduire depuis une chaîne interprétée à l'exécution. Le gain attendu
est secondaire face à SCLI et aux logs. Il doit être mesuré avant de réduire le contrat public du
formateur.

## Fausses pistes

- Déplacer les tables de commandes `const` en mémoire programme économiserait surtout de la RAM :
  leurs octets restent présents dans l'image flash et les accès AVR deviennent plus coûteux.
- Supprimer les sections DWARF de l'ELF réduit la taille du fichier de débogage, pas celle de
  l'image programmée.
- Retirer le scan I2C de démarrage changerait la découverte et la réconciliation des pilotes ; ce
  n'est pas une optimisation neutre.
- Réduire les piles de threads ne diminue pas la flash, car elles résident dans `.bss`.
- Remplacer le formateur TaskMate par le `printf` standard risque au contraire d'augmenter fortement
  la taille et d'affaiblir les bornes adaptées au microcontrôleur.

## Ordre de mise en œuvre recommandé

1. Ajouter et mesurer `-mcall-prologues` dans le backend AVR.
2. Introduire un profil production piloté par les entrées autoCode, sans SCLI.
3. Ajouter des niveaux de logs éliminés à la compilation et un catalogue sans textes.
4. Décider commande par commande du contenu d'un éventuel profil SCLI compact.
5. Prototyper la factorisation des contrôleurs et conserver seulement un gain mesuré.
6. Spécialiser le formateur en dernier.

Après chaque étape, exécuter `bmake autoCode_alone` si ses entrées changent, puis
`bmake clean && bmake`, comparer `.text`, `.data`, `.bss`, le fichier map et les symboles du même
ELF. Les changements de prologue, de factorisation et de formatage doivent aussi être vérifiés dans
le désassemblage. Enfin, le build AVR valide l'intégration logicielle mais pas le comportement sur
Arduino Mega, les interruptions, les délais matériels ou le bus I2C réel.

## Verdict

Le meilleur gain sans retrait fonctionnel identifié est de 652 octets avec `-mcall-prologues`, ou
734 octets avec une politique d'inlining plus restrictive.

Pour une réduction réellement sensible, un profil production est nécessaire. Retirer SCLI permet à
LTO d'éliminer environ 7,1 Kio, soit 44 % de l'image actuelle. Les logs et textes d'erreurs doivent
ensuite être traités comme des fonctionnalités de diagnostic configurables, tout en conservant les
codes, les états, `panic()` et les frontières `services -> sysCall -> HAL`.
