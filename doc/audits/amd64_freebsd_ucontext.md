# Audit de faisabilité — portage amd64/FreeBSD avec `ucontext`

Date de l'audit : 2026-09-08
Révision analysée : `1d10828` (`codex/scheduler-context`)

## Conclusion

Le portage est techniquement faisable pour construire une cible de simulation fonctionnelle de
TaskMate sous FreeBSD/amd64. Le verdict est **GO conditionnel** : le modèle de contexte récemment
introduit fournit une bonne frontière d'abstraction, mais trois contrats doivent encore être
renforcés avant d'ajouter le backend hôte :

1. fournir au HAL la base et la taille de chaque pile ;
2. encapsuler entièrement le démarrage du premier contexte dans le HAL ;
3. remplacer la copie directe de `hal_context_t` par une opération HAL compatible avec les
   particularités de `ucontext_t`.

La cible pourra valider le scheduler, les niveaux d'exécution, les compteurs logiciels, les tâches
et des périphériques virtuels. Elle ne pourra pas valider fidèlement les coûts mémoire, les cycles,
les priorités d'interruption ni les contraintes électriques de l'AVR.

## Points favorables

- [`mod_thread_item_t`](../../srcs/system/sysCore/modules.h#L33) contient désormais un
  `hal_context_t` et ne connaît plus la représentation du pointeur de pile AVR.
- Le callback du scheduler reçoit et retourne un `hal_context_t *` dans
  [`tm_scheduler.c`](../../srcs/system/sysCore/tm_scheduler.c#L101). La politique est donc déjà
  découplée de la représentation matérielle du contexte.
- FreeBSD fournit les fonctions `getcontext`, `makecontext`, `setcontext` et `swapcontext` dans sa
  libc. `makecontext` travaille sur un contexte initialisé par `getcontext`, avec une pile et un
  lien de retour, ce qui convient au point d'entrée `void(void)` des tâches TaskMate.
- Le retrait de `ucontext` de POSIX.1-2008 impose toutefois d'assumer explicitement une cible
  FreeBSD, et non un backend hôte POSIX générique.

## Écarts à traiter

| Sujet | AVR8 actuel | Besoin FreeBSD/amd64 |
|---|---|---|
| Représentation du contexte | pointeur de pile dans `hal_context_t` | `ucontext_t` et état associé |
| Initialisation de pile | adresse du sommet uniquement | base et taille de la zone |
| Dimensionnement | 256 mots de pile fixes | tailles propres au target, généralement en dizaines de Kio |
| Premier démarrage | restauration AVR, activation IRQ, `reti` | primitive HAL, typiquement `setcontext` |
| Sauvegarde du contexte | affectation directe de la structure | opération HAL contrôlant la copie |
| Préemption | ISR nue du Timer1 | signal périodique avec `SA_SIGINFO` |
| Atomicité | sauvegarde de `SREG` puis `cli` | blocage et restauration des signaux d'horloge |
| Compteur logiciel | Timer3 séparé | second timer ou tick dérivé |
| Périphériques | registres MCU | GPIO, USART et I2C virtuels |
| Construction | compilateur et édition de liens AVR | backend natif amd64 dans le pipeline `bmake` |

## 1. Contrat de pile

Les tâches générées possèdent actuellement un tableau `hal_stack_word_t` de taille fixe
`TM_MOD_THREAD_STACK_SIZE`, soit 256 éléments dans
[`tm_modules.h`](../../srcs/interfaces/tm_modules.h#L63). autoCode transmet seulement l'adresse du
dernier élément à l'initialisation du contexte dans
[`parseTag.c`](../../srcs/autoCode/parseTag.c#L414).

Un backend `makecontext` doit connaître la base et la taille en octets de la pile. Le contrat cible
devrait donc évoluer vers une forme équivalente à :

```c
hal_threadContextInit(thread_function, context, stack_base, stack_size);
```

La taille de pile doit être configurable par target. Il ne faut pas augmenter artificiellement la
taille de `hal_stack_word_t`, car cela pénaliserait directement la RAM AVR.

## 2. Démarrage du premier contexte

[`tm_schedulerStart()`](../../srcs/system/sysCore/tm_scheduler.c#L62) enchaîne encore
`hal_contextRestore`, `hal_setGlobalInterrupt` et `hal_returnFromInterrupt`. Cette séquence expose
au noyau la méthode de démarrage propre à l'AVR.

Une primitive `hal_contextStart()` devrait porter toute la transition. Le backend AVR y conserverait
la restauration, l'activation des interruptions et le retour d'interruption ; le backend amd64
pourrait appeler `setcontext`.

## 3. Copie de `ucontext_t`

Le scheduler copie actuellement le contexte par affectation. Sur FreeBSD/amd64, le contexte machine
contient notamment des champs décrivant un état FPU étendu indirect. Une simple affectation de
`ucontext_t` ne doit donc pas devenir un contrat portable sans validation spécifique ou stratégie de
copie profonde.

Le scheduler devrait demander au HAL de sauvegarder ou copier le contexte. Le backend AVR garderait
une opération triviale ; le backend FreeBSD maîtriserait la durée de vie et les éventuels pointeurs
internes.

## Préemption sous FreeBSD

Une simulation uniquement coopérative ne suffit pas : certaines tâches attendent l'évolution des
compteurs logiciels dans des boucles actives sans céder volontairement le processeur.

Le mécanisme envisageable est le suivant :

1. armer un timer monotone périodique pour le scheduler ;
2. installer un gestionnaire avec `SA_SIGINFO` ;
3. recevoir le `ucontext_t` du code interrompu ;
4. le sauvegarder via le HAL ;
5. appeler la politique d'ordonnancement portable ;
6. faire restaurer le contexte sélectionné par le retour de signal FreeBSD.

Avec `SA_SIGINFO`, FreeBSD transmet au gestionnaire un pointeur vers le contexte interrompu.
`sigreturn` restaure de manière atomique le masque de signaux, la pile, les registres et le compteur
ordinal. Cette dépendance doit rester strictement confinée au backend FreeBSD.

Il est déconseillé d'appeler directement `swapcontext` depuis le gestionnaire : cette fonction ne
figure pas dans la liste des fonctions garanties sûres dans un contexte de signal asynchrone.

### Sections atomiques et yield coopératif

[`sc_coopYield()`](../../srcs/system/sysCall/sc_modules.c#L167) peut conserver son rôle, en envoyant
ou déclenchant le même événement de planification que le timer.

L'équivalent hôte d'une section atomique doit bloquer les signaux du scheduler et du compteur
logiciel, puis restaurer exactement le masque précédent. Si un tick survient pendant la section, le
signal reste pendant et est traité après la restauration du masque.

## Gestion du temps

La cible AVR utilise actuellement un tick scheduler de 1 ms et un tick STC de 10 ms, notamment dans
[`timerSTC.c`](../../srcs/hal/mcu/atmega2560/timerSTC.c#L93).

Le backend hôte devrait utiliser des timers POSIX basés sur `CLOCK_MONOTONIC`, avec deux signaux
distincts, plutôt que `setitimer`. FreeBSD ne met en attente qu'une occurrence d'un même signal de
timer ; les expirations supplémentaires doivent donc être comptabilisées pour éviter une dérive des
STC en cas de retard du processus.

Cette horloge restera une approximation fonctionnelle : la latence de l'ordonnanceur hôte, les
interruptions de syscalls et les dépassements de quantum ne reproduiront pas le timing cycle par
cycle du microcontrôleur.

## Organisation proposée du target

```text
srcs/user/target/simFreeBSD/
srcs/hal/board/hostSim/
srcs/hal/mcu/freebsd/
srcs/hal/arch/amd64/
```

Le nom `mcu/freebsd` est imparfait sur le plan matériel, mais conserve la sélection existante : cette
couche fournirait les mécanismes et périphériques exposés par le système hôte. Il faudrait :

- déclarer le target dans `hardware-targets.conf` ;
- ajouter les sélecteurs `ARCH_amd64` et `MCU_freebsd` ;
- fournir les règles d'architecture et la chaîne de compilation native ;
- conserver autoCode, les contrôles de frontières et l'orchestration `bmake` ;
- produire un exécutable natif et un rapport mémoire adapté à l'hôte.

La sélection actuelle se trouve dans
[`hardware_target.mk`](../../mk/hardware_target.mk#L15), tandis que les options du compilateur AVR
sont isolées dans [`archCC.mk`](../../srcs/hal/arch/avr8/archCC.mk#L29).

## Périphériques simulés

L'USART est utilisée très tôt pendant le démarrage dans
[`TaskMate.c`](../../srcs/system/TaskMate.c#L43). Le service système initialise aussi I2C, RTC et LCD
dans [`system.c`](../../srcs/system/services/system.c#L50).

Pour exécuter le firmware applicatif existant sans court-circuiter les couches :

- conserver les vrais drivers logiques LCD et RTC ;
- fournir un bus I2C virtuel avec un LCD à l'adresse `0x3C` et une RTC à `0x68` ;
- implémenter l'USART par un terminal non bloquant ;
- représenter les GPIO comme des booléens observables et traçables.

Pour `tm_libc`, les chaînes ROM et RAM peuvent partager la même représentation sur amd64, avec une
sortie caractère redirigée vers le terminal. Le backend AVR doit conserver son traitement `PROGMEM`,
actuellement défini dans [`tmlibc.h`](../../srcs/hal/mcu/atmega2560/tmlibc.h#L34).

## Plan d'implémentation recommandé

1. Renforcer les contrats portables : pile base/taille, démarrage et sauvegarde de contexte HAL.
2. Ajouter un backend amd64 minimal avec ordonnancement coopératif et test des changements de
   contexte.
3. Ajouter la préemption FreeBSD, les masques de signaux et le traitement des dépassements de timer.
4. Ajouter les périphériques virtuels nécessaires au démarrage complet.
5. Valider la stabilité d'autoCode sur les deux cibles, les builds propres, une séquence déterministe,
   la préemption sous charge, les boucles coopératives, les niveaux d'exécution, la dérive des timers
   et la non-régression AVR.

## Limites définitives de la simulation

La cible serait pertinente pour tester :

- la politique du scheduler et les niveaux d'exécution ;
- les services, syscalls et tâches ;
- les scénarios SCLI ;
- les états GPIO et l'injection de transactions I2C.

Elle ne validerait pas :

- la sauvegarde exacte des registres AVR ;
- les contraintes des 8 Kio de SRAM et de la mémoire flash ;
- la séparation Harvard et les accès `PROGMEM` ;
- les priorités, latences et imbrications réelles des interruptions ;
- les registres volatils et le comportement électrique ;
- le temps réel matériel.

## Sources FreeBSD

- [Manuel `getcontext`, `makecontext`, `setcontext` et `swapcontext`](https://man.freebsd.org/cgi/man.cgi?query=swapcontext)
- [Manuel `sigaction(2)` et contrat `SA_SIGINFO`](https://man.freebsd.org/cgi/man.cgi?query=sigaction&sektion=2&manpath=FreeBSD+14.3-RELEASE)
- [Manuel `sigreturn(2)`](https://man.freebsd.org/cgi/man.cgi?query=sigreturn&sektion=2)
- [Définition amd64 de `ucontext` dans les sources FreeBSD](https://cgit.freebsd.org/src/plain/sys/x86/include/ucontext.h)
- [Manuel `timer_settime(2)` et dépassements de timer](https://man.freebsd.org/cgi/man.cgi?query=timer_settime&sektion=2&manpath=FreeBSD+14.0-RELEASE)

## Conditions de validation

Cet audit est fondé sur les sources TaskMate et la documentation officielle FreeBSD. Aucun build ni
prototype FreeBSD/amd64 n'a été exécuté dans l'environnement actuel. Le verdict devra être confirmé
par un prototype de changement de contexte préemptif avant d'engager le portage complet.
