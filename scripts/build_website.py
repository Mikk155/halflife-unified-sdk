# I know the same result could be achieved with JavaScript
# What i don't know is JavaScript :p

import os

abs = os.path.abspath( '../src/website/' )

#========================================================#
#====== Loads all the languages from sentences.res ======#
#========================================================#
def load_languages():

    folder = 'sentences/'

    filelist = os.listdir( os.path.abspath( f'{abs}/{folder}' ) )

    for r in filelist:

        fulldir = os.path.join( f'{abs}/{folder}', r )

        with open( fulldir, 'rt' ) as f:
            lines = f.readlines()

            InValue = False
            dictionary = {}
            KeyName = ''
            ValueName = ''

            for line in lines:
                line = line.replace( '\n', '<br>' )
                if not InValue:
                    if line.startswith( '//' ):
                        continue
                    elif line[0] == "{":
                        InValue = True
                    elif line[0] == "\"":
                        KeyName = line[ 1 : line.rfind( "\"" ) ]
                elif InValue:
                    if line[0] == "}": # Done reading, save and close
                        dictionary[ KeyName ] = ValueName
                        KeyName = ''
                        ValueName = ''
                        InValue = False
                        continue
                    ValueName = f'{ValueName}{line}'
            build_pages( dictionary, r[ 0 : r.find('.res')] )
            f.close()

#========================================================#
#========= Build pages with the given languages =========#
#========================================================#
def build_pages( dictionary, language ):

    dictionary: dict
    language: str

    for dirpath, _, filenames in os.walk( f'{abs}\\' ):

        for filename in filenames:

            if filename.endswith('.html'):

                Old = os.path.join( dirpath, filename )
                NewDirpath = dirpath.replace( '\\src\\website\\', f'\\docs\\{language}\\' )
                New = os.path.join( NewDirpath, filename )

                WriteLines = []

                if not os.path.exists( NewDirpath ):
                    os.makedirs( NewDirpath )

                with open( New, 'w') as w, open( Old, 'rt' ) as r:

                    lines = r.readlines()

                    for line in lines:

                        for key, value in dictionary.items():
                            if line.find( f'#{key}#' ) != -1:
                                line = line.replace( f'#{key}#', value )
                        WriteLines.append( line )

                    w.writelines( WriteLines )
                    r.close()
                    w.close()

load_languages()
