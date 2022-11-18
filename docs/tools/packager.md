# Packager

The Packager tool is used to create a zip archive containing a mod installation.

This tool is included in the `hlu/tools` directory in the game installation package available here: https://github.com/SamVanheer/halflife-unified-sdk/releases

## Purpose

When executed this tool creates a zip archive containing a mod installation.
Old packages will be removed automatically on successful completion.

## Prerequisites

### Install the .NET SDK

See [Setting up the .NET SDK](/docs/tutorials/setting-up-dotnet-sdk.md)

## Usage

### Command line

```
dotnet "path/to/HalfLife.UnifiedSdk.Packager.dll"
--mod-directory "path/to/half-life/unified/sdk/mod"
--package-manifest "path/to/PackageManifest.json"
--package-name "PackagePrefix"
[--verbose]
```
The mod directory should point to your local copy of the [mod installation](/INSTALL.md).

The package manifest should point to the package manifest file contained in the game installation.

The package name is a prefix used to create the file name of the package. The format is `PackageName-yyyy-MM-dd-HH-mm-ss.zip`. Note that timestamps use UTC time, not local time.

You can optionally specify `--verbose` to get additional log output to see which files are being added and renamed.

It is recommended to use a script to pass the arguments automatically. For example, using a batch file placed in the directory containing the game installation (typically `Half-Life`):
```bat
dotnet "path/to/HalfLife.UnifiedSdk.Packager.dll"^
	--mod-directory "hlu"^
	--package-manifest "hlu/installer/PackageManifest.json"^
	--package-name "HalfLifeUnified-Game"^
	--verbose

pause
```

You can then execute the batch file without having to provide the arguments every time.

A possible result of this script is the creation of the file `HalfLifeUnified-Game-2022-05-17-09-28-18.zip`.

## Error reporting

If any errors occur the tool will log it to the console and stop execution.

## PackageManifest.json format

The package manifest file contains a list of include and exclude patterns to control which files are added.

The format is a JSON object:
```jsonc
{
	"PatternGroups": [
		{
			"Paths": [
				{
					"Path": "relative/path",
					"Required": false
				}
			],
			"IncludePatterns": [
			],
			"ExcludePatterns": [
			]
		}
	]
}
```

`PatternGroups` is an array of objects containing pattern information.

`Paths` is an array of objects containing path information. A path is a relative path starting in the game directory (e.g. `Half-Life`). If it is required then the packager will stop if it does not exist.

If the `Required` is not specified then it will default to `true`.

`IncludePatterns` is an array of glob patterns for files to include.
`ExcludePatterns` is an array of global patterns for files to exclude. This takes precedence over `IncludePatterns`, so a files that matches both will not be included.

The syntax for these patterns is described here: https://docs.microsoft.com/en-us/dotnet/api/microsoft.extensions.filesystemglobbing.matcher

Any occurrences of `%ModDirectory%` in `Path`, `IncludePatterns` and `ExcludePatterns` will be substituted for the mod directory name.

Example:
```jsonc
// See the MSDN documentation on the Matcher class for more information on what kind of patterns are supported:
// https://docs.microsoft.com/en-us/dotnet/api/microsoft.extensions.filesystemglobbing.matcher
{
	"PatternGroups": [
		{
			//The OpenAL library has to be in the game directory in order for the OS to find it when it's loaded.
			"Paths": [
				{
					"Path": "."
				}
			],
			"IncludePatterns": [
				"openal-%ModDirectory%.dll"
			]
		},
		{
			"Paths": [
				{
					"Path": "%ModDirectory%",
				},
				{
					"Path": "%ModDirectory%_hd",
					"Required": false
				},
				{
					"Path": "%ModDirectory%_lv",
					"Required": false
				}
			],
			
			// List of files and directories to package.
			// Filenames ending with ".install" will be renamed to remove this extension after being added to the archive.
			"IncludePatterns": [
				"cl_dlls/client.dll",
				"dlls/server.dll",
				"delta.lst",
				"halflife-unified.fgd",
				"liblist.gam",
				"server.cfg",
				"settings.scr.install",
				"user.scr.install",
				"titles.txt",
				"*.wad",
				"cfg/**",
				"events/*.sc",
				"installer/**",
				"maps/*.bsp",
				"maps/graphs/*",
				"models/**",
				"redist/VC_redist.x86.exe",
				"resource/**",
				"scripts/**",
				"sound/**",
				"sprites/**",
				"tools/**"
			],
			
			// Files and directories to exclude from the archive.
			"ExcludePatterns": [
				//Generated by game; contains player spray.
				"tempdecal.wad",
				// Exclude player model image (autogenerated by game).
				"models/player/remapped.bmp",
				// Exclude nuget configuration file. Only used for referencing dev packages.
				"**/nuget.config",
				// Exclude the VS Code directory. The contents are available on the wiki if modders need them.
				"**/.vscode/**",
				// Exclude omnisharp configuration files.
				"**/omnisharp.json"
			]
		}
	]
}
```