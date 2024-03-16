import os
import json
import shutil
import subprocess

data = ""

while True:

    print( f'Note: You must install the original mod\'s resources within \"limitlesspotential/mods/assets/\" before using this tool.' )

    package = input( f'Write the name of the .json installer package: ' )

    if os.path.exists( f'{package}.json' ):
        with open( f'{package}.json', 'r' ) as load:
            data = json.load( load )
        break
    else:
        print( f'\"limitlesspotential/mods/{package}.json\" not found.' )

if not data:
    exit(0)

input( f'Press enter if you\'ve propertly installed the assets from {data.get( "url", "" ) }' )

ripent = os.path.join( os.path.dirname( __file__ ), 'tools/ripent.exe' )

if "ripent" in data:
    r = data.get( "ripent", {} )
    if "import" in r:
        i = r.get( "import", {} )
        for d, m in i.items():
            if os.path.exists( f'assets/maps/{d}.ent' ):
                shutil.copy( f'assets/maps/{d}.ent', f'mod/maps/{d}.ent' )
                arguments = [ '-import', f'mod/maps/{d}.ent' ]
                command = [ripent] + arguments
                process = subprocess.Popen(command)
                process.wait()

if "resources" in data:
    r = data.get( "resources", {} )
    for d, m in r.items():
        if os.path.exists( f'mod/{d}' ):
            shutil.move( f'mod/{d}', f'../{d}' )

shutil.rmtree( 'mod/')
if not os.path.exists( 'mod/' ):
    os.makedirs( 'mod/' )