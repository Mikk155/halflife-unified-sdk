import json
import os

wikiflags = {}

ShouldCapChoices = False

current_directory = os.path.dirname(os.path.abspath(__file__))

def FixString( string ):

    index = string.find( "<", 0 )

    while index != -1:

        indexend = string.find( ">", index + 1 )

        if indexend != -1:
            string = string[:index] + string[indexend + 1:]
        else:
            break

        index = string.find( "<", index)

    string = string.replace( '\"', "'")

    return string

def WriteTable( html, Json, BaseClass ):
    for key, args in Json.items():
        if key == "atributes":
            if "base" in args:
                BaseClasses = args.get( "base", "" )
                for NameType in BaseClasses.split(","):
                    NameType = NameType.strip()
                    if NameType in BaseClass:
                        WriteTable( html, BaseClass.get( NameType, {} ), BaseClass )
            continue

        if key == "spawnflags":
            for FlagBit, FlagData in args.items():

                WikiInfo = {}

                title = ""
                if "title" in FlagData:
                    title = FlagData.get( "title", "" )
                WikiInfo[ "title" ] = title

                description = ""
                if "description" in FlagData:
                    description = FlagData.get( "description", "" )
                WikiInfo[ "description" ] = description

                wikiflags[ int(FlagBit) ] = WikiInfo
            continue

        html.write(f'\t\t\t<tr><td>{key}</td>')

        default = ""
        if "default" in args:
            default = args.get("default", "")
        html.write(f'<td>{default}</td>')

        type = ""
        if "type" in args and not isinstance(args, str):
            type = args.get("type", "")
        if type == "cap":
            type = "integer"
        html.write(f'<td>{type}</td>')

        title = ""
        if "title" in args:
            title = args.get("title", "")
        html.write(f'<td>{title}</td>')

        description = ""
        if "description" in args:
            description = args.get("description", "")
        html.write(f'<td class="table_description">{description}</td></tr>\n')

        if type == "choices":
            if "choices" in args:
                choices = args.get("choices", {})
                for choice, args in choices.items():
                    html.write(f'\t\t\t<tr><td>{key}</td>')
                    html.write(f'<td>{choice}</td>')
                    html.write(f'<td></td>')
                    html.write(f'<td>{args.get("title", "")}</td>')
                    html.write(f'<td class="table_description">{args.get("description", "")}</td></tr>\n')

def ParseClass( FGD, Type, Name, ClassData, BaseClass, readme ):
    HTMLFile = f'{current_directory}/../docs/entities/{Type}/{Name}.html'

    if Type != "Base":
        with open( f"{HTMLFile}", 'w') as html:
            wikiflags.clear()
            readme.write(f'\t\t<a href="entities/{Type}/{Name}.html">{Name}</a><br>\n')

            title = ""
            description = ""

            if "atributes" in ClassData:
                atributes = ClassData.get( "atributes", {} )
                if "title" in atributes:
                    title = atributes.get( "title", "" )
                if "description" in atributes:
                    description = atributes.get( "description", "" )
            html.write(f'<!DOCTYPE html>\n<html lang="en">\n<head>\n\t<meta charset="UTF-8">\n\t<meta name="viewport" content="width=device-width, initial-scale=1.0">\n\t<title>{Name}</title>\n\t<link rel="stylesheet" href="../../styles.css">\n</head>\n</body>\n\t<h1>{Name}</h1>\n\t<h2>{title}</h2>\n\t<p>{description}</p>\n\t<p></p>\n' )

    HasBase = False

    if Type != "Base" and len(ClassData) > 1 and Name != "worldspawn":
        ParseClass( FGD, "Base", f'CEnt{Name}', ClassData, BaseClass, readme )
        HasBase = True

    FGD.write( f'@{Type}Class ' )

    if Type != "Base" or not Name.startswith( 'CEnt' ):
        if "atributes" in ClassData:
            atributes = ClassData.get( "atributes", {} )

            if "color" in atributes:
                FGD.write( f'color({atributes.get("color","255 255 255")}) ')

            if "size" in atributes:
                FGD.write( f'size({atributes.get("size","")}) ')

            if "flags" in atributes:
                FGD.write( f'flags({atributes.get("flags","")}) ')

            if "studio" in atributes:
                studio = atributes.get("studio","")
                if studio != "":
                    studio = f"\"{studio}\""
                FGD.write( f'studio({studio}) ')
                if Type != "Base":
                    with open( f"{HTMLFile}", 'a') as html:
                        html.write( f'\t<p>studio model {studio}</p>\n' )

            if "sprite" in atributes:
                sprite = atributes.get("sprite","")
                if sprite != "":
                    sprite = f"\"{sprite}\""
                FGD.write( f'sprite({sprite}) ')
                if Type != "Base":
                    with open( f"{HTMLFile}", 'a') as html:
                        html.write( f'\t<p>sprite model {sprite}</p>\n' )

            if "iconsprite" in atributes:
                FGD.write( f'iconsprite("{atributes.get("iconsprite","")}") ')

            if Name != "worldspawn":
                FGD.write( f'base(' )

                # All entities support this
                FGD.write( f'Mandatory' )

                if HasBase:
                    FGD.write( f', CEnt{Name}' )

                if "base" in atributes:
                    FGD.write( f', {atributes.get("base","")}')

                # All entities support this
                FGD.write( f', appearflags' )

                FGD.write( f') ')

        if Type != "Base":
            with open( f"{HTMLFile}", 'a') as html:
                html.write(f'\t<h2>KeyValues</h2>\n\t<table id="{Name}" border="1">\n\t<tr>\n\t\t<th>Key</th>\n\t\t<th>Value</th>\n\t\t<th>Type</th>\n\t\t<th>FGD</th>\n\t\t<th>Description</th>\n\t\t<tr>\n')

                if Name != "worldspawn":
                    WriteTable( html, BaseClass.get( "Mandatory", {} ), BaseClass )
                if HasBase or Name == "worldspawn":
                    WriteTable( html, ClassData, BaseClass )
                if Name != "worldspawn":
                    WriteTable( html, BaseClass.get( "appearflags", {} ), BaseClass )
                html.write(f'\t\t</tr>\n\t</tr>\n\t</table>\n')

                if Name != "worldspawn":
                    html.write(f'\t<h2>Spawnflags</h2>\n\t<table id="{Name} flags" border="1">\n\t<tr>\n\t\t<th>Bit</th>\n\t\t<th>FGD</th>\n\t\t<th>Description</th>\n\t<tr>\n\t\t<tr>\n')
                    for FlagBit in sorted( wikiflags ):
                        wikiinfo = wikiflags[ FlagBit ]

                        title = ""
                        if "title" in wikiinfo:
                            title = FixString( wikiinfo[ "title" ] )

                        description = ""
                        if "description" in wikiinfo:
                            description = FixString( wikiinfo[ "description" ] )

                        html.write(f'\t\t\t<td>{FlagBit}</td>')
                        html.write(f'<td>{title}</td>')
                        html.write(f'<td class="table_description">{description}</td></tr>\n')

                    html.write(f'\t\t</tr>\n\t</tr>\n\t</table>\n')

                if "atributes" in ClassData:
                    atributes = ClassData.get( "atributes", {} )
                    if "notes" in atributes:
                        html.write(f'<h2>Notes</h2>\n')
                        notes = atributes.get("notes", {})
                        for num, note in notes.items():
                            html.write(f'<p>{note}</p>\n')

                html.write('</body></html>\n')

    FGD.write(f'= {Name}')

    if Type != "Base":
        if "atributes" in ClassData:
            atributes = ClassData.get( "atributes", {} )

            Description = ""
            if "description" in atributes:
                Description = FixString( atributes.get( "description", "" ) )

            Title = ""
            if "title" in atributes:
                Title = atributes.get( "title", "" )

            if Name != "worldspawn":
                FGD.write(f' : \"{Title}\" : \"{Description}\" []')
            else:
                FGD.write(f' : \"{Title}\"')
        if Name != "worldspawn":
            FGD.write(f'\n\n')
            return

    FGD.write(f'\n[\n')

    for Key, KeyData in ClassData.items():
        if Key == "atributes":
            continue

        KeyType = ""
        if "type" in KeyData and not isinstance(KeyData, str):
            KeyType = KeyData.get( "type", "" )
        elif Key == "spawnflags":
            KeyType = "flags"

        if KeyType == "":
            continue

        CapChoice = False

        if KeyType == "cap":
            if ShouldCapChoices:
                KeyType = "choices"
                CapChoice = True
            else:
                KeyType = "integer"

        FGD.write(f'\t{Key}({KeyType}) ')

        if KeyType == "flags":
            FGD.write(f'=\n\t[\n')
            for FlagBit, FlagData in KeyData.items():
                Default = 0
                Description = ""
                if "default" in FlagData:
                    Default = int(FlagData.get( "default", 0 ))
                if "description" in FlagData:
                    Description = FixString( FlagData.get( "description", "" ) )
                FGD.write(f'\t\t{FlagBit} : \"{FlagData.get( "title", "" )}\" : {Default} : \"{Description}\"\n')
            FGD.write(f'\t]\n')
            continue
        else:
            Default = ""
            Description = ""

            if "default" in KeyData:
                Default = KeyData.get( "default", "" )
            if "description" in KeyData:
                Description = FixString( KeyData.get( "description", "" ) )

            FGD.write(f': \"{KeyData.get( "title", "" )}\" : \"{Default}\" : \"{Description}\"')

            if KeyType == "choices":
                FGD.write(f' =\n\t[\n')

                if CapChoice:
                    for i in range( int(KeyData.get( "min", 0 )), int(KeyData.get( "max", 1 )) + 1 ):
                        FGD.write(f'\t\t{str(i)} : ')
                        FGD.write(f'"{str(i)}"\n')
                else:
                    Choices = KeyData.get( "choices", {} )

                    for Choice, ChoiceData in Choices.items():
                        Title = ""
                        Description = ""
                        if "description" in ChoiceData:
                            Description = FixString( ChoiceData.get( "description", "" ) )
                        if "title" in ChoiceData:
                            Title = ChoiceData.get( "title", "" )
                        FGD.write(f'\t\t\"{Choice}\" : \"{Title}\" : \"{Description}\"\n')

                FGD.write(f'\t]\n')
            else:
                FGD.write(f'\n')

    FGD.write(f']\n\n')

def ParseType( Type, FGD, readme ):
    Path = f'{current_directory}/../entitydata/{Type}Class/'
    JsonFiles = sorted(os.listdir(Path))

    for Base in JsonFiles:
        Json = f'{Path}{Base}'
        with open( f"{Json}", 'r') as js:
            data = json.load(js)
            for Name, ClassData in data.items():
                if Name == "infodecal":
                    FGD.write(f'@PointClass decal() base(Mandatory, Appearflags) = infodecal : "Decal"\n[\n\ttexture(decal)\n]\n\n')
                    continue
                ParseClass( FGD, Type, Name, ClassData, BaseClass, readme )

if __name__ == "__main__":

    FGDFile = f"{current_directory}/../game/dev/limitlesspotential.fgd"
    BaseClassFile = f"{current_directory}/../entitydata/BaseClass.json"
    DocMainFile = f"{current_directory}/../docs/entities.html"

    with open( FGDFile, 'w') as FGD, open( BaseClassFile, 'r') as BaseClassF, open( DocMainFile, 'w') as readme:

        readme.write('<!DOCTYPE html>\n<html lang="en">\n<head>\n\t<meta charset="UTF-8">\n\t<meta name="viewport" content="width=device-width, initial-scale=1.0">\n\t<title>Wiki</title>\n\t<link rel="stylesheet" href="styles.css">\n\t<style>\n\t\t.left-column {\n\t\t\tfloat: left;\n\t\t\twidth: 50%;\n\t\t}\n\t\t.right-column {\n\t\t\tfloat: left;\n\t\t\twidth: 50%;\n\t\t}\n\t</style>\n</head>\n</body>\n<div><a href="entities_update.html">Updating Entity wiki</a><div>\n<div class="left-column">\n\t<h2>Point:</h2>\n')

        BaseClass = json.load( BaseClassF )

        for Name, ClassData in BaseClass.items():
            if Name == "END":
                continue

            ParseClass( FGD, "Base", Name, ClassData, BaseClass, readme )

        ParseType( "Point", FGD, readme )
        readme.write('</div>\n<div class="right-column">\n\t<h2>Solid:</h2>\n')
        ParseType( "Solid", FGD, readme )
        readme.write('</div>\n</body>\n</html>\n')
