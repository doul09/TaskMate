# Audit de la dépendance USART à `tm_string_t`

## Objet et périmètre

Cet audit examine la dépendance du pilote `atmega2560/usart.c` à `tm_string_t` et évalue les
architectures permettant de limiter ce pilote à des octets `uint8_t`. L'analyse correspond au
commit `774e9df`. Elle couvre le pilote USART, le backend matériel de `tm_libc`, le formatage, les
syscalls associés, le démarrage et `panic()` ; elle ne modifie pas le firmware.

## Constat

`tm_string_t` est défini dans la couche neutre `interfaces/`. Sa présence dans un contrat HAL ne
constitue donc pas formellement une dépendance du HAL vers `tm_libc`. Elle reste toutefois une
abstraction trop riche pour un périphérique série : elle décrit une chaîne terminée par NUL, sa
taille maximale et son emplacement en RAM ou en mémoire programme.

`hal_usartWriteString()` interprète actuellement ces propriétés dans le pilote MCU. Il parcourt la
chaîne jusqu'à `TM_STRING_SIZE_MAX`, appelle `hal_string_getChar()` pour distinguer RAM et ROM, puis
alimente le buffer TX. Le contrat générique `interfaces/drv_usart.h` importe donc aussi
`interfaces/tm_string.h` uniquement pour cette opération.

Une dépendance circulaire existe entre les deux unités HAL :

```text
atmega2560/usart.c  -> hal_string_getChar()  -> atmega2560/tmlibc.c
atmega2560/tmlibc.c -> hal_usartWriteChar()  -> atmega2560/usart.c
```

Le sens des responsabilités est également inversé : le transport série connaît la représentation
des textes alors que le backend texte connaît déjà le transport série. Le pilote devrait seulement
recevoir, stocker et émettre des octets.

## Contraintes AVR et chemins particuliers

- Un pointeur `const uint8_t *` ordinaire désigne la RAM et ne permet pas de lire directement une
  chaîne AVR `PROGMEM`. La conversion RAM/ROM doit rester au-dessus du pilote.
- Le buffer circulaire TX offre 63 emplacements utiles. Une écriture groupée doit définir une
  sémantique explicite en cas de remplissage partiel.
- `hal_usartSendTXBuffer()` attend activement le registre matériel. Cette opération est bloquante et
  ne doit pas être masquée dans une primitive présentée comme non bloquante.
- Le boot journalise avant le démarrage de l'ordonnanceur. Il ne peut donc pas dépendre d'une
  synchronisation fondée sur la coopération entre threads.
- `panic()` coupe les interruptions et doit conserver un chemin de sortie autonome, sans syscall ni
  verrou partagé.

## Options étudiées

### 1. Retirer seulement l'écriture de chaînes du pilote

Le contrat USART devient limité aux primitives suivantes, avec des noms définitifs à choisir selon
les conventions du projet :

```c
hal_driver_state_t hal_usartRead(uint8_t *data);
hal_driver_state_t hal_usartWriteByte(uint8_t data);
hal_driver_state_t hal_usartFlush(void);
```

`hal_usartWriteString()` disparaît de `interfaces/drv_usart.h` et de `usart.c`. Le backend texte
continue à extraire les octets RAM/ROM avant d'appeler USART. `panic.c`, seul appelant direct actuel
de l'écriture de chaîne, reçoit un petit helper privé qui parcourt `tm_string_t`, transmet chaque
octet et force l'émission.

Cette solution supprime immédiatement le cycle et ne change ni la place de `tm_libc`, ni le chemin
normal des logs. Elle conserve cependant un backend `tm_libc` directement relié au HAL USART.

### 2. Rendre toute la sortie normale dépendante de `sysCall`

Le chemin cible serait :

```text
services -> tm_libc/console -> sysCall -> HAL USART -> matériel
```

Une API telle que `sc_usartWrite(uint8_t)` et `sc_usartFlush()` préserverait la frontière déjà
utilisée pour la réception SCLI. Une capacité logique `sc_consoleWriteByte()` serait encore plus
portable si la destination future ne doit pas nécessairement être un USART.

Ce déplacement n'est pas applicable directement à l'état courant :

- `sc_hal.c` appelle `tm_syslog()` pendant le scan I2C ;
- les syscalls de recherche de pilotes et de threads appellent `tm_strncmp()` ;
- le formateur appelle `sc_coopYield()` lorsqu'il observe son verrou occupé ;
- le boot et `panic()` utilisent la sortie hors du chemin normal des tâches.

Déplacer `tm_libc` en bloc créerait donc un cycle `sysCall <-> tm_libc`. Il faudrait d'abord
retirer la journalisation de `sc_i2cScan()`, rendre les primitives de chaînes utilisables sous
`sysCall` sans dépendance remontante et isoler la politique de concurrence du formateur.

Un syscall par octet ajoute aussi des appels sur tout le flux formaté. Une écriture groupée
réduirait ce coût, mais nécessiterait soit un buffer RAM intermédiaire, soit une API plus complexe
de résultat partiel. Ce compromis doit être mesuré sur AVR avant de modifier le contrat.

### 3. Injecter un puits d'octets dans le formateur

Le formateur peut recevoir un callback et un contexte :

```c
typedef err_codes_t (*tm_byte_sink_t)(uint8_t data, void *context);
```

La même logique de formatage pourrait alors cibler un buffer RAM, une console via syscall ou une
sortie HAL d'urgence. Cette inversion rend les dépendances explicites et facilite les tests.

Elle introduit toutefois des appels indirects, un contrat supplémentaire et potentiellement un coût
flash et pile. Sur ATmega2560, elle ne doit être retenue qu'après comparaison de l'ELF et du
désassemblage avec une version à appels directs.

## Architecture recommandée

La solution recommandée combine les options 1 et 2 en deux étapes :

1. rendre immédiatement le pilote USART strictement orienté `uint8_t` en supprimant
   `hal_usartWriteString()` ;
2. séparer ensuite le noyau de chaînes et de formatage de la politique de sortie, puis faire passer
   la console normale par un syscall.

Les responsabilités finales seraient :

```text
interfaces       : descripteur portable tm_string_t et contrats de données
tm_libc core      : lecture RAM/ROM, copie, comparaison et formatage sans syscall
console normale  : politique de log et sortie par syscall
sysCall          : validation, traduction d'erreur et accès à la capacité de console
HAL USART        : réception, mise en file et émission de uint8_t
console urgence  : chemin HAL borné réservé au boot critique et à panic()
```

Le changement de statut de `tm_libc` est donc une direction pertinente pour la console, mais pas
pour l'ensemble des primitives de chaînes. Le noyau RAM/ROM reste une préoccupation transversale de
représentation ; la journalisation et le choix du périphérique de sortie appartiennent à une couche
supérieure.

Cette séparation évite également que `panic()` dépende d'un service, d'un syscall ou du verrou
global du formateur. Le chemin d'urgence doit rester minimal, synchrone, sans allocation et sans
interruption.

## Plan de mise en œuvre et validation

1. Remplacer les appels de `panic.c` par un helper local d'émission de `tm_string_t` vers des
   octets.
2. Supprimer `hal_usartWriteString()` et l'inclusion de `interfaces/tm_string.h` dans le contrat
   USART.
3. Vérifier par recherche négative que `usart.c` ne connaît plus `char`, `tm_string_t`, `TM_STR`,
   `TM_MEM_*`, `PROGMEM` ni `hal_string_getChar()`.
4. Exécuter `bmake clean && bmake` et comparer flash, RAM, symboles et profondeur de pile.
5. Valider sur Arduino Mega les chaînes RAM et ROM, le remplissage du buffer TX, les fins de ligne,
   le démarrage précoce et `panic()` avec interruptions coupées.
6. Dans une évolution séparée, déplacer la trace I2C hors de `sysCall`, découpler le formateur de
   `sc_coopYield()` et introduire la sortie console par syscall.
7. Mesurer une variante par octet et une variante groupée avant de choisir l'API console définitive.

## Verdict

`atmega2560/usart.c` doit utiliser uniquement des octets `uint8_t`. La correction immédiate la
plus sûre consiste à supprimer l'API de chaîne du pilote et à déplacer son parcours dans les
consommateurs qui comprennent `tm_string_t`.

Le chemin `tm_libc -> sysCall -> USART` est recommandé pour la console normale après découpage, mais
pas comme déplacement global immédiat de `tm_libc`. Sans cette préparation, il remplace le cycle
actuel entre USART et le backend texte par un cycle plus large entre `sysCall` et `tm_libc`, tout en
affaiblissant les chemins spéciaux de boot et de panique.
