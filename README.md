d2lootfilter
==============

d2lootfilter is a plugin that can be used with [PlugY](http://plugy.free.fr/en/index.html) or other mod systems like
[Diablo II Base Mod](https://www.moddb.com/mods/basemod) to filter loot on 1.10f, 1.13c, and 1.14d.
The syntax for filtering loot was made to be similar to [Path of Exile](https://pathofexile.fandom.com/wiki/Item_filter).

* [Sample Filter](docs/d2lootfilter-Rules.txt)
* [Features](#Features)
* [Improvement](#Improvement)
* [Installing](#Installing)
* [Basic Syntax](#Basic-Syntax)
* [Operators](#Operators)
* [Conditions](#Conditions)
  * [Stats](#Stats)
* [Actions](#Actions)
* [Styles](#Styles)
* [Settings](#Settings)
* [In Game Commands](#In-Game-Commands)
* [Credits](#Credits)

This is major improved [0.7-alpha version](https://github.com/qqkookie/d2lootfilter/tree/Kookie) by @qqkookie,
based on [d2lootfilter version 0.6-alpha](https://github.com/dschu012/d2lootfilter) by @dschu012, released on Sep. 23, 2021.

Original README of 0.6-alpha is [here](docs/README-v6a.md).
Original v0.6-alpha filter example is [here](docs/item.filter-v6a.txt) for expert D2 user.

NOTE: **This has not been heavily tested, crashes may happen, and items may be lost**


### Feature

* Filter loot by various attributes (still a work in progress)
* Notifications for loot
* Minimap icons for loot
* Custom name/descriptions/background colors for ground and inventory loot.

Still to do:

* Error handling. Filter just silently ignores errors right now.
* Disable sound for hidden drops (no plan)
* Tier system to change filter levels in game.

Example with custom item name/border/background color/inventory background color and chat notification.

![image](https://user-images.githubusercontent.com/1458109/114068726-33ed1f00-986c-11eb-8cca-686efb629e8e.png)

<br>

See what rules in your config caused the item to be shown. Set background colors for charms/items
to quickly find charms that need to be replaced etc...

![image](https://user-images.githubusercontent.com/1458109/114068812-4cf5d000-986c-11eb-9795-fd7e1d6a8683.png)

<br>

### Improvement

This version has few improvements over version 0.6-alpha, and uses little different rule syntax and conditions.
New features are:

* `Check` clause instead of `Show` ... `Continue`
* New `Or` condition that combines two alternative condition lines.
* All \{token\} names have no space in it, like \{Ping Level\} -> \{PingLevel\}
* Old quoted token style like `"Ping Level" > 2` is deprecated. Use \{PingLeve\} instead. Stats variables are exception.
* Few more named colors for text and background. Color name changed like \{Dark Green\} -> \{DarkGreen\}
* `CheckNotify` can take a digit argument as minimum ping level.
* In setting ini file, `Path=` filter file path may have \{SaveDir\} and \{CharName\} variable token.
* `Rarity` order fixed: Magic < Rare < Set < Unique.
* Long condition line can span multiple physical lines with trailing comma (,). Back slash (\\) at end of line is optional.
* `Difficulty` condition can take game Act number. Like `Nightmare Act2` or `Act3` regardless of difficulty.
* Additional tokens in `SetName` or `SetDescription` action:
  \{ItemType\}, \{ItemLevel\}, \{ItemCode\}, \{AffixLevel\}, \{GemGrade\}, \{GemColor\}\.
* \{PotionNumber\} \{RuneNumber\} changed to \{PotionGrade\} \{RuneGrade\}\.
* Throwing potions (like Exploding Potion) have {PotionGrade}
* `ItemCat` condition is one of Weapon, Armor, Accessory, Socketable, Consumable, Misc.
* `ItemSize` condition is number of squares it takes in inventory.
* `CharacterName` and `CharacterMaxHP` (character's max HP) condition.
* `ItemMode` : Item placement. Drop on ground, inventory, stash, equipped, mercenary, vendor, etc ...
* Old `Defense`, `Armor`, `Weapon`, `Hight` and `Width` conditions are removed.
* Section name in `d2lootfilter.ini` is changed to "\[D2LootFilter\]" from old "\[Settings\]".
* Setting can be read from "\[D2LootFilter\]" section of `BaseMod.ini` or `Plugy.ini`
* Settings ini and filter rule file are reloaded whenever starting new game session.
* `d2lootfilter.dll` is optimized for speed. DLL file size reduced by 20%.
* Few bug fix and `/help` chat command.
* Development toolset is updated to MS Visual C++ 2022 (v143).


### Installing

Steps to install on PlugY:

* Download the [latest zip file](https://github.com/qqkookie/d2lootfilter/releases/latest/download/d2lootfilter.zip) and unzip it.
* Copy `d2lootfilter.dll` file to your Diablo 2 install directory. And `d2lootfilter-Rules.txt` file to Diablo 2 Save directory.
* Change `DllToLoad=` to `DllToLoad=d2lootfilter.dll` in PlugY.ini.
* Or alternatively, it can be loaded by Diablo 2 [BaseMod](https://www.moddb.com/mods/basemod) as ExtraDll.
Change `Basemod.ini` like below. PlugY.dll can be loaded by Base Mod, too.

```ini
[ExtraDll2]
Enabled=1
LoadDll2="PlugY.dll"

[ExtraDll3]
Enabled=1
LoadDll3="d2lootfilter.dll"
```


### Basic Syntax

The basic syntax is a collection of `Show`, `Hide` and `Check` condition blocks.
If an item matches **all conditions** on the block, the item will respectively be hidden or shown.
`Check` condition block does same condition match and action but does not show or hide action.
Use of old `Continue` clause at end of Show block is deprecated.
Item matched by `Check` block will continue trying to match other condition blocks,
otherwise processing stops after the first `Show` or `Hide` block is matched.
A sample filter can be found [here](docs/d2lootfilter-Rules.txt).

```text
# hides all inferior items
Hide
    Rarity Inferior

# append the rune number to all runes
Check
    Class Rune
    SetName {Name} {Red} {RuneNumber}

# Even rune is matched in Check block, rune will continue to next block.
# This Show block will also match. The modified name carries over
# from the previous condition block
Show
    Rune >= Vex
    ChatNotify True
    MinimapIcon Purple
```

Hiding item applies only to items dropped on ground. Items in inventory or store will show always.

### Operators

The operators are as followed. If no operator is specified `=` is implied.

| Operator | Description |
|-|-|
| `=` | Equals |
| `!=` | Not Equals |
| `<` | Less Than |
| `<=` | Less Than Equals |
| `>` | Greater Than |
| `>=` | Greater Than Equals |
| `in X-Y` | Between values X and Y |
| `!` | Negate True/False (unary prefix operator) |

<br>

### Conditions

| Name | Valid Values |
|-|-|
| Code `<Operator> <Value>` | 3 character item code found in `weapons.txt`, `armor.txt`, or `misc.txt`. For 1.13c and 1.14d these can be found [here](docs/Types.md). |
| Type `<Operator> <Value>` | Name of item in `strings.txt` lookup from `weapons.txt`, `armor.txt`, or `misc.txt` . i.e. `Swirling Crystal`. For 1.13c and 1.14d these can be found [here](docs/Types.md). |
| Class `<Operator> <Value>` | ItemType column from `itemtypes.txt`. For 1.13c and 1.14d these can be found [here](docs/Class.md). |
| Rarity `<Rarity>` | Inferior, Normal, Superior, Magic, Rare, Set, Unique, Crafted |
| Ethereal `<Boolean>` | Boolean True or False |
| Runeword `<Boolean>` | Is this runeword item or not. Boolean True or False |
| Rune `<Operator> <Value>` | Rune Name or Number. For 1.13c and 1.14d these can be found [here](docs/Runes.md). |
| ItemLevel `<Operator> <Value>` | Number |
| Quality `<Operator> <Quality>` | Normal, Exceptional, Elite |
| ItemId `<Operator> <Value>` | (to do) Unique or Set ID from `sets.txt` or `uniques.txt` |
| ItemMode `<Value>` | Item placement: Drop, Vendor, Inventory, Stash, Cube, Equip, Belt, Hiring, Socket |
| Prefix `<Operator> <Value>` | Prefix ID from `magicprefix.txt` (to do... human readable name) |
| Suffix `<Operator> <Value>` | Suffix ID from `magicsuffix.txt` (to do... human readable name) |
| Stats `<Expression>` | Expression that evaluates to true or false to filter an item based on stats. More details can be found in [Stats](#Stats) |
| Identified `<Boolean>` | Boolean True or False |
| Sockets `<Operator> <Number>` | Number of sockets |
| Price `<Operator> <Value>` | Price when vendoring item |
| Gold `<Operator> <Value>` | Gold value |
| ItemCat `Value` |  Item category: Weapon, Armor, Accessory, Socketable, Consumable, Misc |
| ItemSize `<Operator> <Number>` | Number of squares item occupies in inventory. (Width * Hight) |
| AffixLevel `<Operator> <Number>` | Magic/rare item affix level |
| WeaponDamage `<Operator> <Value>` | Average of min & max damage * enhanced damage |
| ArmorDefense `<Operator> <Value>` | Item defense * enhanced defense |
| Or | Ignore a failed condition in previous line and proceed to next condition line. |
| Difficulty `<Operator> <Value>` | Normal, Nightmare, Hell, Act1, Act2, ... Act5, or Normal Act1, ..., Hell Act5  |
| CharacterClass `<Value>` | Amazon, Assassin, Barbarian, Druid, Necromancer, Paladin, Sorceress |
| CharacterLevel `<Operator> <Value>` | Number |
| CharacterName `<Value>` | Character name of player, case-sensitive. |
| CharacterMaxHP `<Value>` | Character maximum HP |
| Defense `<Operator> <Value>` | (DEPRECATED) Item defense (armor class) or it can be replaced by `Stats {Defense}`. |
| Armor `<Boolean>` | (REMOVED) Boolean True or False. True if item is any armor. |
| Weapon `<Boolean>` | (REMOVED) Boolean True or False. True if item is any weapon. |
| Width `<Operator> <Number>` | (REMOVED) Width of item in inventory |
| Height `<Operator> <Number>` | (REMOVED) Height of item in inventory |

<br>

* `,` essentially works as an `or` operator. i.e.
`Type Swirling Crystal, Dimensional Shard` would match either item.
* In `Rarity`: Superior < Magic < Rare < Set < Unique
* In `ItemCat`: Helm, gloves, boots, belt, shield are `Armor`.
Ring, amulet, charm, torch are `Accessory`. Gem, jewel, rune are `Socketable`.
* Throwing axe, knife, javelin, orb, staff, wand are `Weapon`.
Throwable potions, arrow and bolt are `Consumable`, not weapon.
* Quest items and other are `Misc`. Quest weapons like "Khalim's Will" are `Weapon`.
* `Or` condition will ignore failed result of previous line. `Or` should be used alone in separate line.
* In `ItemMode` : Drop means on ground. Vendor in vendor store. Equip on body.
Hiring, equipped by mercenary. Belt in potion row. Socket means inserted in socket of other item.


### Stats

Stats are condition expressions that evaluate to true or false.
i.e `"All Resists" > 0 and "Life" > 0` would match items with both all resists and life.
A list of keywords that can be used in stats expressions can be found in these
[Stats](docs/Stats.md), [Skills](docs/Skills.md) documents.
These keywords must be quoted like `"..."` or `{...}`.
Keyword contains spaces like `"Amazon +%d to Bow and Crossbow Skills"` or `{Faster Hit Recovery}`.

<br>

| Functions | Description |
|-|-|
| `Stat` | Use other stats that are not in [Stats](docs/Stats.md). 2 arg function. The first arg is the stat id from `itemcoststats.txt`. The second arg is optional layer (used for skill stats). Returns the unsigned value of the stat. |
| `ChargedSkill` | Check the skill level of a charged skill. i.e. `ChargedSkill(54) > 0` will check if an item has charges of teleport |
| `ClassSkill` | Check if an item has a certain class skill. i.e. `ClassSkill(1) > 0` will check if an item has +To All Sorc skills. |
| `TabSkill` | Check if an item has a certain tab skill. i.e. `TabSkill(34) > 0` will check if an item has +To to Warcries skills. |
| `Class` | Check if an item is a certain type/class. 1 arg function of the class from `itemtypes.txt`. |
| `Min` | varargs. returns the minimum (non-zero, exclude stats that don't exist) value from a list |
| `MinIn` | varargs. returns the minimum (including zeros, i.e. stats that don't exist on the item) value from a list |
| `Max` | varargs. returns the maximum (non-zero, exclude stats that don't exist) value from a list |

e.x. `Max(Stat(39), Stat(43), Stat(41), Stat(45)) > 0` can be used to filter the existence of
any resistance and `MinIn(Stat(39), Stat(43), Stat(41), Stat(45)) > 0` for all resistances

* Additional numeric (integer) variables can used in expression evaluation :<br>
   \{FilterLevel\}, \{PingLevel\}, \{ItemLevel\}, \{QualityLevel\}, \{MagicLevel\},
   \{AffixLevel\}, \{CraftAffixLevel\}, \{Sockets\}, \{Random99\}, \{Price\}, \{RuneGrade\},
   \{ItemCode\}, \{WeaponDamage\}, \{ItemSize\}, \{CharacterLevel\}, \{CharacterMaxHP\}\.
* \{WeaponDamage\} is average of maximum and minimum damage of weapon. \{Random99\} is between 0-99.


### Actions

* Name and Description can use the following color tokens: <br>
 \{White\}, \{Red\}, \{Green\}\(set item color\), \{Blue\}\(magic\), \{Gold\}\(unique\), \{Gray\}\(socket\),
 \{Black\}, \{Tan\} \(dimer \{Gold\}\), \{Orange\}, \{Yellow\}\(rare\), \{Purple\}, \{DarkGreen\}\.
* Additional text colors : <br> \{MediumRed\} \(blood red\), \{MediumGreen\} \(grass green, lighter than set \{Green\}\),
 \{MediumYellow\}\(dim yellow than rare \{Yellow\}\), \{MediumBlue\} \(turquoise blue\-green color\),
 \{DarkBlue\} \(darker than magic \{Blue\}, hard to read on dark background\)\.
* These text colors can not be used as pallet color.
 These additional text colors depend on graphic mode (ddraw/d3d, opengl, glide, 3dfx(d2dx)), D2 version and glider wrapper.
 \{Coral\}, \{Sage\}, \{Teal\}, \{LightGray\} color works only on glide mode D2 v1\.13c or below\.
* D2 Glide wrapper [D2DX](https://github.com/bolrog/d2dx) have a glitch that displays some text color black.
 It is fixed by my [patch](https://github.com/qqkookie/d2dx/tree/Kookie)
* Additional pallet colors: \{Coral\}, \{Sage\}, \{Teal\}, \{LightGray\} may be used
 in `SetBackgroundColor`, `SetInventoryColor`, `SetBorderColor` as pallet color.
* Name and Description can use the following special tokens: <br>
 \{Price\} \(item vendor buy price\), \{Sockets\} \(number of sockets\), \{NewLine\},
 \{RuneGrade\}, \{PotionGrade\}, \{GemGrade\} \(1: chipped gem \- 5: perfect gem\), \{GemColor\}\.
 \{ItemLevel\}, \{ItemType\} (readable class(type) name from [Class.md](docs/Class.md),
 \{ItemCode\} \(numerical item type\), \{AffixLevel\}\.
* Throwing potions (like Exploding Potion, Rancid Gas Potion) have {PotionGrade}
* Note that all color names and tokens have no space in their name.

<br>

| Name | Valid Values |
|-|-|
| SetStyle `<Value>` | Sets the styling for an item. A style is a group of actions that will be applied. See [Styles](#Styles). |
| SetName `<Value>` | Sets the name for an item. Special token \{Name\} is the base name for the item. When using continue it will append from the previous condition block. |
| SetDescription `<Value>` | Sets the description for an item.  Special token \{Description\} is the base name for the item. When using continue it will append from the previous condition block. |
| SetBackgroundColor `<Value>` | Sets the background color of the item when on the ground. Pallette index color in decimal number or hexadecimal ("x3c") or White, Red, Green, Blue, Gold, Gray, Black, Tan, Orange, Yellow, Purple, DarkGreen. No curly braces (\{White\}) needed. |
| SetInventoryColor `<Value>` | Sets the background color of the item when in your inventory. Value is a pallette index color same as SetBackgroundColor value. |
| SetBorderColor `<Value>` | Sets the border color of the item when on the ground. Value is a pallette index color. |
| ChatNotify `<Boolean>` or `<digit>` \[msg\] | Notify  in chat when the item drops. True or False |
| MinimapIcon `<Value>` | Sets the color of the item on your minimap when on the ground. Value is a pallette index color. |

<br>

NOTE:<br>
`ChatNotify`: PingLevel may mean either lower bound or higher bound for notification.
When ChatNotify argument is single digit number like `ChatNotify 2`,
it is same as `ChatNotify {PingLevel} <= 2`.
It will notify when ping level is equal to or LESS than 2.
Low ping level means more frequent notification. This usage support optional text message.
Old reverse ping level usage like `ChatNotify {PingLevel} > 4` is also supported.
This will notify when ping level is HIGHER than 4.


### Styles

Lets you apply a group of actions to an item. i.e.

```text
Style Tier 1 Items
    SetName {Purple}T1 {Name}
    ChatNotify True
    MinimapIcon Purple
    SetInventoryColor Purple

Show
    Type Diadem
    Rarity Unique
    SetStyle Tier 1 Items

Show
    Type Unearthed Wand
    Rarity Unique
    SetStyle Tier 1 Items
```

will apply all of the `Tier 1 Items` styles to the items.

### Settings

The first time you join a game with the plugin loaded it should create a `d2lootfilter.ini` settings file
in your Diablo II directory. These are the following settings that can be changed.
Or setting can be read from  "\[D2LootFilter\]" section of `BaseMod.ini` or `Plugy.ini`.


| Setting | Description |
|-|-|
| Path | Path to your filter file. Default: `{SaveDir}/d2lootfilter-Rules.txt` |
| FilterLevel | (to do) Used to dynamically change how strict your filter while playing. (Currently unused, still a planned feature) Default: `1` |
| PingLevel | Used to dynamically change how strict drop notification are while playing. Default: `1` |

* Settings and filter file are reloaded for each new game sessions.
* `Path=` setting may include \{SaveDir\} and \{CharName\} variable token.
 \{SaveDir\} is "%USERPROFILE%/Saved Games/Diablo II" dir for Diablo v1.14d version
 or "./Save" dir under Diablo game install folder for older versions.
 \{CharName\} is case-sensitive name of Diablo game character, user is playing.
* If `d2lootfilter.ini` does not exist in D2 dir, d2lootfilter settings can be read from
 "\[D2LootFilter\]" section of `BaseMod.ini` or `PlugY.ini`.
* PingLevel may mean either lower bound or higher bound
 for notification defending on ChatNotify usage. See NOTE of Action section.

### In Game Commands

The filter has a few in-game commands for changing settings.

| Command | Description |
|-|-|
| `/reload` | Reloads your filter. |
| `/debug` | Toggles debugging. |
| `/test <number>` | Tests a specific rule (by line number) against your currently hovered mouse item. |
| `/filterlevel <number>` | (to do) Change the filter level. |
| `/pinglevel <number>` | Change the ping/notification level. |
| `/help` | Display short chat '/command' list of d2lootfilter, D2, PlugY  |

* In debug mode, all items hidden by the rules will be displayed
 in gray color with "*" marker at end to indicate it is hidden.

### Credits

* Original author of d2lootfilter is Daniel Schumacher (@dschu012).
It can be found [here](https://github.com/dschu012/d2lootfilter).
* This version is forked on Nov. 25, 2022 and improved by @qqkookie.
Plan to merged to dschu012/master. [Git repo](https://github.com/qqkookie/d2lootfilter/tree/Kookie)

Special thanks to everyone that has shared their work at
[Phrozen-Keep](https://d2mods.info/forum/viewforum.php?f=8) [(Discord)](https://discord.gg/NvfftHY).

To name a few Necrolis, Lectem, Kingpin, whist, Revan, etc...

Thanks to coffin_spirit on Discord for the 1.10f implementation.
