# Audit du tokenizer et des stratégies d’allocation

## Périmètre

Cet audit étudie la faisabilité de sortir `tokenizer()` d’autoCode pour en faire un composant
généraliste utilisable à la fois :

- par autoCode sur l’hôte, avec une allocation dynamique fondée sur `malloc()`/`realloc()` ;
- par TaskMate sur microcontrôleur, avec un stockage déterministe et éventuellement un slab
  allocator.

L’analyse couvre le tokenizer d’autoCode, ses neuf appels, le tokenizer privé de SCLI, les contrats
mémoire actuels, les contraintes AVR et les frontières d’architecture. Aucun code n’a été modifié et
aucune génération autoCode n’a été lancée pendant l’audit.

## Conclusion

La refactorisation est faisable, avec une bonne valeur pour autoCode et SCLI. En revanche, le moteur
lexical ne doit pas dépendre directement de `malloc()` ou d’un slab allocator.

L’architecture recommandée est :

```text
tokenizer générique, sans allocation
            |
     tokens = vues {ptr, longueur}
       /                     \
autoCode                    TaskMate/SCLI
buffer dynamique            buffer borné existant
malloc/realloc              tableau fixe ou slab optionnel
```

| Élément | Faisabilité | Intérêt actuel |
|---|---:|---:|
| Sortir le moteur lexical d’autoCode | Élevée | Élevé |
| Allocation dynamique dans autoCode | Élevée | Élevé |
| Réutilisation dans SCLI | Élevée | Élevé |
| Slab générique dans TaskMate | Moyenne à élevée | À justifier |
| Slab spécifiquement pour les tokens SCLI | Élevée techniquement | Faible actuellement |

## État actuel

Il existe déjà deux tokenizers indépendants :

- celui d’autoCode, avec gestion partielle des guillemets et copie des tokens dans des buffers fixes
  ([tokenizer.c](../../srcs/autoCode/tokenizer.c#L17)) ;
- `scliTokenize()`, qui découpe directement le buffer de commande en y insérant des caractères nuls
  ([scli.c](../../srcs/system/services/scli.c#L87)).

Cela confirme l’intérêt d’un moteur commun, mais leurs contrats sont différents :

- autoCode doit conserver la ligne originale, notamment lorsque `parseTag()` la réécrit ;
- autoCode conserve les guillemets des messages `*.err`, car ils sont ensuite injectés comme
  littéraux C ;
- SCLI peut modifier son buffer et ne gère ni guillemets ni échappements.

Le tokenizer d’autoCode associe actuellement trois responsabilités dans une même structure :

- le stockage de la ligne, limité à 256 octets ;
- quatre tokens de 64 octets chacun ;
- l’état et le résultat de l’analyse lexicale
  ([tokenizer.h](../../srcs/autoCode/tokenizer.h#L20)).

Tous les appelants consomment les tokens pendant le traitement de la ligne courante. Aucun ne conserve
un pointeur vers un token après l’analyse de la ligne. Cette durée de vie courte facilite une
allocation dynamique par ligne côté hôte et l’emploi de vues côté MCU.

## Défauts à corriger avant l’extraction

### P1 — Débordement d’un token entre guillemets

Avec un contenu cité de 62 caractères, le guillemet fermant peut remplir le dernier octet disponible,
puis le caractère nul est écrit hors du tableau
([tokenizer.c](../../srcs/autoCode/tokenizer.c#L53)).

Ce défaut doit être corrigé avant de figer un nouveau contrat ou d’écrire des tests de compatibilité.

### P1 — Dépendance à des emplacements inexistants dans un vecteur dynamique

Les appelants supposent que quatre emplacements de tokens existent toujours, même lorsque `count` est
inférieur. `parseTag()` lit par exemple `tokens[1]` avant de contrôler le nombre de tokens
([parseTag.c](../../srcs/autoCode/parseTag.c#L123)).

Le tableau fixe actuel masque ces accès en initialisant le premier caractère de chaque emplacement.
Une conversion directe vers un vecteur dynamique provoquerait des lectures hors limites.

### P1 — Un mauvais nombre de tokens ne stoppe pas globalError()

`globalError()` signale un nombre différent de trois, mais continue le traitement et accède aux trois
éléments ([globalError.c](../../srcs/autoCode/globalError.c#L42)). Le nouveau tokenizer devra retourner
un statut explicite, et l’appelant devra abandonner la ligne avant tout accès aux tokens manquants.

### P1 — Les limites actuelles masquent des débordements dans les appelants

Un nom de module de longueur exactement égale à `MOD_NAME_SIZE_MAX` est accepté, puis copié avec
`strcpy()` dans un tableau qui n’a plus de place pour le caractère nul
([parseInitrc.c](../../srcs/autoCode/parseInitrc.c#L71)).

Le contrôle du nombre de modules utilise également `>` au lieu de `>=` avant l’écriture dans le
tableau ([parseInitrc.c](../../srcs/autoCode/parseInitrc.c#L120)).

Supprimer la limite de 64 caractères du tokenizer augmenterait immédiatement l’exposition de ces
défauts. Tous les consommateurs doivent donc valider leurs limites sémantiques avant les copies.

### P2 — La ligne d’entrée reste limitée

Les fichiers sont lus avec `fgets()` dans un buffer de 256 octets. Une ligne plus longue est interprétée
comme plusieurs enregistrements sans diagnostic
([options.c](../../srcs/autoCode/options.c#L129)).

Rendre seulement les tokens dynamiques ne rendrait pas l’entrée généraliste. L’adaptateur autoCode doit
aussi utiliser un lecteur de ligne dynamique, soit avec `getline()` et une dépendance POSIX documentée,
soit avec un lecteur portable fondé sur `realloc()`.

### P2 — Troncatures silencieuses

Un token trop long n’est pas rejeté : sa suite peut être interprétée comme un nouveau token. Les tokens
au-delà du quatrième sont ignorés sans statut d’erreur. Ces deux situations peuvent faire accepter une
entrée différente de celle fournie.

### P2 — Grammaire insuffisamment définie

Le tokenizer ne gère pas les séquences d’échappement, accepte implicitement un guillemet non fermé et
traite une tabulation comme un séparateur même entre guillemets. Les commentaires sont interprétés par
les appelants plutôt que par le tokenizer.

Le nouveau contrat doit préciser :

- les séparateurs reconnus ;
- la conservation ou la suppression des guillemets ;
- la politique des échappements ;
- la détection des guillemets non fermés ;
- le traitement des commentaires ;
- le résultat retourné en cas de capacité insuffisante.

## Architecture recommandée pour le tokenizer

Le cœur commun doit être non modifiant et sans allocation. Il peut exposer un itérateur qui retourne
une vue à chaque appel :

```c
typedef struct
{
	const char *data;
	size_t length;
	bool quoted;
} token_view_t;

tokenizer_status_t tokenizerNext(tokenizer_t *state, token_view_t *token);
```

Une vue évite de copier le texte, préserve la ligne originale pour `parseTag()` et permet à chaque
environnement de choisir son stockage. Le tokenizer ne doit pas inclure autoCode, HAL, sysCall,
services ou tâches.

L’implémentation pourrait vivre dans une nouvelle bibliothèque feuille, hors d’autoCode et hors de
`interfaces/`. Le dossier `interfaces/` doit rester un ensemble de contrats portables et neutres, pas
devenir une bibliothèque d’implémentations.

Le build devra sélectionner explicitement le même cœur pour les deux programmes :

- le firmware l’ajoutera à `PATHS_SOURCES` ;
- la cible hôte l’ajoutera à `FILES_AUTOCODE_SRC`, actuellement limitée aux sources sous
  `srcs/autoCode/` ([sources.mk](../../mk/sources.mk#L29)).

## Adaptateur autoCode avec malloc()

L’usage de l’allocation dynamique est acceptable côté hôte. autoCode emploie déjà `malloc()` et
`free()` pour le nom des fichiers temporaires
([fileUtility.c](../../srcs/autoCode/fileUtility.c#L129)).

L’adaptateur recommandé doit :

- conserver une ligne redimensionnable ;
- accumuler des `token_view_t` dans un vecteur contigu agrandi avec `realloc()` ;
- appliquer une stratégie de croissance contrôlée avec vérification de débordement ;
- libérer la liste après le traitement de chaque ligne ;
- retourner un statut d’échec d’allocation plutôt que des données partielles ;
- préserver les limites métier propres aux noms, chemins, erreurs et commandes.

Une allocation par token n’est pas nécessaire. Les vues peuvent toutes pointer dans le buffer de la
ligne. Cette solution réduit le nombre d’allocations, simplifie la libération et évite la fragmentation
du heap hôte.

Le comportement en cas d’échec peut rester fatal pour autoCode, mais la bibliothèque commune ne doit
pas appeler `exit()` elle-même.

## Adaptateur TaskMate

SCLI dispose actuellement d’une ligne statique de 64 octets et de quatre pointeurs d’arguments
([scli.c](../../srcs/system/services/scli.c#L27)). Sur AVR, ce tableau de pointeurs coûte seulement
quelques octets et fournit un comportement simple et déterministe.

La première migration TaskMate devrait donc réutiliser le cœur commun tout en conservant ce stockage
fixe. Cela permet de supprimer le tokenizer dupliqué sans introduire immédiatement :

- une nouvelle panne par épuisement de pool ;
- des règles de propriété et de libération ;
- une section critique supplémentaire ;
- un coût statique permanent en SRAM.

Si SCLI doit ensuite accepter un nombre variable de tokens, un slab peut fournir dynamiquement des
descripteurs. Le texte doit rester dans `scli_line` ; le slab ne doit pas allouer une copie de chaque
chaîne.

## Faisabilité d’un slab allocator TaskMate

Un slab générique est cohérent s’il reste un pool statique d’objets homogènes. Il ne doit pas être
présenté comme un remplacement général de `malloc()`.

Le contrat conseillé est :

- mémoire et capacité du pool déterminées à la compilation ;
- taille et alignement homogènes pour tous les blocs du pool ;
- allocation et libération en temps constant par liste libre ;
- bitmap d’état pour détecter une double libération ou un pointeur étranger ;
- retour immédiat d’une erreur en cas d’épuisement, sans attente ni allocation secondaire ;
- compteur de blocs libres et pic d’utilisation pour le diagnostic ;
- initialisation avant le démarrage du scheduler ;
- usage depuis une ISR interdit dans une première version ;
- section atomique limitée au retrait ou au retour d’un bloc.

Les primitives AVR savent déjà sauvegarder l’état des interruptions, les désactiver puis restaurer
l’état précédent ([atomic.h](../../srcs/hal/arch/avr8/atomic.h#L23)). Le parcours lexical ne doit jamais
être exécuté dans la section atomique ; seules les opérations sur la liste libre doivent être
protégées.

L’alignement doit être explicite. Un simple tableau de `uint8_t` n’offre pas un alignement portable
pour tous les objets. La taille de bloc doit être arrondie à l’alignement déclaré et la base du pool
doit recevoir le même alignement.

L’API doit également valider lors de la libération :

- que le pointeur appartient au pool ;
- qu’il désigne exactement le début d’un bloc ;
- que le bloc était alloué ;
- que le calcul d’index ne déborde pas.

Si l’allocation est exposée aux tâches, l’accès doit passer par `sysCall`. L’implémentation et sa
politique appartiennent plutôt à `sysCore`; elles ne doivent pas être appelées directement par les
tâches ni être placées dans `interfaces/`.

## Adéquation du slab avec les tokens

Un slab convient aux descripteurs de tokens parce qu’ils ont tous la même taille. Il ne convient pas
directement à des chaînes de longueur variable. Fournir des chaînes possédées nécessiterait plusieurs
classes de tailles ou des chaînes de blocs, ce qui transformerait le composant en gestionnaire de heap
plus complexe et potentiellement fragmentable.

TaskMate possède un candidat plus convaincant pour valider un slab : les quatre canaux de messages
forment déjà un pool fixe avec un bit `IN_USE` et une recherche linéaire
([msg.c](../../srcs/system/services/msg.c#L29)).

Une migration ultérieure des canaux pourrait mutualiser l’allocation déterministe, mais elle devra
d’abord définir l’atomicité, la propriété et la durée de vie des objets. Elle ne doit pas être incluse
dans la première extraction du tokenizer.

## Impact SRAM

Le dernier artefact présent dans `build/` indique 1 807 octets de SRAM utilisés sur 8 192, dont
1 024 octets de piles statiques de threads. Cette valeur est indicative : elle n’a pas été reproduite
par une compilation propre et isolée pendant cet audit.

Un pool slab ajoute sa capacité complète à `.bss`, même lorsqu’il est vide. La mesure devra comparer au
minimum plusieurs capacités, par exemple 64, 128 et 256 octets, puis contrôler le delta exact avec
`avr-size`, `avr-nm` et la map de l’éditeur de liens.

Pour le seul tableau `argv[4]` de SCLI, le slab coûterait probablement plus de SRAM et de code qu’il
n’en économiserait. Son adoption doit être fondée sur plusieurs consommateurs ou sur une exigence
fonctionnelle de capacité dynamique.

## Stratégie de validation

### Tests du cœur lexical

- ligne vide et ligne ne contenant que des séparateurs ;
- espaces et tabulations ;
- tokens simples et tokens entre guillemets ;
- chaîne vide entre guillemets ;
- guillemet non fermé et séquences d’échappement ;
- limites exactes et dépassement de capacité ;
- tokens supplémentaires après la capacité fournie ;
- entrée non terminée par un retour à la ligne ;
- pointeurs nuls et longueurs nulles ;
- vérification que l’entrée n’est jamais modifiée.

### Tests autoCode

- sorties générées octet pour octet identiques sur les entrées valides actuelles ;
- diagnostics précis pour chaque fichier et numéro de ligne ;
- simulation d’échecs de `malloc()` et `realloc()` ;
- tests ASan et UBSan du programme hôte ;
- lignes et tokens dépassant les anciennes limites ;
- `bmake autoCode_alone`, revue du log et revue de tous les fichiers générés.

### Tests du slab

- épuisement exact du pool ;
- réutilisation après libération ;
- double libération ;
- pointeur extérieur et pointeur au milieu d’un bloc ;
- respect de l’alignement ;
- compteurs libres et pic d’utilisation ;
- alternance allocation/libération depuis plusieurs tâches ;
- mesure du temps maximal avec interruptions actives ;
- build AVR complet et mesure SRAM/flash ;
- test sur Arduino Mega avec préemption et scheduler actifs.

## Découpage recommandé

1. Ajouter des tests de caractérisation du tokenizer actuel et des sorties autoCode.
2. Corriger les dépassements, les contrôles de compte et les troncatures silencieuses.
3. Extraire le cœur lexical sans allocation.
4. Migrer autoCode vers une ligne et un vecteur dynamiques.
5. Migrer SCLI vers le cœur commun en conservant initialement son stockage fixe.
6. Implémenter le slab séparément avec ses tests hôte et AVR.
7. Mesurer SRAM/flash, régénérer autoCode, compiler le firmware et valider sur Arduino Mega.

L’extraction du tokenizer et le backend `malloc()` d’autoCode peuvent donc être engagés. Le slab
TaskMate est également réalisable, mais il doit rester une primitive indépendante et son intégration
aux tokens doit attendre qu’un besoin runtime justifie son coût permanent.
