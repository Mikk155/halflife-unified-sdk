import os, requests, shutil, sys, zipfile
from github import Github, GithubException

user = 'Mikk155'
repository = 'halflife-unified-sdk'

url = f"https://github.com/{user}/{repository}/archive/game-assets.zip"

response = requests.get( url, stream = True )

abs = os.path.abspath( '' )

with open( f'{abs}/game-assets.zip', 'wb') as z:

    shutil.copyfileobj( response.raw, z )

with zipfile.ZipFile( f'{abs}/game-assets.zip', 'r') as z:

    z.extractall( f'{abs}/game-assets' )
    z.close()

os.rename( f'{abs}/game-assets/halflife-unified-sdk-game-assets/', f'{abs}/game-assets/lp' )

os.remove( f'{abs}/game-assets.zip' )

with zipfile.ZipFile( f'{abs}/game-assets.zip', 'w', zipfile.ZIP_DEFLATED ) as z:

    for root, _, files in os.walk( 'game-assets' ):

        for file in files:

            absp = os.path.join(root, file)

            relp = os.path.relpath( absp, 'game-assets' )

            z.write( absp, relp )
            dbg = relp[ relp.rfind( '\\' ) + 1 : ]
            print( f'Compressing {dbg}')

access_token = os.getenv( "TOKEN" )

g = Github(access_token)

repo = g.get_repo( f'{user}/{repository}')

tag_name = sys.argv[1]

try:

    release = repo.create_git_release(tag_name, tag_name, f"# {tag_name}" )

    new_body = ""

    changelog = os.path.join( os.path.dirname(__file__), f'../docs/changelog.md' )

    with open( changelog, 'r') as cl:

        for line in cl.readlines():

            new_body = f'{new_body}{line}'

        cl.close()

    if new_body:
        release.update_release(release.title, new_body)

    release.upload_asset(f'{abs}/game-assets.zip', label='game-assets.zip')

    print(f'Updated release for "{tag_name}"')

except GithubException as e:

    if e.status == 422:

        print(f'Release with tag "{tag_name}" already exists.')

        exit(1)
