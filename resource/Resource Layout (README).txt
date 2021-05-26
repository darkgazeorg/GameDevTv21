This game uses a resource file that is compiled by gorgon.
To create this file, another Gorgon Application needs to be run called Tile Application, which can be found in the resource folder.

If any resource (asset) is modified, changed or added, this application will need to be run to generate the new resource file.

The current layour of resources is as follows:

-assets
    -MapTiles
        -All Tiles for map layout in png format
    -Towers
        -All towers in png format
        -Text file for each tower, giving parameters and specifications.
        -Additional png items such as bases, rockets and other flying objects.
    -Enemies
        -All enemies in png format
        -Text file for each enemy, giving parameters and specifications.
        -Additional png items such as turrets.
