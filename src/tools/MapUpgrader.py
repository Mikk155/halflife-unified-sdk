import os, subprocess

MAPS = []
WHOLEDATA = {}

def env_funnel( Entity ):
    Entity[ "classname" ] = "env_effect_funnel"
    if "spawnflags" in Entity:
        sf = int( Entity[ "spawnflags" ] )
        if sf & 1:
            sf &= 1
            Entity[ "health" ] = "1"
        else:
            Entity[ "health" ] = "0"
        if sf & 2:
            sf &= 2
            sf |= 1
        Entity[ "spawnflags" ] = str(sf)
        if Entity[ "spawnflags" ] == "0":
            Entity.pop( "spawnflags" )
    else:
        Entity[ "health" ] = "0"
    return Entity

def worldspawn( Entity ):
    if "wad" in Entity:
        newwads=""
        wads = Entity[ "wad" ].split( ";" )
        for wad in wads:
            wa = wad.split( '\\' )
            for w in wa:
                if w.endswith( ".wad" ):
                    newwads = f'{newwads}{w};'
        Entity[ "wad" ] = newwads
    return Entity

def DecompileMaps():

    for file in os.listdir( "../maps/" ):
        if file.endswith(".bsp"):
            MAPS.append(file)

    if len( MAPS ) == 0:
        Exit("No BSP files were found!")

    for map in MAPS:
        bsp = f'../maps/{map}'
        subprocess.call( [ "ripent.exe", "-export", bsp ] )

        with open( bsp.replace( ".bsp", ".ent" ), "r" ) as ent:
            lines = ent.readlines()

            ReadingEntity = False
            Entity = {}

            for line in lines:
                if line[0] == '{' and not ReadingEntity:
                    ReadingEntity = True
                elif line[0] == '}' and ReadingEntity:
                    pData = []

                    if bsp in WHOLEDATA:
                        pData = WHOLEDATA[ bsp ]

                    ShouldPrint = True

                    if "classname" in Entity:
                        classname = Entity[ "classname" ]

                    if not classname or classname == "worldspawn":
                        worldspawn( Entity )
                    elif classname == "env_funnel":
                        env_funnel( Entity )
                    else:
                        ShouldPrint = False

                    if ShouldPrint:
                        print( f'Fixed {classname}')

                    pData.append( Entity )
                    WHOLEDATA[ bsp ] = pData
                    ReadingEntity = False
                    Entity = {}
                elif ReadingEntity:
                    if line[0] == '"':
                        key = line[ 1 : line.find( '"', 1 ) ]
                        value = line[ line.find( '" "' ) + 3: line.rfind( '"' ) ]
                        Entity[ key ] = value
            ent.close()

def EditMaps():
    for map, ents in WHOLEDATA.items():
        with open( map.replace( ".bsp", ".ent" ), "w" ) as ent:
            for entity in ents:
                if entity:
                    ent.write( '{\n' )
                    for key, value in entity.items():
                        ent.write( f'"{key}" "{value}"\n')
                    ent.write( '}\n' )
            ent.close()

def CompileMaps():
    for map in MAPS:
        bsp = f'../maps/{map}'
        subprocess.call( [ "ripent.exe", "-import", bsp ] )
        #os.remove( bsp.replace( ".bsp", ".ent" ) )

DecompileMaps()
EditMaps()
#CompileMaps()

def Exit( message ):
	print( f'{message}' )
	print( "Exiting..." )