### d2lootfilter

d2lootfilter is a plugin that can be used with [PlugY](http://plugy.free.fr/en/index.html) or other mod systems  like [Base Mod](https://www.moddb.com/mods/basemod) to filter loot on 1.10f, 1.13c, and 1.14d. The syntax for filtering loot was made to be similar to [Path of Exile](https://pathofexile.fandom.com/wiki/Item_filter).

* [Sample Filter](doc/d2lootfilter-Rules.txt)
* [Features](#Features)
* [Difference](#Difference)
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

* This is improved [0.7-alpha version](https://github.com/qqkookie/d2lootfilter/tree/Kookie) by #qqkookie, based on [d2lootfiler version 0.6-alpha](https://github.com/dschu012/d2lootfilter) by #dschu012, released on Sep. 23, 2021.
* Original README of 0.6-alpha is [here](doc/README-v6a.md)
* Orignal fiter file example is [doc/item.filter-v6a.txt] for expert D2 user.

**This has not been heavily tested, crashes may happen, and items may be lost**

### Features

* Filter loot by various attributes (still a work in progress)
* Notifications for loot
* Minimap icons for loot
* Custom name/descriptions/background colors for ground and inventory loot.

Still to do:

* Error handling. Filter just silently ignores errors right now.
* Disable sound for hidden drops
* Tier system to change filter levels in game.

Example with custom item name/border/background color/inventory background color and chat notification.

![image](https://user-images.githubusercontent.com/1458109/114068726-33ed1f00-986c-11eb-8cca-686efb629e8e.png)

See what rules in your config caused the item to be shown. Set background colors for charms/items to quickly find charms that need to be replaced etc...

![image](https://user-images.githubusercontent.com/1458109/114068812-4cf5d000-986c-11eb-9795-fd7e1d6a8683.png)

### Difference

This version has few improvements over version 0.6-alpha, released Sep. 23. 2021, and uses similar but diffrent rule syntax. New features are:

 * `Check` statement instead of `Show` ... `Continue`
 * New `Or` condition to relate two condition blocks.
 * All {token} names have no space in it, like `{Potion Number}` -> `{PotionNumber}`
 * Old quoted token style like `"Ping Level" > 2` is deprecated. Use `{PingLeve}` instead. Stats variables are excetion.
 * Few more named colors for text and background. Color namke change like `{Dark Green}` -> `{DarkGreen}`
 * `CheckNotify` may have single digit number as minimum ping level.
 * `{SaveDir}` and `{CharName}` token in `Path=` setting to set filter rule file path.
 * `Rarity` order fixed: Magic < Rare < Set < Unique.
 * Condition line can span muliple physical lines with trailing comma (`,`).
 * `Difficulty` condition can take game Act number. Like `Nightmare Act2` or `Act3` regardless of difficulty.
 * Additional tokens in `SetName` or `SetDescription` action: `{ItemType}`, `{ItemLevel}`, `{ItemCode}`,`{AffixLevel}`.
 * `ItemCat` condition is one of { `Weapon`, `Armor`, `Accessory`, `Socketable`, `Consumable`, `Misc` }
 * `ItemSize` condition is number of squares it takes in inventory.
 * `CharacterName` (character name) and `CharacterMaxHP` (character's max HP) condition,
 * `ItemMode` : Item placement. Drop on ground, inventory, stash, equipped, mercenary, vendor, etc ...
 * Old `Defense`, `Armor`, `Weapon`, `Hight` and `Width` conditions are removed. Defense can be replaced with `Stats {Defense}`.
 * Section name in "d2lootfilter.ini" is changed to "[D2LootFilter]" (old:"[Settings]")
 * Settings ini and filter rule file are reloaded whenever entring new game session.
 * Default `d2lootfilter.dll` is optimized for speed. DLL size is also reducend by 20%.
 * Toolset is updated to MS Visual C++ 2022 (v143).


### Installing

Steps to install on PlugY:

* Download the [latest zip file](https://github.com/qqkookie/d2lootfilter/releases/latest/download/d2lootfilter.zip) and unzip it.
* Copy both d2lootfilter.dll and  d2lootfilter-Rules.txt files to your Diablo 2 directory.
* Change `DllToLoad=` to `DllToLoad=d2lootfilter.dll` in PlugY.ini
* Or alternatively, it can be loaded by Diablo 2 [BaseMod](https://www.moddb.com/mods/basemod) as ExtraDll. Change Basemod.ini like below. PlugY.dll can be loaded by Base Mod, too.

```
[ExtraDll2]
Enabled=1
LoadDll2="PlugY.dll"

[ExtraDll3]
Enabled=1
LoadDll3="d2lootfilter.dll"
```

### Basic Syntax

The basic syntax is a collection of `Show`, `Hide` and `Check` condition blocks. If an item matches **all conditions** on the block, the item will respectively be hidden or shown. `Check` condition block does same condition match and action but does not show or hide action. Item matched by `Check` block will continue trying to match other condition blocks, otherwise processing stops after the first block is matched. A sample filter can be found [here](doc/d2lootfilter-Rules.txt).

```
# hides all inferior items
Hide
    Rarity Inferior

# append the rune number to all runes
Check
    Class Rune
    SetName {Name} {Red}{RuneNumber}

# Even rune is matched in Check block, rune will continue to next block.
# This Show block will also match. the name carries over from the previous condiition block
Show
    Rune >= Vex
    ChatNotify True
    MinimapIcon Purple
```

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
| `!` | Negate True/False (unary operator) |

### Conditions

* `,` essentially works as an `or` operator. i.e. `Type Swirling Crystal, Dimensional Shard` would match either item.

| Name | Valid Values |
|-|-|
| Code `<Operator> <Value>` | 3 character item code found in `weapons.txt`, `armor.txt`, or `misc.txt`. For 1.13c and 1.14d these can be found [here](doc/Types.md). |
| Type `<Operator> <Value>` | Name of item in `strings.txt` lookup from `weapons.txt`, `armor.txt`, or `misc.txt` . i.e. `Swirling Crystal`. For 1.13c and 1.14d these can be found [here](doc/Types.md). |
| Class `<Operator> <Value>` | ItemType column from `itemtypes.txt`. For 1.13c and 1.14d these can be found [here](doc/Class.md). |
| Rarity `<Rarity>` | Inferior, Normal, Superior, Magic, Rare, Set, Unique, Crafted |
| Ethereal `<Boolean>` | Boolean True or False |
| Runeword `<Boolean>` | Boolean True or False |
| Rune `<Operator> <Value>` | Rune Name or Number. For 1.13c and 1.14d these can be found [here](doc/Runes.md). |
| ItemLevel `<Operator> <Value>` | Number |
| Quality `<Operator> <Quality>` | Normal, Exceptional, Elite |
| ItemId `<Operator> <Value>` | (to do) Unique or Set ID from `sets.txt` or `uniques.txt` |
| ItemMode `<Value>` | Item placement: { `Drop`, `Vendor`, `Inventory`, `Stash`, `Cube`, `Equip`, `Belt`, `Hiring`, `Socket`,  } |
| Prefix `<Operator> <Value>` | Prefix ID from `magicprefix.txt` (to do... human readable name) |
| Suffix `<Operator> <Value>` | Suffix ID from `magicsuffix.txt` (to do... human readable name) |
| Stats `<Expression>` | Expression that evaluates to true or false to filter an item based on stats. More details can be found in [Stats](#Stats) |
| Identified `<Boolean>` | Boolean True or False |
| Sockets `<Operator> <Number>` | Number of sockets |
| Price `<Operator> <Value>` | Price when vendoring item |
| Gold `<Operator> <Value>` | Gold value |
| ItemCat `Value` |  Item category: `Weapon`, `Armor`, `Accessory`, `Socketable`, `Consumable`, `Misc` |
| ItemSize `<Operator> <Number>` | Number of squares item occupys in inventory. (Width * Hight) |
| AffixLevel `<Operator> <Number>` | Magic/rare item affix level |
| Or | Ignore a failed condition in previous line and proceed to next condition line. |
| Difficulty `<Operator> <Value>` | Normal, Nightmare, Hell, Act1, Act2, ... Act5, or Normal Act1, ..., Hell Act5  |
| CharacterClass <Value>` | `Amazon`, `Assassin`, `Barbarian`, `Druid`, `Necromancer`, `Paladin`, `Sorceress` |
| CharacterLevel `<Operator> <Value>` | Number |
| CharacterName <Value>` | Character name of player. Case-sensitive. |
| CharacterMaxHP `<Value>` | Character maximum HP |
| Defense `<Operator> <Value>` | (REMOVED) Item defense (armor class) |
| Armor `<Boolean>` | (REMOVED) Boolean True or False. True if item is any armor. |
| Weapon `<Boolean>` | (REMOVED) Boolean True or False. True if item is any weapon. |
| Width `<Operator> <Number>` | (REMOVED) Width of item in inventory |
| Height `<Operator> <Number>` | (REMOVED) Height of item in inventory |


NOTE:
 * In `Rarity`: Superior < Magic < Rare < Set < Unique
 * In `ItemCat`: Helm, gloves, boots, belt, shield are `Armor`. Ring, amulet, charm, torch are `Accessory`. Gem, jewel, rune are `Socketable`.
 * Throwing axe, knife, javelin, orb, staff, wand are `Weapon`. Throwable potions, arrow and bolt are `Consumable`, not weapon.
 * Quest items and other are `Misc`. Quest weapons like "Khalim's Will" are `Weapon`.
 * `Or` condition will ignore failed result of previous line. `Or` should be used alone in separate line.
 * In ItemMode : Drop (on ground), Equip (on body), Hiring (equiped by mercenary), Belt (potion), Socket (inserted in socket), Vendor (in vendor store)

### Stats

Stats are condition expressions that evaluate to true or false. i.e `"All Resists" > 0 and "Life" > 0` would match items with both all resists and life. A list of keywords that can be used in stats expressions can be found here. [Stats](doc/Stats.md), [Skills](Skills.md). These keywords must be quoted like "..."` or `{...}`. They can contain spaces like `"Amazon +%d to Bow and Crossbow Skills"` or `{Faster Hit Recovery}`.

| Functions | Description |
|-|-|
| `Stat` | Use other stats that are not in [Stats](doc/Stats.md). 2 arg function. The first arg is the stat id from `itemcoststats.txt`. The second arg is optional layer (used for skill stats). Returns the unsigned value of the stat. |
| `ChargedSkill` | Check the skill level of a charged skill. i.e. `ChargedSkill(54) > 0` will check if an item has charges of teleport |
| `ClassSkill` | Check if an item has a certain class skill. i.e. `ClassSkill(1) > 0` will check if an item has +To All Sorc skills. |
| `TabSkill` | Check if an item has a certain tab skill. i.e. `TabSkill(34) > 0` will check if an item has +To to Warcries skills. |
| `Class` | Check if an item is a certain type/class. 1 arg function of the class from `itemtypes.txt`. |
| `Min` | varargs. returns the minimum (non-zero, exclude stats that don't exist) value from a list |
| `MinIn` | varargs. returns the minimum (including zeros, i.e. stats that don't exist on the item) value from a list |
| `Max` | varargs. returns the maximum (non-zero, exclude stats that don't exist) value from a list |

e.x. `Max(Stat(39), Stat(43), Stat(41), Stat(45)) > 0` can be used to filter the existance of any resistance and `MinIn(Stat(39), Stat(43), Stat(41), Stat(45)) > 0` for all resistances

 * Additional numeric (integer) vaiables can used in expression evalution :
   {FilterLevel}, {PingLevel}, {CharacterLevel}, {ItemLevel}, {QualityLevel}, {MagicLevel}, {AffixLevel, {CraftAffixLevel},
   {Sockets}, {Random99}, {Price}, {RuneNumber}, {ItemCode}, {WeaponDamage}, {ItemSize}, {CharacterMaxHP},
   {WeaponDamage} is averge of maxmum and minimum damage of weapon. {Random99} is between 0-99.

### Actions

 * Name and Description can use the following color tokens. `{White}`, `{Red}` , `{Green}`(set item) , `{Blue}`(magic), `{Gold}`(unique), `{Gray}`(socket), `{Black}`, `{Tan}` (dimer {Gold}), `{Orange}`, `{Yellow}`(rare), `{Purple}`, `{DarkGreen}`.
 * Additionl text colors : {MediumRed} (blood red), {MediumGreen} (grass green, lighter than {Green}), {MediumYellow} (dim yellow than rare {Yellow}), {MediumBlue} (turquoise blue-green color), {DarkBlue} (darker than magic {Blue}, hard to read on dark backgound). These additional text color depends on graphic mode (ddraw/d3d, glide, 3dfx(d2dx)) and Diablo version(1.13c/1.14d). These text colors can not be used as pallet color.
 * Addtional pallet colors: {Coral}, {Sage}, {Teal}, {LightGray} may be used in `SetBackgroundColor`, `SetInventoryColor`, `SetBorderColor` as pallet color.
 * Name and Description can use the following special tokens: `{Price}` (item vendor price), `{Sockets}` (number of sockets), {NewLine}, {RuneNumber}, {PotionNumber}, {ItemLevel}, and {ItemType} (readable class(type) name from "Class.md"), {ItemCode} (numerical item type), {AffixLevel}.
 * Note that all color names and tokens have no space in their name.


| Name | Valid Values |
|-|-|
| SetStyle `<Value>` | Sets the styling for an item. A style is a group of actions that will be applied. See [Styles](#Styles). |
| SetName `<Value>` | Sets the name for an item. Special token `{Name}` is the base name for the item. When using continue it will append from the previous condition block. |
| SetDescription `<Value>` | Sets the description for an item.  Special token `{Description}` is the base name for the item. When using continue it will append from the previous condition block. |
| SetBackgroundColor `<Value>` | Sets the background color of the item when on the ground. Pallette index color or White, Red, Green, Blue, Gold, Gray, Black, Tan, Orange, Yellow, Purple, Dark Green |
| SetInventoryColor `<Value>` | Sets the background color of the item when in your inventory. Value is a pallette index color. |
| SetBorderColor `<Value>` | Sets the border color of the item when on the ground. Value is a pallette index color. |
| ChatNotify `<Boolean>` or `<ndigit>` | Notify when the item drops in chat. True or False |
| MinimapIcon `<Value>` | Sets the color of the item on your minimap when on the ground. Value is a pallette index color. |

NOTE:
ChatNotiy: PingLevel may mean either lower bound or higher bound for notification. When ChatNotiy argument is single digit number like `ChatNotify 2`, it is same as `ChatNotify {PingLevel} <= 2`. It will notify when ping level is LOWER than or equal to 2. Low ping level means more frequent notification. When arument is boolean expression like `ChatNotify {PingLevel} > 4`, it will notify when ping level is higer than 4.

### Styles

Lets you apply a group of actions to an item. i.e.

```
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

The first time you join a game with the plugin loaded it should create a `d2lootfilter.ini` settings file in your Diablo II directory. These are the following settings that can be changed.

| Setting | Description |
|-|-|
| Path | Path to your filter file. Default: `{SaveDir}/d2lootfilter-Rules.txt` |
| FilterLevel | (to do) Used to dynamically change how strict your filter while playing. (Currently unused, still a planned feature) Default: `1` |
| PingLevel | Used to dynamically change how strict drop notification are while playing. Default: `1` |

 * Setting ("d2lootfilter.ini") and filetr file will be reloaded for each new game sessions.
 * `Path` may include `{SaveDir}` and `{CharacterName}` token. {SaveDir} is "%USERPROFILE%/Saved Games/Diablo II" dir for Diablo v1.14d version or "./Save" dir under Diablo game install folder for older versions. {CharacterName} is case-sensive name of Diablo game character, played by player.
 * If "d2lootfilter.ini" in game direcory does not exists, d2lootfilter settings can be read from "[D2LootFilter]" section of "BaseMod.ini" or "PlugY.ini".
 * PingLevel may mean either lower bound or higher bound for notification defending on ChatNotiy usage. See NOTE of Action section.


### In Game Commands

The filter has a few in-game commands for changing settings.

| Command | Description |
|-|-|
| `/reload` | Reloads your filter. |
| `/debug` | Toggles debugging. |
| `/test <number>` | Tests a specific rule (by line number) against your currently hovered mouse item. |
| `/filterlevel <number>` | Change the filter level. |
| `/pinglevel <number>` | Change the ping/notification level. |

 * In debug mode, all items hidden by the rules will be displayed in gray color with "*" marker to indicate it will be hidden.


### Credits

* Orignal author of d2lootfiter is Daniel Schumacher. It can be found [here](https://github.com/dschu012/d2lootfilter).
* This version is forked on Nov. 25, 2022 and improved by #qqkookie. Will be merged into dschu012/master. https://github.com/qqkookie/d2lootfilter/tree/Kookie

Special thanks to everyone that has shared their work at [Phrozen-Keep](https://d2mods.info/forum/viewforum.php?f=8) [(Discord)](https://discord.gg/NvfftHY).

To name a few Necrolis, Lectem, Kingpin, whist, Revan, etc...

Thanks to coffin_spirit on Discord for the 1.10f implementation.
