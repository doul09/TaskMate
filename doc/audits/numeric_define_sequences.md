# Audit des suites numériques déclarées avec `#define`

## Objet et méthode

Cet audit couvre les macros objet des fichiers C et des en-têtes sous `srcs/`. Les régions générées
ont été lues, mais ne doivent pas être modifiées directement. Une suite est considérée comme candidate
à un `typedef enum` lorsqu'elle décrit les valeurs mutuellement exclusives d'un même domaine, et non
une taille, une fréquence, une adresse, un masque ou une option évaluée par le préprocesseur.

Commande de repérage utilisée :

```sh
rg -n --glob '*.{c,h}' '^\s*#\s*define\s+[A-Za-z_][A-Za-z0-9_]*\s+' srcs
```

Les usages de chaque groupe ont ensuite été examinés afin de distinguer un domaine métier d'une
simple proximité numérique. Le type d'un `enum` C n'étant pas garanti sur 8 bits, les recommandations
ci-dessous conservent un stockage explicite en `uint8_t` là où la RAM, le format d'un état ou l'ABI le
requiert.

## Résultats

### Priorité haute — niveaux d'exécution

`tm_runLevel.h` déclare `RL_RUN_NONE` à `RL_RUN_USER` (0 à 4) et `RL_LEVEL_COUNT` avec six macros.
Ces constantes sont les valeurs fermées d'un même domaine et sont donc la meilleure candidate :

```c
typedef enum
{
	RL_RUN_NONE,
	RL_RUN_CORE,
	RL_RUN_DRIVER,
	RL_RUN_SERVICE,
	RL_RUN_USER,
	RL_LEVEL_COUNT
} rl_run_level_t;
```

`RL_LEVEL_MASK` et `RL_GET_RUN_LEVEL()` doivent rester des macros : ils décrivent l'encodage des trois
bits faibles d'un octet d'état, pas des membres supplémentaires du domaine. Les champs d'état et le
membre de `hal_driver_control_data_t` devraient rester en `uint8_t` si la taille est contractuelle. Les
API peuvent employer `rl_run_level_t` seulement après vérification de l'impact de `sizeof(enum)` avec
la chaîne AVR. Les affectations issues d'un masque nécessiteront alors une conversion explicite.

**Gain :** domaine nommé, signature plus lisible, valeur sentinelle liée à la suite et suppression du
risque de désynchroniser `RL_LEVEL_COUNT`.

### Priorité haute — catégories et sous-types de modules

`tm_modules.h` juxtapose actuellement deux domaines différents :

- `TM_MOD_DRIVER_ID` et `TM_MOD_THREAD_ID` indexent `modules_type[]` ;
- `TM_MOD_THREAD_SYS_ID` et `TM_MOD_THREAD_USER_ID` décrivent le sous-type d'un thread.

Ils ne devraient pas former artificiellement une seule suite 0 à 3. Deux types rendent la frontière
explicite :

```c
typedef enum
{
	TM_MOD_DRIVER_ID,
	TM_MOD_THREAD_ID,
	TM_MOD_TYPE_COUNT
} mod_type_id_t;

typedef enum
{
	TM_MOD_THREAD_SYS_ID = 2,
	TM_MOD_THREAD_USER_ID
} mod_thread_type_id_t;
```

La valeur explicite `2` du second type préserve le format consommé et généré par autoCode. La
migration doit partir des sources de vérité autoCode et vérifier le code régénéré. Dans
`module_item_t`, le stockage de `type` et `subtype` peut rester `unsigned char`; remplacer ces champs
par des enums sans mesurer leur taille augmenterait potentiellement chaque entrée de la base de
modules sur AVR.

**Gain :** empêche de confondre un index de table avec un sous-type et rattache le cardinal de la table
au bon domaine.

### Priorité moyenne — politique d'ouverture des fichiers autoCode

`FILE_READONLY` et `FILE_CREATE` sont deux politiques mutuellement exclusives passées à `fileOpen()`.
L'outil autoCode s'exécute sur l'hôte et n'est pas soumis au budget RAM du microcontrôleur. Il peut donc
adopter directement :

```c
typedef enum
{
	FILE_READONLY = 1,
	FILE_CREATE
} file_open_policy_t;
```

Le paramètre `special_mode` de `fileOpen()` doit prendre ce type. Un troisième membre explicite
`FILE_OPEN_NORMAL = 0` ne doit être ajouté que si le comportement correspondant est réellement voulu.

**Gain :** la signature interdit conceptuellement des entiers sans signification et documente les
seules politiques acceptées.

## Groupes examinés mais non candidats

- `TM_LIBC_TASKMATE` et `TM_LIBC_CSTD` sont des commutateurs utilisés dans `#if`. Des identifiants
  d'énumération ne sont pas disponibles à l'évaluation préprocesseur ; ils doivent rester des macros,
  ou être remplacés par une unique macro de sélection lors d'un changement séparé.
- Les tailles et limites (`*_SIZE`, `*_COUNT_MAX`, `BYTE_INDEX`, `AVR8_REGISTER_COUNT`) servent aux
  dimensions de tableaux ou aux contraintes de capacité. Elles ne représentent pas les choix d'un
  domaine fermé.
- Les fréquences, durées et dimensions matérielles (`USART_BAUD_RATE`, `I2C_FREQ`, constantes RTC et
  LCD) sont des grandeurs avec unités, pas des suites.
- Les adresses, masques et motifs binaires (`*_I2C_ADDR`, `TM_MOD_CANARY`, `RL_LEVEL_MASK`) doivent
  conserver leur largeur, leur suffixe entier et leur rôle de constante de représentation.
- `TM_MOD_DRIVER_COUNT` et `TM_MOD_THREAD_COUNT` sont générés depuis la configuration. Ils décrivent
  des quantités propres au build, et non des valeurs alternatives.
- `DRV_CTRL_*`, `DRV_BIT_*`, `DRV_STATE_*` et `THREAD_BIT_*` sont déjà regroupés dans des `typedef
  enum`; aucune conversion n'est nécessaire.

## Ordre de migration recommandé

1. Convertir `FILE_READONLY` / `FILE_CREATE`, puis compiler l'outil autoCode et exécuter la génération.
2. Séparer les catégories et sous-types de modules dans leurs deux enums, adapter autoCode à ces types,
   régénérer et comparer toutes les régions balisées.
3. Convertir les niveaux d'exécution après avoir décidé si les API exposent `rl_run_level_t` ou gardent
   des octets de transport ; ajouter des assertions de build sur les masques et valeurs encodées.
4. Vérifier au minimum `bmake autoCode_alone`, la construction AVR complète, les frontières
   architecturales et la différence de taille flash/RAM avant/après.

Cette séquence commence par le changement hôte le moins risqué et réserve pour la fin le domaine des
niveaux d'exécution, transversal et encodé dans les octets d'état des threads et drivers.
