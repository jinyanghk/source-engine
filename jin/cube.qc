$modelname "contrib/pi/cube.mdl"
$upaxis Y
$scale 1.00
$body "Body" "cube.obj"
$surfaceprop "cloth"
$cdmaterials "models/contrib/pi/cube"
$sequence "idle" "cube.obj" fps 30 numframes 30 loop
$collisionmodel "cube.obj" {
$mass 5.0
}
