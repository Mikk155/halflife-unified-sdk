# This python scripts does take information from some json files to generate the FGD file.
import json
import os

current_directory = os.path.dirname(os.path.abspath(__file__))

# Set to True if you want to cap some "choices" keyvalues (Note: this will significately increase the FGD size)
ShouldCapChoices = False

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

def PrintClass( Folder, FGD ):

    Path = f'{current_directory}/../entitydata/{Folder}/'
    JsonFiles = sorted(os.listdir(Path))
    for Base in JsonFiles:
        with open( f"{Path}{Base}", 'r') as f:
            data = json.load(f)

            for ClassName, ClassData in data.items():
                FGD.write(f'@{Folder} ')
                if "BaseClass" in ClassData:
                    FGD.write(f'base({ClassData.get("BaseClass", "")}) ')
                if "entcolor" in ClassData:
                    FGD.write(f'color({ClassData.get("entcolor", "")}) ')
                if "entsize" in ClassData:
                    FGD.write(f'size({ClassData.get("entsize", "")}) ')
                if "flags" in ClassData:
                    FGD.write(f'flags({ClassData.get("flags", "")}) ')
                if "studio" in ClassData:
                    FGD.write(f'studio({ClassData.get("studio", "" )}) ')
                if "sprite" in ClassData:
                    FGD.write(f'sprite({ClassData.get("sprite", "" )}) ')
                if "iconsprite" in ClassData:
                    FGD.write(f'iconsprite({ClassData.get("iconsprite", "")}) ')
                FGD.write(f'= {ClassName}')

                if Folder != "BaseClass":
                    Description = ""
                    if "description" in ClassData:
                        Description = FixString( ClassData.get( "description", "" ) )
                    Title = ""
                    if "title" in ClassData:
                        Title = ClassData.get( "title", "" )

                    FGD.write(f' : \"{Title}\" : \"{Description}\"')

                FGD.write(f'\n')
                FGD.write('[\n')

                for ClassKey, ClassData in ClassData.items():
                    if ClassKey in ["BaseClass","entcolor","entsize","flags","sprite","studio","iconsprite" ]:
                        continue

                    KeyType = ""
                    if "type" in ClassData and not isinstance(ClassData, str):
                        KeyType = ClassData.get( "type", "" )
                    elif ClassKey == "spawnflags":
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

                    FGD.write(f'\t{ClassKey}({KeyType}) ')

                    if KeyType == "flags":
                        FGD.write(f'=\n\t[\n')
                        for FlagBit, FlagData in ClassData.items():
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

                        if "default" in ClassData:
                            Default = ClassData.get( "default", "" )
                        if "description" in ClassData:
                            Description = FixString( ClassData.get( "description", "" ) )

                        FGD.write(f': \"{ClassData.get( "title", "" )}\" : \"{Default}\" : \"{Description}\"')

                        if KeyType == "choices":
                            FGD.write(f' =\n\t[\n')

                            if CapChoice:
                                for i in range( int(ClassData.get( "min", 0 )), int(ClassData.get( "max", 1 )) + 1 ):
                                    FGD.write(f'\t\t{str(i)} : ')
                                    FGD.write(f'"{str(i)}"\n')
                            else:
                                Choices = ClassData.get( "choices", {} )

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

                FGD.write(']\n\n')

def ParseKeyValueData(f, key, atributes, data):
    if key in ["entsize", "entcolor", "studio", "iconsprite", "BaseClass", "spawnflags", "sprite" ]:
        return

    f.write(f'<tr><td>{key}</td>')

    default = ""
    if "default" in atributes:
        default = atributes.get("default", "")
    f.write(f'<td>{default}</td>')

    type = ""
    if "type" in atributes and not isinstance(atributes, str):
        type = atributes.get("type", "")
    if type == "cap":
        type = "integer"
    f.write(f'<td>{type}</td>')

    title = ""
    if "title" in atributes:
        title = atributes.get("title", "")
    f.write(f'<td>{title}</td>')

    description = ""
    if "description" in atributes:
        description = atributes.get("description", "")
#    f.write(f'<td>{description}</td></tr>\n')
    f.write(f'<td class="table_description">{description}</td></tr>\n')

    if "BaseClass" in atributes:
        GetBaseClass(f, atributes, data)

    if type == "choices":
        if "choices" in atributes:
            choices = atributes.get("choices", {})
            for choice, args in choices.items():
                f.write(f'<tr><td>{key}</td>')
                f.write(f'<td>{choice}</td>')
                f.write(f'<td></td>')
                f.write(f'<td>{args.get("title", "")}</td>')
                f.write(f'<td class="table_description">{args.get("description", "")}</td></tr>\n')

def GetBaseClass(f, entity_data, data):
    with open( f'{current_directory}/../entitydata/BaseClass/BaseClass.json', 'r') as b:
        BaseClassMain = json.load(b)

    if "BaseClass" in entity_data:
        base = entity_data.get("BaseClass", "")
        bases = base.split(",")

        for dsplit in bases:
            gbase = dsplit.replace(" ", "")
            if gbase in BaseClassMain:
                baseclass = BaseClassMain.get(gbase, {})
                for key, args in baseclass.items():
                    ParseKeyValueData(f, key, args, BaseClassMain)

def PrintEntitiesToWiki( readme, Folder ):

    Path = f'{current_directory}/../entitydata/{Folder}Class/'

    JsonFiles = sorted( os.listdir( Path ) )

    for Base in JsonFiles:
        with open( f"{Path}{Base}", 'r') as f:
            data = json.load(f)

            for entity_name, entity_data in data.items():
                readme.write(f'\t\t<a href="entities/{Folder}/{entity_name}.html">{entity_name}</a><br>\n')

                print(f'{current_directory}/../docs/entities/{Folder}/{entity_name}.html')
                with open(f'{current_directory}/../docs/entities/{Folder}/{entity_name}.html', 'w') as f:

                    f.write('<!DOCTYPE html>\n')
                    f.write('<html lang="en">\n')
                    f.write('<head>\n')
                    f.write('\t<meta charset="UTF-8">\n')
                    f.write('\t<meta name="viewport" content="width=device-width, initial-scale=1.0">\n')
                    f.write(f'\t<title>{entity_name}</title>\n')
                    f.write('\t<link rel="stylesheet" href="../../styles.css">\n')
                    f.write('</head>\n')
                    f.write('</body>\n')

                    f.write(f'\t<h1>{entity_name}</h1>\n')
                    f.write(f'\t<h2>{entity_data.get("name", "")}</h2>\n')
                    f.write(f'\t<p>{entity_data.get("description", "")}</p>\n')
                    f.write('\t<p></p>\n')
                    f.write('\t<h2>KeyValues</h2>\n')
                    f.write(f'\t<table id="{entity_name}" border="1">\n')
                    f.write(f'\t<tr>\n')
                    f.write(f'\t\t<th>Key</th>\n')
                    f.write(f'\t\t<th>Value</th>\n')
                    f.write(f'\t\t<th>Type</th>\n')
                    f.write(f'\t\t<th>FGD</th>\n')
                    f.write(f'\t\t<th>Description</th>\n')
                    f.write(f'\t<tr>\n')

                    for key, args in entity_data.items():
                        if key in ["Class", "name", "description", "notes", "BaseClass"]:
                            continue

                        atributes = entity_data.get(key, {})

                        ParseKeyValueData(f, key, atributes, data)
                        GetBaseClass(f, entity_data, data)

                    f.write(f'</table>\n')

                    if "notes" in entity_data:
                        f.write(f'<h2>Notes</h2>\n')
                        notes = entity_data.get("notes", {})
                        for num, note in notes.items():
                            f.write(f'<p>{note}</p>\n')

                    f.write('</body></html>\n')

def GenerateWikiFile():

    with open(f'{current_directory}/../docs/entities.html', 'w') as readme:
        readme.write('<!DOCTYPE html>\n')
        readme.write('<html lang="en">\n')
        readme.write('<head>\n')
        readme.write('\t<meta charset="UTF-8">\n')
        readme.write('\t<meta name="viewport" content="width=device-width, initial-scale=1.0">\n')
        readme.write('\t<title>Wiki</title>\n')
        readme.write('\t<link rel="stylesheet" href="styles.css">\n')
        readme.write('\t<style>\n')
        readme.write('\t\t.left-column {\n')
        readme.write('\t\t\tfloat: left;\n')
        readme.write('\t\t\twidth: 50%;\n')
        readme.write('\t\t}\n')
        readme.write('\t\t.right-column {\n')
        readme.write('\t\t\tfloat: left;\n')
        readme.write('\t\t\twidth: 50%;\n')
        readme.write('\t\t}\n')
        readme.write('\t</style>\n')
        readme.write('</head>\n')
        readme.write('</body>\n')
        readme.write('<div class="left-column">\n')
        readme.write('\t<h2>Point:</h2>\n')
        PrintEntitiesToWiki( readme, "Point" )
        readme.write('</div>\n')
        readme.write('<div class="right-column">\n')
        readme.write('\t<h2>Solid:</h2>\n')
        PrintEntitiesToWiki( readme, "Solid" )
        readme.write('</div>\n')
        readme.write('</body>\n')
        readme.write('</html>\n')

if __name__ == "__main__":

    FGDFile = f"{current_directory}/../game/dev/limitlesspotential.fgd"

    print(f"File at \"{FGDFile}\"\n")

    GenerateWikiFile()

    with open( FGDFile, 'w') as FGD:
        PrintClass( "BaseClass", FGD )
        PrintClass( "PointClass", FGD )
        PrintClass( "SolidClass", FGD )

