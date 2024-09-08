- [Getting Started](#getting-started)
- [Type Overview](#type-overview)
  - [Adaptive Values](#adaptive-values)
  - [Functional Values](#functional-values)
  - [Exclusive Values](#exclusive-values)
  - [Routines](#routines)
  - [Properties](#properties)
  - [Include Lists](#include-lists)
- [Properties](#properties-1)
- [Members](#members)
  - [Shared members among Extra Values and Routines](#shared-members-among-extra-values-and-routines)
  - [Shared members among Extra Values](#shared-members-among-extra-values)
  - [Adaptive Values](#adaptive-values-1)
  - [Functional Values](#functional-values-1)
  - [Exclusive Values](#exclusive-values-1)
  - [Include Lists](#include-lists-1)
  - [Routines](#routines-1)
- [Formulas](#formulas)
  - [Calling on Routines](#calling-on-routines)
  - [Using arguments](#using-arguments)
- [Commons](#commons)
- [Examples](#examples)



# Getting Started


Before Extra Values (new actor values) or extra functionality is covered, first let's go over where to put your configurations, what to name them, and what general contents should go inside.

All AVG config files must be placed within the skyrim path, "Data/SKSE/Plugin/ActorValueData". In addition it must have "_AVG" as a suffix to its name, and the file type must be ".toml". For more information about the general setup of a toml file, you can read about its specific documentation here: "https://toml.io/en/v1.0.0". In this document for brevity's sake I'll be moving forward with the assumption that you know the types pertaining to toml, as well as how to create them. If you're unsure how to, please refer to the toml documentation aforementioned.

And a reminder that barring certain vanilla circumstances, this adheres to toml casing rules, meaning most things are case sensitive.

A reserved name for config files is named "Commons", as that is where base AVG implemented structures will soon go.



# Type Overview


There are currently 6 types of core data you can create: An Adaptive Value, a Functional Value, an Exclusive Value, a Routine, Properties, and an Include List.


## Adaptive Values
  Extra Values that bear the closest resemblance to Actor Values. Each actor has their own store of adaptive values that can be manipulated as you would any other actor value, and with the same level of persistence.

## Functional Values
 Extra Values that do not exist relative to an actor. Instead, they have get functions that can be used to determine their value when called for. If one has an SKSE plugin they'd like to have people communicate with through that isn't a set value, this may be what they'd be looking for.

## Exclusive Values 
 ~~Mix between Adaptive and Functional values. Stores data for the player exclusively, and functions like a regular Functional Value for anyone else.~~ (Currently not implemented)

## Routines 
 Functions that you create within this space to reuse code, and simplify the usage of lengthy formulas, more on routines and specifically formulas later.

## Properties
global objects that one can create as constants to refer to within code. (Currently, they are the only way to use forms.)

## Include Lists
 Apply Alias Values to plugins that weren't ported with the original extra value. It additionally allows for the use of keywords instead of plugins, preventing overlap and allowing specific forms to have different aliases.


All of the above types are created using tables, however properties' members, or entries associated with the main type, are free to have whatever name they please. While on the other hand all Extra Value and Routine declarations only use specificly named members, but can have nearly any names they'd like.



# Properties


Generally, the syntax for declaring a property is as follows: 

`param_name = "ParamType: DefaultValue"`

All properties only take strings as their toml value as of right now. When declaring the default value for a property, you can currently only use fixed values ('string' or 2 for example).

Properties are composed of 3 types, objects (or this plugin forms. You can use editorId's as well), numbers (which are always floats unless an operator casts it), and strings. These are valid ways to declare them.

```
number_property = "Number: 1"
string_property = "String: test string"
object_property1 = "Object: 0x39"
object_property2 = "Object: Skyrim.esm::0x39"
object_property3 = "Object: DaysPassed"
```

As one can see, you can specify what plugin an object comes from, allowing you to pull an object from a specific plugin.

For EditorIDs, make sure you are using an object that retains it's EditorID like quests, keywords, and globals.



# Members

Each type generally has its own designated members (aside from properties), but there is some overlap. Namely speaking, the type name. Member's requirements are expressed below as the following:

`member_name[expected_toml_types](* if optional, ** if optional BUT) = description`



## Shared members among Extra Values and Routines

- <u>\[\<TypeName>]</u>	First you'd start by declaring the type's name like so. The type name has to be unique, among Extra values and routines declared.

- <u>type(string)</u> = The type to be expected. In this set up, where type name is declared as a table, only "Routine", "Functional", "Adaptive", and "Exclusive" are acceptable inputs.



## Shared members among Extra Values


- <u>alias(string)</u> = The Actor Value this extra value is aliasing. Uses of that AV will be replaced with this EV in its specified plugins and its dependent plugins.

- <u>plugins(array of strings)</u> = Specifies the plugins that use this Extra Value. As mentioned, plugins that inherit from it will also inherit its aliases.

Both aliases and plugins are optional. Without them however, you need to use an include list.


## Adaptive Values

- <u>regen.rate(string)**</u> = Controls by how much one recovers their health per frame. ** optional if not using regen, if so this must be defined.
- <u>regen.delay(string)*</u> = Controls how long it takes for the actor value to start recovering. It takes a formula (more on these later).
- <u>regen.fixed(bool)*</u> = Controls whether "regen.rate" is a percent or a fixed value that affects damage.

- <u>default.formula(string)*</u> = Controls the base value of the Extra Value associated.
- <u>default.type(string)*</u> = Controls how long the base value remains equal to the formula. "Implicit" defaults are only equal on creation, "Explicit" defaults will only become unequal when manually set, and "Constant" defaults are never unequal to the formula. "default.type" is "Implicit" if not stated.


## Functional Values


- <u>get.etc..(string)**</u> = 	"base", "permanent", "temporary", and "damage" are all get formulas that determine the value of each modifier specified. **All are optional, 
but at least one being specified is required. Setting get to a value is the equivalent of setting it to base.

- <u>set.etc..(string)**</u> = same submembers as get ("base", "permanent", "temporary", "damage"), but instead the expected string is the name of a registered function (In the API). Similar to get, setting set to a string is the equivalent of setting it to base.

*A note for SKSE developers, It's important to note that while a get or set function is optional, a Functional Value must have the required set functions to be used in a magic effect, and must have both a get and set to use ModActorValue on it's respective modifier.


## Exclusive Values

- ~~Exclusive values, by their nature will encorporate the members of both adaptive and functional values.~~ (Not Implemented)


## Include Lists


Similar to properties, there no set members of include, and instead you place it however you wish. The syntax for declaring an include entry goes as such:

`"plugin/keyword name" = "ExtraValueName"`

It is worth noting, that whenever you are declaring a plugin name as the thing you're trying to include to, the left hand side of the '=' must always be encased in quotes (otherwise, it would mistake it for a table entry). You don't have to put the quotes if it's a keyword (as the .es_ determine if it's plugin or not) but it is harmless to do so.

There are additional settings that allow one to change what alias is being used on the respective plugin, such things would look like this:

`"plugin/keyword name" = "ExampleAV=ExampleEV"`

In practice, these would look like this:

```
"AVGTest.esp" = "TestEV"
"AVGTest.esp" = "Health=LevelValue"
```

\* It is additionally notable that if Keyword Item Distributor is installed, it will wait for keyword distribution before handling the include list.\*


## Routines


- params(array of strings)* = These are the parameters that can be used within the routine. These strings adhere to property declaration rules with a few caveats.
  - First, the default value is optional. 
  - Second if default value is used, the following parameters are not allowed to be non-default.
  
- formula (string) = This string evaluates the resulting number of the routine. For more information see "Formulas" below.

- \<!> Currently there are a few different native routines, more pending. You can find a list of them in the Functions text included.



# Formulas


The thing that routines use as their code, adaptive EVs use as their regen values, and functional EVs used as their get values, Formulas are strings that look similar to their name sake in arithmetic. For instance, "(1 * 4) / 5". Currently, the following symbols are implemented, the ones that cast to integer marked with <c>:

- (): Parentheses
- %: remainder/modulo \<c>
- ^: Exponent
- \*: Multiplication
- +: Addition
- -: Subtraction, but also negates CONSTANT numbers (currently cannot negate the results of a function, nor a property. Bit of an oversight really).
- <: Less Than
- \>: Greater Than
- <=: Less Than or Equal Tp
- \>=: Greater Than or Equal To
- ==: Equal To
- !=: Not Equal To
- &&: Logical AND
- ||: Logical OR
- <<: Left shift operator \<c>
- \>>: Right shift operator \<c>
- ^^: bitwise XOR operator
- &: Bitwise and operator
- |: bitwise or operator

These operators largely adhere to the order of operations found in C++, documented here https://en.cppreference.com/w/cpp/language/operator_precedence



## Calling on Routines


Calling on routines is similar to how you'd do it in papyrus. C++, or really most languages that I'd know of. Something like: 

`routine_name(arg1, arg2, arg3)`

- \<!> Rules largely to adhere to in function calling. 
	- The call's name and the "(" MUST be next to each other, no white spaces, otherwise it will view it as a property.
	- You can use arithmetic, even call functions within each routine's argument, just make sure that parameter expects a number. 
		Ex. routine_name(routine_name2('String') * number_property)
  
   


## Using arguments


The 3 argument types are the same as the 3 property types, String, Number and Object. Currently, there is no literal way to declare an object, one must use a property. For numbers, literals like "1", "2.0", or "3.14" will work. For strings DON'T use the " (quote) character, instead use ' (apostrophe), like 'this'. Other than that, you can use properties, parameters, and routines within them, so long as the types match.



# Commons


There is one last major thing pertaining to all of these different types that needs to be addressed and that is Commons. All AVG implemented internal functions (GetItemCount, GetActorValue, etc) are included in the commons, but also common properties or functional values that users may make. Properties such as the player, gold, Level as an actor value, etc. The commons will be updated with time and requests so to get a feel for it, best to check the file out for oneself.



# Examples


After that wall of text, the best way to end this off will be to see these types in action. So past this point, treat this as if it were an actual config file to see the proper format.

\#######################################################################################


\[Properties]\
"true" = "Number: 1"\
"false" = "Number: 0"

Base = "Number: 1"\
Permanent = "Number: 2"\
Temporary = "Number: 4"\
Damage = "Number: 8"\
Maximum = "Number: 7"\
All = "Number: 15"

twoHandedGlobal = "Object: AVGTest.esp::0x810"\
minorAccumen = "Object: AVGTest.esp::0x814"\
majorAccumen = "Object: AVGTest.esp::0x815"



\[HitsTaken]\
type = "Adaptive"\
alias = "Variable01"\
regen.delay = "GetActorValue('Health', All) / GetActorValue('Health', Maximum)"\
regen.rate = "GetActorValue('HealRate', All)"\
regen.fixed = false



\[TestEV]\
type = "Functional"\
alias = "Variable02"

get.base = "1"\
get.permanent = "2"\
get.temporary = "2"\
get.damage = "-1 * 2 ^ 2"

set.damage = [ "AffectActorValue", "Health" ]



\[TwoHandedPerks]\
type = "Functional"\
alias = "Variable03"\
get = "GetAccumen('TwoHanded', twoHandedGlobal)"


\[FireDamage]\
type = "Adaptive"\
alias = "Stamina"


\[GetAccumen]\
type = "Routine"

params = ["actor_value = String", "fixed_global = Object" ]\
formula = "HasPerks(actor_value) + (GetActorValue(actor_value, Base) >= GetGlobalValueParam(minorAccumen)) + (GetActorValue(actor_value, Maximum) >= GetGlobalValueParam(majorAccumen)) + GetGlobalValueParam(fixed_global)"

\[Include]\
"AVGTest.esp" = ["HitsTaken", "TestEV", "TwoHandedPerks"]\
MagicDamageFire = [ "FireDamage = Health" ]\
WeapTypeSword = [ "HitsTaken = None" ]
