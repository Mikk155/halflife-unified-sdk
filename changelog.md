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

<details><summary>05</summary><p>

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
