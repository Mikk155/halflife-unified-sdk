# Bsp2Obj

> **Note**
> This is a [.NET tool](/docs/dotnet-tools.md)

> **Note**
> This tool is included in the `hlu/tools` directory in the game installation package available [here](https://github.com/SamVanheer/halflife-unified-sdk/releases)

The Bsp2Obj tool is used to convert Half-Life 1 BSP files to the Wavefront OBJ format.

The following files are generated:
* `mapname.obj`
* `mapname.mtl`
* `mapname_material_index.tga`

`mapname` is the name of the BSP file. `index` is the number of the generated material.

Materials are only generated if the texture is embedded in the BSP file, otherwise a dummy material with no texture is used.

## Usage

### Command line

```
dotnet "path/to/HalfLife.UnifiedSdk.Bsp2Obj.dll"
--destination "path/to/destination/directory"
<bsp filename>
```

The `destination` parameter specifies where to save generated files. If not specified the files are saved in the same directory as the BSP file.

Specify a BSP file to convert.

## Error reporting

If any errors occur the tool will log it to the console and stop execution.