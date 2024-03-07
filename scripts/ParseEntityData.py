# Crear entities.html al final cuando todo este completo
    # Ordenar alfabeticamente
# Separar por Class en carpeta point/ y solid/
# mb replace \n -> <br>

import json

def ParseKeyValueData(f, key, atributes, data):
    if key in ["entsize", "entcolor", "studio", "iconsprite", "base", "spawnflags", "sprite" ]:
        return

    f.write(f'<tr><td>{key}</td>')

    default = ""
    if "default" in atributes:
        default = atributes.get("default", "")
    f.write(f'<td>{default}</td>')

    type = ""
    if "type" in atributes:
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

    if "base" in atributes:
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
    if "base" in entity_data:
        base = entity_data.get("base", "")
        bases = base.split(",")

        for dsplit in bases:
            gbase = dsplit.replace(" ", "")
            if gbase in data:
                baseclass = data.get(gbase, {})
                for key, args in baseclass.items():
                    ParseKeyValueData(f, key, args, data)

def GenerateWikiFile():
    json_file = "../entitydata.json"

    with open(json_file, 'r') as f:
        data = json.load(f)

    with open('../docs/entities.html', 'w') as readme:
        readme.write('<!DOCTYPE html>\n')
        readme.write('<html lang="en">\n')
        readme.write('<head>\n')
        readme.write('\t<meta charset="UTF-8">\n')
        readme.write('\t<meta name="viewport" content="width=device-width, initial-scale=1.0">\n')
        readme.write('\t<title>Wiki</title>\n')
        readme.write('\t<link rel="stylesheet" href="styles.css">\n')
        readme.write('</head>\n')

        for entity_name, entity_data in data.items():

            if "Class" not in entity_data:
                continue

            readme.write(f'<a href="entities/{entity_name}.html">{entity_name}</a><br>\n')

            with open(f'../docs/entities/{entity_name}.html', 'w') as f:

                f.write('<!DOCTYPE html>\n')
                f.write('<html lang="en">\n')
                f.write('<head>\n')
                f.write('\t<meta charset="UTF-8">\n')
                f.write('\t<meta name="viewport" content="width=device-width, initial-scale=1.0">\n')
                f.write(f'\t<title>{entity_name}</title>\n')
                f.write('\t<link rel="stylesheet" href="../styles.css">\n')
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
                    if key in ["Class", "name", "description", "notes", "base"]:
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

        readme.write('</body></html>\n')

def GenerateFGDFile():
    json_file = "../entitydata.json"
    output_file = "../game/dev/limitlesspotential.fgd"
    CapIntegers = False

    with open(json_file, 'r') as f:
        data = json.load(f)

    with open(output_file, 'w') as f:
        for entity_name, entity_data in data.items():

            if "Class" in entity_data:
                f.write(f'@{entity_data.get("Class","")}Class ')
            else:
                f.write(f'@BaseClass ')

            if entity_data.get("base",""):
                f.write(f'base( {entity_data.get("base","")} ) ')

            if entity_data.get("iconsprite",""):
                f.write(f'iconsprite( {entity_data.get("iconsprite","")} ) ')

            if entity_data.get("size",""):
                f.write(f'size( {entity_data.get("size","")} ) ')

            if entity_data.get("flags",""):
                f.write(f'flags( {entity_data.get("flags","")} ) ')

            if entity_data.get("color",""):
                f.write(f'color( {entity_data.get("color","")} ) ')

            if entity_data.get( "studio", False ):
                f.write(f'studio() ')
            elif entity_data.get( "sprite", False ):
                f.write(f'sprite() ')

            f.write(f'= {entity_name} ')

            if entity_name == "worldspawn":
                f.write(f': "{entity_data.get("name","")}" : "{entity_data.get("description","")}"')

            if "Class" in entity_data and entity_name != "worldspawn":
                f.write(f' : "{entity_data.get("name","")}" : "{entity_data.get("description","")}" []\n')
                continue

            f.write(f'\n[\n')

            for prop_name, prop_data in entity_data.items():
                if prop_name in [ "notes", "entsize", "entcolor", "Class", "flags", "base", "iconsprite", "studio", "sprite" ]:
                    continue

                if prop_name == "name" and entity_name == "worldspawn":
                    continue

                if prop_name == "spawnflags":
                    f.write(f'\tspawnflags(flags) =\n')
                    f.write(f'\t[\n')

                    flag = entity_data.get( "spawnflags", {} )

                    for flag_bit, flag_data in flag.items():

                        flag_info = flag.get( flag_bit, {} )

                        flag_title = flag_info.get( "title", "" )
                        flag_state = flag_info.get( "default", "0" )
                        flag_description = flag_info.get( "description", "" )
                        f.write(f'\t\t{flag_bit} : ')
                        f.write(f'"{flag_title}" : ')
                        f.write(f'{flag_state} : ')
                        f.write(f'"{flag_description}"\n')
                    f.write(f'\t]\n')

                    continue

                prop_type = ""
                if "type" in prop_data:
                    prop_type = prop_data.get("type", "")

                prop_title = ""
                if "title" in prop_data:
                    prop_title = prop_data.get("title", "")

                prop_default = ""
                if "default" in prop_data:
                    prop_default = prop_data.get("default", "")

                prop_description = ""
                if "description" in prop_data:
                    prop_description = prop_data.get("description", "")
                    if prop_description:
                        index = prop_description.find( "<", 0 )
                        while index != -1:
                            indexend = prop_description.find( ">", index + 1 )
                            if indexend != -1:
                                prop_description = prop_description[:index] + prop_description[indexend + 1:]
                            else:
                                break
                            index = prop_description.find( "<", index)

                if prop_type == "cap":
                    if CapIntegers:
                        min = prop_data.get( "min", '' )
                        max = prop_data.get( "max", '' )

                        if max and min:
                            f.write(f'\t{prop_name}(choices) : "{prop_title}" : "{prop_default}" : "{prop_description}" =\n\t[\n')


                            for i in range( int(min), int(max) + 1 ):
                                f.write(f'\t\t{str(i)} : ')
                                f.write(f'"{str(i)}"\n')
                            f.write(f'\t]\n')

                        continue
                    else:
                        prop_type = "integer"

                f.write(f'\t{prop_name}({prop_type}) : "{prop_title}" : "{prop_default}" : "{prop_description}"')

                if prop_type == "choices":
                    f.write(f' =\n\t[\n')

                    choice_get = prop_data.get(prop_name, {})
                    choice = prop_data.get( "choices", {} )

                    for choice_bit, choice_data in choice.items():

                        choice_info = choice.get( choice_bit, {} )

                        choice_title = choice_info.get( "title", "" )
                        choice_description = choice_info.get( "description", "" )
                        f.write(f'\t\t"{choice_bit}" : ')
                        f.write(f'"{choice_title}" : ')
                        f.write(f'"{choice_description}"\n')
                    f.write(f'\t]\n')

                    continue

                f.write(f'\n')
            f.write(f']\n\n')

if __name__ == "__main__":
    GenerateFGDFile()
    GenerateWikiFile()