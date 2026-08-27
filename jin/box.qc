$modelname "props_junk/box.mdl"

$body mybody "box.smd"

$cdmaterials "models/props_junk/"

$sequence idle "box.smd" fps 1

$collisionmodel "box.smd"
{
	$mass 5.0
	$inertia 1.0
	$damping 0.0
	$rotdamping 0.0
}