$modelname "player/custom_player.mdl"
$cdmaterials "models/player/custom_player"

// 定义身体组
$bodygroup "body"
{
    studio "body_reference.smd"
}

$bodygroup "head"
{
    studio "head_reference.smd"
}

// 定义序列 - 使用 SMD 文件中的动画
$sequence idle "idle" fps 1
$sequence walk "idle" fps 1
$sequence run "idle" fps 1

// 碰撞模型
$collisionmodel "body_reference.smd"
{
    $mass 80.0
}