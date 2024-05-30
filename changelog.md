# About

This page contains the full changelog for this fork only. also this apply to the main branch only as well.

The other branches are almost experiments of mine and probably not all of them will be merged in here.

To see this SDK full changelog (upstream repository) See [FULL_UPDATED_CHANGELOG](FULL_UPDATED_CHANGELOG.md)

This file will be updated from above to bellow, meaning besides from this text, when you scroll down you will see the older changelogs.

# CHANGELOG

---
<!-- Start of 2024 -->
<details><summary>2024</summary><p>

<!-- Start of May -->
<details><summary>5 (May)</summary><p>

<details><summary>30</summary><p>

- Split entities based on CTempEntity into env_effect/

- Inherit CTempEntity from CBaseDelay

- Remove env_funnel from the game code

- New CTempEntity based entities
    - env_effect_funnel
    - env_effect_light
    - env_effect_smoke
    - env_effect_sparks
    - env_effect_tarexplosion
    - env_effect_tracer

- Created a python script for "converting" entities from other mods into my version of them in this SDK
- Added visual FX as a placeholder for the longjump module

---

</p></details>

<details><summary>20</summary><p>

- Implement BaseClass CTempEntity

- New entity env_effect_tarexplosion wich uses the class CTempEntity to display TE_TAREXPLOSION

---

</p></details>

<details><summary>11</summary><p>

- Implement Appearance Flags
These are KeyValues supported on all entities that will deny the DispatchSpawn function if appearflag is matched

Appearance flags keys has 3 possible values
- -1
    - "Not in" Meaning this entity won't spawn if matched the condition
- 1
    - "Only in" Meaning this entity will spawn only if matched the condition
- 0
    - "Default" Has no effect at all

i.e a entity with ``appearflag_singleplayer`` > -1 will only appear if this is not a single player match

on the other side a entity with ``appearflag_singleplayer`` > 1 will only appear in a single player match

---

</p></details>

<details><summary>10</summary><p>

- Implement new entity logic_usevalue, Will fire it's target/netname whatever the flValue it received on trigger is the same as the specified. This entity will make more sense and will be used on advanced logics in a future.

- New events on trigger_eventhandler
    - 100 MonsterKilled
    - 101 MonsterTakeDamage

- Fixed some definitions on the FGD

- Implement new entity trigger_eventhandler. Will fire it's target depending on the catch-type event set

- Implement back and side long jumping inspired by Black Mesa with skills to modify
    - longjump_falldamage
        - 0 do not take fall damage if longjump is equiped
        - 1 always take fall damage
    - longjump_fallvelocity
        - Z velocity to apply to the player when he touch the floor
    - longjump_sound
        - 0 do not any noise
        - 1 do player/pl_long_jump.wav

---

</p></details>

<details><summary>05</summary><p>

- Add bool CBaseEntity::IsMonster() function that will return true for classes that inherits from CBaseMonster that are not:
    - monstermaker
    - scripted_sentence
    - env_explosion
    - cycler
    - op4mortar
    - func_guntarget
    - player
    - nodes

- trigger_endsection will now finish a multiplayer game as a game_end it was

- CBaseTrigger now forces retouch for stationary monsters

- Show chapter title for all players upon spawning, only apply to multiplayer

- Remove game censure on German language

- "Use Angles" for func_wall/wall_toggle/illusionary

- Allow FL_GODMODE on monsters. it is written on TakeDamage so TraceAttack effects will still apply (throw blood, sparks etc)

- Add a bunch of new skill variables:
    - barnacle_health
    - barnacle_speed
        - controlls the speed at wich a barnacle can lift monsters
    - player_health
    - player_armor
    - babycrab_dmg_bite
    - babycrab_health
    - houndeye_squad_bonus
        - multiplier bonus for squad, originally at 1.1
    - islave_speed_zap
        - pev->framerate, 1.0 is normal speed
    - barney_armor_chest
        - divide the damage get on chest for this value, originaly 2
    - barney_armor_helmet
        - reduce the damage get on the helmet for this value, originaly 20
    - hassassin_invisibility
        - 0 = never invisible
        - 1 = semi visible
        - 2 = invisible when out of combat
        - 3 = always semi visible between 50 and 255 amt
    - apache_rockets
        - 0 disable rockets
    - hgrunt_next_glauncher
        - on non-easy, this is the max time interval at wich the grunts will launch another grenade
        - on easy this is a fixed interval
    - turret_max_range
    - turret_turn_rate
    - turret_active_time
    - turret_takedamage_off
        - 1 = the turret won't get damage while it's inactive
    - sentry_takedamage_off
        - 1 = the sentry won't get damage while it's inactive
    - pitdrone_speed_spit
    - shocktrooper_spore_dmg
    - op4mortar_damage
    - plr_hornet_regen
        - rate of regen for hornet gun
        - -1 = never regen

- now grunt and shoocktrooper repel will fire it's target after spawning the child, pActivator will be the child spawned

- monster_barnacle won't die by a single hit of a crowbar

- game_text will now print the messages on the console
    - added spawnflag "No echo console" for preventing this

- trigger_auto now has a spawnflag "Wait for clients" that will prevent the entity from triggering it's target until at least one player is connected

- trigger_relay now has a spawnflag for "Keep activator" forwarding it's activator instead of overriding with the trigger_relay entity

- env_render now supports !activator and !caller for target

- defined ``SND_PRIVATE`` on ServerSoundSystem for sending the audio to only one target. meant for HEV sentences in a future

- added two experimental entities player_command and player_percent, these may change in the future.

- game_text now can print all the supported message types

- game_text new spawnflag "Fire Per Player" that will fire it's target for every player that sees the message

- env_funnel new spawnflag "Repeatable" and "model" keyvalue for custom sprite

- trigger_hurt now doesn't reproduce geiger noises when it is off

- Added std::string Vector::ToString()

- Merged branch [FireTargets-features](https://github.com/Mikk155/halflife-unified-sdk/tree/FireTargets-features)

- Merged branch [monster-free-roaming](https://github.com/Mikk155/halflife-unified-sdk/tree/monster-free-roaming)
    - All credits goes to [FreeSlave](https://github.com/FreeSlave)

- Created a [script](scripts/forge_game_data.py) for building a FGD file from json information

- Created [BaseClass.json](src/fgd/en/EntityData/BaseClass.json) containing all the @BaseClass

- Created a script for porting FGD to json formats that we're going to use later on [forge_game_data.py](scripts/forge_game_data.py)

<details><summary>Script in case of re-usage</summary><p>

```python
import json, os

with open( 'halflife-unified.fgd', 'r') as fgd:
    lineas = fgd.readlines()

ReadingEnt = False
InChoices = False
InSpawnflags = False

datos_json = {}
KeyValueField = {}
Choices = {}

for line in lineas:

    if line.startswith( '@' ):
        ReadingEnt = True
        ClassData = {}

        Class = line[ 1: line.find( 'Class' ) ]

        if Class == 'Point':
            ClassData[ 'type' ] = 'Point'

        Bases = [ 'Mandatory' ]
        if line.find( 'base(' ) != -1:
            bases = line[ line.find( 'base(' ) + 5: line.find( ')', line.find( 'base(' ) ) ].split( ',' )
            for base in bases:
                if base != 'Targetname' and base != 'Appearflags':
                    base = base.strip()
                    Bases.append( base )
        Bases.append( 'Appearflags' )

        if line.find( 'size(' ) != -1:
            Size = []
            size = line[ line.find( 'size(' ) + 5: line.find( ')', line.find( 'size(' ) + 5 ) ].split( ',' )
            for siz in size:
                Size.append( siz.split() )
            ClassData[ 'size' ] = Size

        if line.find( 'color(' ) != -1:
            Color = line[ line.find( 'color(' ) + 6: line.find( ')', line.find( 'color(' ) + 6) ].split( ' ' )
            ClassData[ 'color' ] = Color

        if line.find( 'studio(' ) != -1:
            Studio = line[ line.find( 'studio(' ) + 7: line.find( ')', line.find( 'studio(' ) + 7 ) ]
            ClassData[ 'studio' ] = Studio.strip('"')

        if line.find( 'sprite(' ) != -1:
            Sprite = line[ line.find( 'sprite(' ) + 7: line.find( ')', line.find( 'sprite(' ) + 7 ) ]
            ClassData[ 'sprite' ] = Sprite.strip('"')

        if line.find( 'flags(' ) != -1:
            flags = line[ line.find( 'flags(' ) + 6: line.find( ')', line.find( 'flags(' ) + 6 ) ]
            ClassData[ 'flags' ] = flags.split(',')

        ClassData[ 'base' ] = Bases

        if line.find( '= ' ) != -1:
            ClassName = line[ line.find( '= ' ) + 2: line.find( ':', line.find( '= ' ) + 3 ) ].strip()
            ClassData[ 'classname' ] = ClassName

        if line.find( ': "' ) != -1:
            Name = line[ line.find( ': "' ) + 3: line.find( '"', line.find( ': "' ) + 3 ) ]
            ClassData[ 'name' ] = Name

        ClassData[ 'description' ] = ''

        datos_json[ 'ClassData' ] = ClassData

    elif InSpawnflags:
        dline = line.strip()
        if dline and dline != '' and dline != '[':

            if dline[0] == ']':
                KeyValueField = Choices
                datos_json[ Key ] = KeyValueField
                KeyValueField = {}
                Choices = {}
                InSpawnflags = False
            else:
                ToChoices = {
                    "name": dline[ dline.find( ':' ) + 1 : dline.rfind( '"' ) ].strip().strip( '"' ),
                    "description": ""
                }
                ToAllChoices = Choices.get( 'spawnflags', {} )
                ToAllChoices[ dline[ : dline.find( ':' ) ].strip() ] = ToChoices
                Choices= ToAllChoices

    elif InChoices:
        dline = line.strip()
        if dline and dline != '' and dline != '[':

            if dline[0] == ']':
                KeyValueField= Choices
                datos_json[ Key ] = KeyValueField
                KeyValueField = {}
                Choices = {}
                InChoices = False
            else:
                ToChoices = {
                    "name": dline[ dline.find( ':' ) + 1 : ].strip().strip( '"' ),
                    "description": ""
                }
                ToAllChoices = Choices.get( 'choices', {} )
                ToAllChoices[ dline[ : dline.find( ':' ) ].strip() ] = ToChoices
                Choices[ 'choices' ] = ToAllChoices

    elif ReadingEnt and line[0] == ']':
        path = os.path.abspath( datos_json.get( 'ClassData', {} )[ 'classname' ] + '.json' )
        with open( path, 'w') as js:
            js.write(json.dumps(datos_json, indent=4))
        datos_json = {}
        ReadingEnt = False
    else:
        dline = line.strip()

        if dline.find( '(' ) != -1:
            Key = dline[ 0 : dline.find( '(' ) ]
            if Key == 'spawnflags':
                InSpawnflags = True
                Choices = KeyValueField
            elif Key and Key != '':

                Type = dline[ dline.find( '(' ) + 1 : dline.find( ')' ) ].lower().strip()
                KeyValueField[ 'type' ] = Type

                Name = dline[ dline.find( '"' ) + 1 : dline.find( '"', dline.find( '"' ) + 1 ) ]
                KeyValueField[ 'name' ] = Name
                KeyValueField[ 'description' ] = ''

                Value = ''
                if Type == 'choices':
                    Value = dline[ dline.find( ':', dline.find( '"' ) ) + 1 : dline.find( '=' ) ].strip().strip( '"' )
                    InChoices = True
                    KeyValueField[ 'value' ] = Value
                    Choices = KeyValueField
                elif dline.find( ':', dline.find( '"' ) ) != -1:
                    Value = dline[ dline.find( ':', dline.find( '"' ) ) + 1 :  ].strip().strip( '"' )
                    KeyValueField[ 'value' ] = Value
                    datos_json[ Key ] = KeyValueField
                    KeyValueField = {}
    ClassData = {}
```

</p></details>

---

</p></details>

<details><summary>04</summary><p>

- Disable "Wiki" on the repository

- Enable "Issues" on the repository

---

</p></details>

<details><summary>01</summary><p>

- Create a [script](scripts/install_game_assets.py) for releasing game assets from a branch [game-assets](https://github.com/Mikk155/halflife-unified-sdk/tree/game-assets)

- Replace workflow rules to only run when the yml is modified (version update)

---

</p></details>

<!-- End of May -->
</p></details>

<!-- Start of April -->
<details><summary>4 (April)</summary><p>

<details><summary>30</summary><p>

- Added base web site to this repository

- Created a [script](scripts/build_website.py) for building website by the use of multiple languages

- Create a workflow rules to auto release compiled binaries upon a commit to src/

---

</p></details>

<!-- End of April -->
</p></details>

<!-- End of 2024 -->
</p></details>
