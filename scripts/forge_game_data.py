# This Python script builds our FGD file and our website's Entity Guide page.

import os, json

DEBUG = False

abs = os.path.abspath( '../' )

def WriteStudio( ClassData ):
    std = [ 'studio', 'sprite' ]
    for Studio in std:
        if Studio in ClassData:
            studio = ClassData.get( Studio )
            FGD.write( f'{Studio}(')
            if isinstance( studio, bool ):
                FGD.write( ') ')
            else:
                FGD.write( f' "{studio}" ) ')

def WriteIconSprite( ClassData ):
    if 'iconsprite' in ClassData:
        iconsprite = ClassData.get( 'iconsprite' )
        FGD.write( f'iconsprite( {iconsprite} ) ')

def WriteOffSet( ClassData ):
    if 'offset' in ClassData:
        offset = ClassData.get( 'offset' )
        FGD.write( f'offset( ')
        for i in offset:
            FGD.write( f'{i} ')
        FGD.write( f') ')


def WriteColor( ClassData ):
    if 'color' in ClassData:
        FGD.write( 'color( ')
        for rgb in ClassData.get( 'color', [] ):
            FGD.write(f'{rgb} ')
        FGD.write( ') ')

def WriteSize( ClassData ):
    if 'size' in ClassData:
        FGD.write( 'size(')
        for i, size in enumerate( ClassData.get( 'size', [] ) ):
            for m in size:
                FGD.write(f' {m}')
            if i == 0:
                FGD.write( ',' )
        FGD.write( ' ) ')

def WriteFlags( ClassData ):
    if 'flags' in ClassData:
        FGD.write( 'flags(')
        for i, flags in enumerate( ClassData.get( 'flags', [] ) ):
            FGD.write(f' {flags}')
            if i < len( ClassData.get( 'flags', [] ) ) - 1:
                FGD.write( ',' )
        FGD.write( ' ) ')

def WriteBase( ClassData, IsEntity = False ):
    if 'BaseClass' in ClassData:
        if IsEntity:
            FGD.write( f', ' )
        else:
            FGD.write( f'base( ' )
        base = ClassData.get( 'BaseClass', [] )
        for i, b in enumerate(base):
            FGD.write( f'{b}' )
            if i < len(base) - 1:
                FGD.write( ', ' )
        FGD.write( ' ) ' )
    elif IsEntity:
        FGD.write( ' ) ' )

def WriteSpawnflags( js ):
    if 'spawnflags' in js:
        FGD.write( f'\tspawnflags(Flags) =\n\t[\n' )
        for sk, sv in js.get( 'spawnflags', {} ).items():
            svname = sv.get( 'name', '' )
            svdescription = sv.get( 'description', '' )
            i = int(sk)
            if ( i & ( i - 1 ) ) != 0:
                print( f'Error! Wrong key value bit set, please fix flag "{svname}" bit "{sk}"')
                exit(1)
            FGD.write( f'\t\t{sk} : "{svname}" : ' )
            if 'active' in sv and sv.get( 'active', False ):
                FGD.write( '1' )
            else:
                FGD.write( '0' )
            FGD.write( f' : "{svdescription}"\n' )
        FGD.write('\t]\n')

def WriteEntity( TypeClass, js, BaseClassName = '' ):
    ClassData = js.get( 'ClassData', {} )
    name = ClassData.get( 'name', '' )
    classname = ClassData.get( 'classname', '' )
    description = ClassData.get( 'description', '' )

    if TypeClass == 'Base' or classname == 'worldspawn' or classname == 'infodecal':

        FGD.write( f'@{TypeClass}Class ' )
        WriteColor( ClassData )
        WriteSize( ClassData )
        WriteFlags( ClassData )
        WriteOffSet( ClassData )
        WriteStudio( ClassData )
        WriteIconSprite( ClassData )

        if classname == 'infodecal':
            FGD.write( f'decal() ' )

        if BaseClassName != '':
            WriteBase( ClassData )

        FGD.write( f'= ' )

        if TypeClass == 'Base' and BaseClassName == '':
            FGD.write( 'C' )

        if BaseClassName == '':
            FGD.write( f'{classname} : "{name}" : "{description}"\n[\n' )
        else:
            FGD.write( f'{BaseClassName}\n[\n' )

        WriteSpawnflags( js )

        if classname == 'infodecal':
            FGD.write( f'\ttexture(decal)\n' )

        for key, values in js.items():

            if key == 'choices' or key == 'ClassData' or key == 'spawnflags':
                continue

            type = values.get( 'type', '' )
            name = values.get( 'name', '' )
            value = values.get( 'value', '' )
            description = values.get( 'description', '' )

            # Hack hack, https://developer.valvesoftware.com/wiki/FGD says Hammer die reading quoted integers
            #if type != 'integer':
            value = f'"{value}"'
            # Undone, -Mikk ask in program wich program will you use the FGD for.
            # Hammer will also die if integer is empty ( "name" : : "description" )
            # Meant intentionaly empty (not 0) for specific keyvalues that shouldn't get handled to KeyValue bool.

            FGD.write( f'\t{key}({type}) : "{name}" : {value} : "{description}"' )
            if 'choices' in values:
                FGD.write( ' =\n' )
                choices = values.get( 'choices', {} )
                FGD.write( f'\t[\n' )
                for ck, cv in choices.items():
                    cn = cv.get( 'name', '' )
                    cd = cv.get( 'description', '' )
                    FGD.write( f'\t\t"{ck}" : "{cn}" : "{cd}"\n' )
                FGD.write( f'\t]\n' )
            else:
                FGD.write( '\n' )

        FGD.write( f']\n\n' )
        return

    if len(js) > 1:
        if BaseClassName != 'IsCopyPoint':
            WriteEntity( 'Base', js )
            if DEBUG:
                print(f'{classname}')

    FGD.write( f'@{TypeClass}Class ')
    #WriteColor( ClassData )
    #WriteSize( ClassData )
    #WriteFlags( ClassData )
    #WriteOffSet( ClassData )
    #WriteStudio( ClassData )
    #WriteIconSprite( ClassData )

    if len(js) > 1:
        FGD.write( f'base( Mandatory, C{classname}' )
        if BaseClassName == 'IsCopyPoint':
            FGD.write( f', hullsizes' )
        WriteBase( ClassData, True )
    else:
        WriteBase( ClassData )

    FGD.write( f' = {classname} : "{name}" : "{description}" []\n\n' )

def ReadData( path ):
    with open( path, 'r' ) as f:
        js = json.load( f )
        f.close()

    if js:
        data = js.get( 'ClassData', {} )
        if path.endswith( 'BaseClass.json' ):
            for k, v in js.items():
                WriteEntity( 'Base', v, k )
        elif 'type' in data:
            WriteEntity( data.get( 'type' , '' ), js )
        else:
            WriteEntity( 'Solid', js )
            WriteEntity( 'Point', js, 'IsCopyPoint' )


LangFolders = os.listdir( os.path.abspath( f'{abs}/src/fgd/' ) )

for L in LangFolders:

    with open( f'{abs}/src/fgd/{L}/lp.fgd', 'w' ) as FGD:

        # -Mikk BSPGuy v5 release.
        #FGD.write( '@mapsize( -4096, 4096 )\n\n' )

        JsonLists = os.listdir( os.path.abspath( f'{abs}/src/fgd/{L}/EntityData/' ) )

        ReadData( os.path.abspath( f'{abs}/src/fgd/{L}/EntityData/BaseClass.json' ) )

        for r in JsonLists:

            fulldir = os.path.join( f'{abs}/src/fgd/{L}/EntityData/', r )

            if fulldir.endswith( '.json' ) and not fulldir.endswith( 'BaseClass.json' ):
                print( f'{r}')
                ReadData( fulldir )
        FGD.close()
