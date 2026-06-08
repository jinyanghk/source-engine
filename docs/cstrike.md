
### build client

```sh
python3 ./waf configure -T debug --prefix=cstrike --build-games=cstrike --disable-warns
python3 ./waf build
python3 ./waf install
```

launcher client in fullscreen:

```sh
./hl2_launcher -console -game cstrike +map de_dust2
```

launcher client in window:

```sh
./hl2_launcher -console -game cstrike -window -width 1024 -height 768 -dev +map de_dust2
```

join server from console:

```
connect IP_ADDRESS:PORT
```

### build dedicated server

```sh
python3 ./waf configure -T debug --prefix=cstrike --build-games=cstrike --dedicated --disable-warns
python3 ./waf build
python3 ./waf install
```

```sh
cd cstrike
LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./dedicated_launcher -game cstrike -console -usercon +map de_dust2
```

### console commands

```
+alt1
+alt2
+attack
+attack2
+attack3
+back
+break
+camdistance
+camin
+cammousemove
+camout
+campitchdown
+campitchup
+camyawleft
+camyawright
+commandermousemove
+demoui2 // Bring the advanced demo player UI (demoui2) to foreground.
+duck
+forward
+graph
+grenade1
+grenade2
+jlook
+jump
+klook
+left
+lookdown
+lookup
+mat_texture_list
+movedown
+moveleft
+moveright
+moveup
+posedebug // Turn on pose debugger or add ents to pose debugger UI
+reload
+right
+score
+showbudget
+showbudget_texture
+showbudget_texture_global
+showscores
+showvprof
+speed
+strafe
+use
+vgui_drawtree
+voicerecord
+walk
+zoom
-alt1
-alt2
-attack
-attack2
-attack3
-back
-break
-camdistance
-camin
-cammousemove
-camout
-campitchdown
-campitchup
-camyawleft
-camyawright
-commandermousemove
-demoui2 // Send the advanced demo player UI (demoui2) to background.
-duck
-forward
-graph
-grenade1
-grenade2
-jlook
-jump
-klook
-left
-lookdown
-lookup
-mat_texture_list
-movedown
-moveleft
-moveright
-moveup
-posedebug // Turn off pose debugger or hide ents from pose debugger UI
-reload
-right
-score
-showbudget
-showbudget_texture
-showbudget_texture_global
-showscores
-showvprof
-speed
-strafe
-use
-vgui_drawtree
-voicerecord
-walk
-zoom
achievement_debug "0" // Turn on achievement debug msgs.
addip // Add an IP address to the ban list.
adsp_alley_min "122"
adsp_courtyard_min "126"
adsp_debug "0"
adsp_door_height "112"
adsp_duct_min "106"
adsp_hall_min "110"
adsp_low_ceiling "108"
adsp_opencourtyard_min "126"
adsp_openspace_min "130"
adsp_openstreet_min "118"
adsp_openwall_min "130"
adsp_room_min "102"
adsp_street_min "118"
adsp_tunnel_min "114"
adsp_wall_height "128"
air_density // Changes the density of air for drag computations.
ai_auto_contact_solver "1"
ai_block_damage "0"
ai_debugscriptconditions "0"
ai_debug_assault "0"
ai_debug_avoidancebounds "0"
ai_debug_directnavprobe "0"
ai_debug_doors "0"
ai_debug_dyninteractions "0" // Debug the NPC dynamic interaction system.
ai_debug_efficiency "0"
ai_debug_enemies "0"
ai_debug_expressions "0" // Show random expression decisions for NPCs.
ai_debug_follow "0"
ai_debug_loners "0"
ai_debug_looktargets "0"
ai_debug_los "0" // itl
ai_debug_nav "0"
ai_debug_node_connect // Debug the attempted connection between two nodes
ai_debug_ragdoll_magnets "0"
ai_debug_shoot_positions "0"
ai_debug_speech "0"
ai_debug_squads "0"
ai_debug_think_ticks "0"
ai_default_efficient "0"
ai_drawbattlelines "0"
ai_drop_hint // Drop an ai_hint at the players current eye position.
ai_dump_hints
ai_efficiency_override "0"
ai_enable_fear_behavior "1"
ai_expression_frametime "0" // Maximum frametime to still play background expressions.
ai_expression_optimization "0" // Disable npc background expressions when you cant see them.
ai_fear_player_dist "720"
ai_find_lateral_cover "1"
ai_find_lateral_los "1"
ai_follow_use_points "1"
ai_follow_use_points_when_moving "1"
ai_force_serverside_ragdoll "0"
ai_frametime_limit "50" // frametime limit for min efficiency AIE_NORMAL (in secs).
ai_lead_time "0"
ai_LOS_mode "0"
ai_moveprobe_debug "0"
ai_moveprobe_jump_debug "0"
ai_moveprobe_usetracelist "0"
ai_navigator_generate_spikes "0"
ai_navigator_generate_spikes_strength "8"
ai_norebuildgraph "0"
ai_no_local_paths "0"
ai_no_node_cache "0"
ai_no_select_box "0"
ai_no_steer "0"
ai_no_talk_delay "0"
ai_path_adjust_speed_on_immediate_turns "1"
ai_path_insert_pause_at_est_end "1"
ai_path_insert_pause_at_obstruction "1"
ai_post_frame_navigation "0"
ai_radial_max_link_dist "512"
ai_reaction_delay_alert "0"
ai_reaction_delay_idle "0"
ai_rebalance_thinks "1"
ai_report_task_timings_on_limit "0"
ai_sequence_debug "0"
ai_setupbones_debug "0" // Shows that bones that are setup every think
ai_set_move_height_epsilon // Set how high AI bumps up ground walkers when checking steps
ai_shot_bias "1"
ai_shot_bias_max "1"
ai_shot_bias_min "-1"
ai_shot_stats "0"
ai_shot_stats_term "1000"
ai_show_hull_attacks "0"
ai_show_think_tolerance "0"
ai_simulate_task_overtime "0"
ai_spread_cone_focus_time "0"
ai_spread_defocused_cone_multiplier "3"
ai_spread_pattern_focus_time "0"
ai_strong_optimizations "0"
ai_strong_optimizations_no_checkstand "0"
ai_task_pre_script "0"
ai_test_moveprobe_ignoresmall "0"
ai_think_limit_label "0"
ai_use_clipped_paths "1"
ai_use_efficiency "1"
ai_use_frame_think_limits "1"
ai_use_think_optimizations "1"
ai_use_visibility_cache "1"
ai_vehicle_avoidance "1"
alias // Alias a command.
ammo_338mag_max "30"
ammo_357sig_max "52"
ammo_45acp_max "100"
ammo_50AE_max "35"
ammo_556mm_box_max "200"
ammo_556mm_max "90"
ammo_57mm_max "100"
ammo_762mm_max "90"
ammo_9mm_max "120"
ammo_buckshot_max "32"
ammo_flashbang_max "2"
ammo_hegrenade_max "1"
ammo_smokegrenade_max "1"
anim_3wayblend "1" // Toggle the 3-way animation blending code.
askconnect_accept // Accept a redirect request by the server.
async_allow_held_files "1" // Allow AsyncBegin/EndRead()
async_mode "0" // 1 = synchronous)
async_resume
async_serialize "0" // Force async reads to serialize for profiling
async_simulate_delay "0" // Simulate a delay of up to a set msec per file operation
async_suspend
audit_save_in_memory // Audit the memory usage and files in the save-to-memory system
autoaim_max_deflect "0"
autoaim_max_dist "2160"
autobuy // Attempt to purchase items with the order listed in cl_autobuy
autosave // Autosave
autosavedangerous // AutoSaveDangerous
autosavedangerousissafe
banid // Add a user ID to the ban list.
banip // Add an IP address to the ban list.
benchframe // Takes a snapshot of a particular frame in a time demo.
bench_end // Ends gathering of info.
bench_showstatsdialog // Shows a dialog displaying the most recent benchmark results.
bench_start // Starts gathering of info. Arguments: filename to write results into
bench_upload // Uploads most recent benchmark stats to the Valve servers.
bind // Bind a key.
binds_per_command "1"
BindToggle // Performs a bind <key> increment var <cvar> 0 1 1
bind_mac // not win32
blink_duration "0" // How many seconds an eye blink will last.
bloodspray // blood
bot_add // bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.
bot_add_ct // bot_add_ct <type> <difficulty> <name> - Adds a Counter-Terrorist bot matching the given criteria.
bot_add_t // bot_add_t <type> <difficulty> <name> - Adds a terrorist bot matching the given criteria.
bot_allow_grenades "1" // bots may use grenades.
bot_allow_machine_guns "1" // bots may use the machine gun.
bot_allow_pistols "1" // bots may use pistols.
bot_allow_rifles "1" // bots may use rifles.
bot_allow_rogues "1" // nor pursue scenario goals.
bot_allow_shotguns "1" // bots may use shotguns.
bot_allow_snipers "1" // bots may use sniper rifles.
bot_allow_sub_machine_guns "1" // bots may use sub-machine guns.
bot_all_weapons // Allows the bots to use all weapons
bot_auto_follow "0" // bots with high co-op may automatically follow a nearby human player.
bot_auto_vacate "1" // bots will automatically leave to make room for human players.
bot_chatter "0" // or normal.
bot_crouch "0"
bot_debug "0" // For internal testing purposes.
bot_debug_target "0" // For internal testing purposes.
bot_defer_to_human "0" // the bots will not do the scenario tasks.
bot_difficulty "1" // 3=expert.
bot_dont_shoot "0" // bots will not fire weapons (for debugging).
bot_eco_limit "2000" // bots will not buy if their money falls below this amount.
bot_flipout "0" // they run around randomly.
bot_freeze "0"
bot_goto_mark // Sends a bot to the selected nav area (useful for testing navigation meshes)
bot_join_after_player "1" // bots wait until a player joins before entering the  game.
bot_join_delay "0" // Prevents bots from joining the server for this many seconds after a map change.
bot_join_team "0" // or CT.
bot_kick // matching the given criteria.
bot_kill // matching the given criteria.
bot_knives_only // Restricts the bots to only using knives
bot_loadout "0" // bots are given these items at round start
bot_mimic "0" // Bot uses usercmd of player by index.
bot_mimic_yaw_offset "180"
bot_pistols_only // Restricts the bots to only using pistols
bot_prefix "0" // This string is prefixed to the name of all bots that join the game. <difficulty> will be replaced with the bots difficulty. <w
bot_profile_db "0" // The filename from which bot profiles will be read.
bot_quota "0" // Determines the total number of bots in the game.
bot_quota_mode "0" // Determines the type of quota. Allowed values: normal, fill, and match. If fill, the server will adjust bots to keep N p
bot_randombuy "0" // should bots ignore their prefered weapons and just buy weapons at random?
bot_show_battlefront "0" // Show areas where rushing players will initially meet.
bot_show_nav "0" // For internal testing purposes.
bot_show_occupy_time "0" // Show when each nav area can first be reached by each team.
bot_snipers_only // Restricts the bots to only using sniper rifles
bot_stop "0" // immediately stops all bot processing.
bot_traceview "0" // For internal testing purposes.
bot_walk "0" // not run.
bot_zombie "0" // bots will stay in idle mode and not attack.
box // Draw a debug box.
breakable_disable_gib_limit "0"
breakable_multiplayer "1"
buddha // Toggle. Player takes damage but wont die. (Shows red cross when health is zero)
budget_averages_window "30" // number of frames to look at when figuring out average frametimes
budget_background_alpha "128" // how translucent the budget panel is
budget_bargraph_background_alpha "128" // how translucent the budget panel is
budget_bargraph_range_ms "16" // budget bargraph range in milliseconds
budget_history_numsamplesvisible "100" // number of samples to draw in the budget history window. The lower the better as far as rendering overhead of the budget panel
budget_history_range_ms "66" // budget history range in milliseconds
budget_panel_bottom_of_history_fraction "0" // number between 0 and 1
budget_panel_height "384" // height in pixels of the budget panel
budget_panel_width "512" // width in pixels of the budget panel
budget_panel_x "0" // number of pixels from the left side of the game screen to draw the budget panel
budget_panel_y "50" // number of pixels from the top side of the game screen to draw the budget panel
budget_peaks_window "30" // number of frames to look at when figuring out peak frametimes
budget_show_averages "0" // enable/disable averages in the budget panel
budget_show_history "1" // turn history graph off and on. . good to turn off on low end
budget_show_peaks "1" // enable/disable peaks in the budget panel
budget_toggle_group // Turn a budget group on/off
bug // Show/hide the bug reporting UI.
bugreporter_includebsp "1" // Include .bsp for internal bug submissions.
bugreporter_uploadasync "0" // Upload attachments asynchronously
bug_swap // Automatically swaps the current weapon for the bug bait and back again.
buildcubemaps // Rebuild cubemaps.
building_cubemaps "0"
buyequip // Show equipment buy menu
buymenu // Show main buy menu
cache_print // cache_print [section] Print out contents of cache memory.
cache_print_lru // cache_print_lru [section] Print out contents of cache memory.
cache_print_summary // cache_print_summary [section] Print out a summary contents of cache memory.
callvote // Start a vote on an issue.
camortho // Switch to orthographic camera.
cam_collision "1" // an attempt is made to keep the camera from passing though walls.
cam_command "0"
cam_idealdelta "4" // Controls the speed when matching offset to ideal angles in thirdperson view
cam_idealdist "150"
cam_idealdistright "0"
cam_idealdistup "0"
cam_ideallag "4" // Amount of lag used when matching offset to ideal angles in thirdperson view
cam_idealpitch "0"
cam_idealyaw "0"
cam_showangles "0" // print viewangles/idealangles/cameraoffsets to the  console.
cam_snapto "0"
cancelselect
cast_hull // Tests hull collision detection
cast_ray // Tests collision detection
ccs_create_convars_from_hwconfig // useful for diffing purposes
cc_captiontrace "1" // 2 = show in hud)
cc_emit // Emits a closed caption
cc_findsound // Searches for soundname which emits specified text.
cc_flush // Flushes asyncd captions.
cc_lang "0" // Current close caption language (emtpy = use  game UI language)
cc_linger_time "1" // Close caption linger time.
cc_minvisibleitems "1" // Minimum number of caption items to show.
cc_predisplay_time "0" // Close caption delay before showing caption.
cc_random // Emits a random caption
cc_sentencecaptionnorepeat "4" // How often a sentence can repeat.
cc_showblocks // Toggles showing which blocks are pending/loaded async.
cc_smallfontlength "300" // force usage of small font size.
cc_subtitles "0" // wont help hearing impaired players).
centerview
changelevel // Change server to the specified map
changelevel2 // Transition to the specified map in single player
changelevel_next // Immediately changes to the next map in the map rotation for the server.
chooseteam // Choose a new team
ch_createairboat // Spawn airboat in front of the player.
ch_createjeep // Spawn jeep in front of the player.
clear // Clear all console output.
clear_debug_overlays // clears debug overlays
clientport "27005" // Host game client port
closecaption "0" // Enable close captioning.
cl_allowdownload "1" // Client downloads customization files
cl_allowupload "1" // Client uploads customization files
cl_anglespeedkey "0"
cl_animationinfo // Hud element to examine.
cl_autobuy "0" // The order in which autobuy will attempt to purchase items
cl_autohelp "1" // Auto-help
cl_autowepswitch "1" // Automatically switch to picked up weapons (if more powerful)
cl_backspeed "400"
cl_bob "0"
cl_bobcycle "0"
cl_bobup "0"
cl_burninggibs "0" // A burning player that gibs has burning gibs.
cl_buy_favorite // Purchase a favorite weapon/equipment loadout
cl_buy_favorite_nowarn "0" // Skips the error prompt when saving an invalid buy favorite
cl_buy_favorite_quiet "0" // Skips the prompt when saving a buy favorite in the buy menu
cl_buy_favorite_reset // Reset favorite loadouts to the default
cl_buy_favorite_set // Saves the current loadout as a favorite
cl_c4dynamiclight "0" // Draw dynamic light when planted c4 flashes
cl_c4progressbar "1" // Draw progress bar when defusing the C4
cl_chatfilters "63" // Stores the chat filter settings
cl_class "0" // Default class when joining a game
cl_clearhinthistory // Clear memory of client side hints displayed to the player.
cl_clockdrift_max_ms "150" // Maximum number of milliseconds the clock is allowed to drift before the client snaps its clock to the servers.
cl_clockdrift_max_ms_threadmode "0" // Maximum number of milliseconds the clock is allowed to drift before the client snaps its clock to the servers.
cl_clock_correction "1" // Enable/disable clock correction on the client.
cl_clock_correction_adjustment_max_amount "200" // Sets the maximum number of milliseconds per second it is allowed to correct the client clock. It will only correct this amount
cl_clock_correction_adjustment_max_offset "90" // it moves towards apply
cl_clock_correction_adjustment_min_offset "10" // then no clock correction is applied.
cl_clock_correction_force_server_tick "999" // Force clock correction to match the server tick + this offset (-999 disables it).
cl_clock_showdebuginfo "0" // Show debugging info about the clock drift.
cl_cmdrate "30" // Max number of command packets sent to server per second
cl_crosshairalpha "255"
cl_crosshaircolor "3" // 5=custom
cl_crosshaircolor_b "50"
cl_crosshaircolor_g "250"
cl_crosshaircolor_r "250"
cl_crosshairdot "1"
cl_crosshairscale "0" // Crosshair scaling factor (deprecated)
cl_crosshairsize "4"
cl_crosshairspreadscale "0"
cl_crosshairthickness "0"
cl_crosshairusealpha "1"
cl_customsounds "0" // Enable customized player sound playback
cl_debugrumble "0" // Turn on rumble debugging spew
cl_debug_player_perf "0"
cl_demoviewoverride "0" // Override view during demo playback
cl_detaildist "1200" // Distance at which detail props are no longer visible
cl_detailfade "400" // Distance across which detail props fade in
cl_detail_avoid_force "0" // percentage of the width of the detail sprite )
cl_detail_avoid_radius "64" // radius around detail sprite to avoid players
cl_detail_avoid_recover_speed "0" // how fast to recover position after avoiding players
cl_detail_max_sway "5" // Amplitude of the detail prop sway
cl_detail_multiplier "1" // extra details to create
cl_disablefreezecam "0" // Turn on/off freezecam on client
cl_disablehtmlmotd "0" // Disable HTML motds.
cl_downloadfilter "0" // mapsonly)
cl_drawhud "1" // Enable the rendering of the hud
cl_drawleaf "-1"
cl_drawmaterial "0" // Draw a particular material over the frame
cl_drawmonitors "1"
cl_drawshadowtexture "0"
cl_dump_particle_stats // dump particle profiling info to particle_profile.csv
cl_dynamiccrosshair "1" // 3=l
cl_ejectbrass "1"
cl_entityreport "0" // draw entity states to  console
cl_entityreport_sorted "0" // 3 = peak
cl_ent_absbox // Displays the clients absbox for the entity under the crosshair.
cl_ent_bbox // Displays the clients bounding box for the entity under the crosshair.
cl_ent_rbox // Displays the clients render box for the entity under the crosshair.
cl_extrapolate "1" // Enable/disable extrapolation if interpolation history runs out.
cl_extrapolate_amount "0" // Set how many seconds the client will extrapolate entities for.
cl_fastdetailsprites "1" // whether to use new detail sprite system
cl_fasttempentcollision "5"
cl_find_ent // Find and list all client entities with classnames that contain the specified substring. Format: cl_find_ent <substring>
cl_find_ent_index // Display data for clientside entity matching specified index. Format: cl_find_ent_index <index>
cl_first_person_uses_world_model "0" // Causes the third person model to be drawn instead of the view model
cl_flushentitypacket "0" // For debugging. Force the engine to flush an entity packet.
cl_forcepreload "0" // Whether we should force preloading.
cl_forwardspeed "400"
cl_fullupdate // Forces the server to send a full update packet
cl_hudhint_sound "1" // Disable hudhint sounds.
cl_idealpitchscale "0"
cl_ignorepackets "0" // Force client to ignore packets (for debugging).
cl_interp "0" // Sets the interpolation amount (bounded on low side by server interp ratio settings).
cl_interp_all "0" // Disable interpolation list optimizations.
cl_interp_npcs "0" // if greater)
cl_interp_ratio "2" // Sets the interpolation amount (final amount is cl_interp_ratio / cl_updaterate).
cl_jiggle_bone_debug "0" // Display physics-based jiggle bone debugging information
cl_jiggle_bone_debug_pitch_constraints "0" // Display physics-based jiggle bone debugging information
cl_jiggle_bone_debug_yaw_constraints "0" // Display physics-based jiggle bone debugging information
cl_jiggle_bone_framerate_cutoff "45" // Skip jiggle bone simulation if framerate drops below this value (frames/second)
cl_lagcompensation "1" // Perform server side lag compensation of weapon firing events.
cl_language "0" // Language (from HKCUSoftwareValveSteamLanguage)
cl_left_hand_ik "0" // Attach players left hand to rifle with IK.
cl_legacy_crosshair_recoil "0" // Enable legacy framerate dependent crosshair recoil
cl_legacy_crosshair_scale "0" // Enable legacy crosshair scaling
cl_leveloverview "0"
cl_leveloverviewmarker "0"
cl_localnetworkbackdoor "1" // Enable network optimizations for single player  games.
cl_locationalpha "150"
cl_logofile "0" // Spraypoint logo decal.
cl_maxrenderable_dist "3000" // Max distance from the camera at which things will be rendered
cl_meathook_neck_pivot_ingame_fwd "3"
cl_meathook_neck_pivot_ingame_up "7"
cl_minmodels "0" // Uses one player model for each team.
cl_min_ct "1" // Controls which CT model is used when cl_minmodels is set.
cl_min_t "1" // Controls which Terrorist model is used when cl_minmodels is set.
cl_mouseenable "1"
cl_mouselook "1" // 0 for keyboard look. Cannot be set while connected to a server.
cl_new_impact_effects "0"
cl_nowinpanel "0" // Turn on/off win panel on client
cl_observercrosshair "1"
cl_overdraw_test "0"
cl_panelanimation // Shows panel animation variables: <panelname | blank for all panels>.
cl_particleeffect_aabb_buffer "2" // it wont have to be reinserted as oft
cl_particle_batch_mode "1"
cl_particle_retire_cost "0"
cl_particle_show_bbox "0"
cl_particle_show_bbox_cost "0" // Show # of particles: green->blue->red. Use a negative number to show ALL particles even cheap ones
cl_particle_stats_start // Start or restart particle stats - also dumps to particle_stats.csv
cl_particle_stats_stop // or snapshot this frame - also dumps to particle_stats.csv
cl_particle_stats_trigger_count "0" // Dump stats if the particle count exceeds this number.
cl_pclass "0" // Dump entity by prediction classname.
cl_pdump "-1" // Dump info about this entity to screen.
cl_phys_props_enable "1" // Disable clientside physics props (must be set before loading a level).
cl_phys_props_max "300" // Maximum clientside physic props
cl_phys_props_respawndist "1500" // Minimum distance from the player that a clientside prop must be before its allowed to respawn.
cl_phys_props_respawnrate "60" // between clientside prop respawns.
cl_phys_timescale "1" // Sets the scale of time for client-side physics (ragdolls)
cl_pitchdown "89"
cl_pitchspeed "225" // Client pitch speed.
cl_pitchup "89"
cl_playback_screenshots "0" // Allows the client to playback screenshot and jpeg commands in demos.
cl_playerspraydisable "0" // Disable player sprays.
cl_precacheinfo // Show precache info (client).
cl_predict "1" // Perform client side prediction.
cl_predictionlist "0" // Show which entities are predicting
cl_predictweapons "1" // Perform client side prediction of weapon effects.
cl_pred_optimize "2" // and also for not repredicting if there were no errors (2)
cl_pred_track // for field fieldname.
cl_radaralpha "200"
cl_radartype "0"
cl_radar_locked "0" // Lock the angle of the radar display?
cl_ragdoll_collide "0"
cl_ragdoll_physics_enable "1" // Enable/disable ragdoll physics.
cl_rebuy "0" // The order in which rebuy will attempt to repurchase items
cl_removedecals // Remove the decals from the entity under the crosshair.
cl_resend "6" // Delay in seconds before the client will resend the connect attempt
cl_righthand "1" // Use right-handed view models.
cl_round_win_fade_time "1"
cl_rumblescale "1" // Scale sensitivity of rumble effects (0 to 1.0)
cl_scalecrosshair "1" // Enable crosshair scaling (deprecated)
cl_scoreboard_clan_ct_color_blue "255" // Scoreboard CT player clan tag blue channel
cl_scoreboard_clan_ct_color_green "200" // Scoreboard CT player clan tag green channel
cl_scoreboard_clan_ct_color_red "150" // Scoreboard CT player clan tag red channel
cl_scoreboard_clan_t_color_blue "90" // Scoreboard T player clan tag blue channel
cl_scoreboard_clan_t_color_green "90" // Scoreboard T player clan tag green channel
cl_scoreboard_clan_t_color_red "240" // Scoreboard T player clan tag red channel
cl_scoreboard_ct_color_blue "255" // Scoreboard CT player data blue channel
cl_scoreboard_ct_color_green "200" // Scoreboard CT player data green channel
cl_scoreboard_ct_color_red "150" // Scoreboard CT player data red channel
cl_scoreboard_dead_clan_color_blue "125" // Scoreboard dead player clan tag blue channel
cl_scoreboard_dead_clan_color_green "125" // Scoreboard dead player clan tag green channel
cl_scoreboard_dead_clan_color_red "125" // Scoreboard dead player clan tag red channel
cl_scoreboard_dead_color_blue "125" // Scoreboard dead player data blue channel
cl_scoreboard_dead_color_green "125" // Scoreboard dead player data green channel
cl_scoreboard_dead_color_red "125" // Scoreboard dead player data red channel
cl_scoreboard_t_color_blue "90" // Scoreboard T player data blue channel
cl_scoreboard_t_color_green "90" // Scoreboard T player data green channel
cl_scoreboard_t_color_red "240" // Scoreboard T player data red channel
cl_screenshotname "0" // Custom Screenshot name
cl_SetupAllBones "0"
cl_shadowtextureoverlaysize "256"
cl_showbattery "0" // Draw current battery level at top of screen when on battery power
cl_ShowBoneSetupEnts "0" // Show which entities are having their bones setup each frame.
cl_showdemooverlay "0" // -1 - show always)
cl_showents // Dump entity list to  console.
cl_showerror "0" // 2 for above plus detailed field deltas.
cl_showevents "0" // Print event firing info in the console
cl_showfps "0" // 2 = smooth fps)
cl_showhelp "1" // Set to 0 to not show on-screen help
cl_showpausedimage "1" // Show the Paused image when  game is paused.
cl_showpluginmessages "1" // Allow plugins to display messages to you
cl_showpos "0" // Draw current position at top of screen
cl_ShowSunVectors "0"
cl_showtextmsg "1" // Enable/disable text messages printing on the screen.
cl_show_achievement_popups "1"
cl_show_connectionless_packet_warnings "0" // Show console messages about ignored connectionless packets on the client.
cl_show_num_particle_systems "0" // Display the number of active particle systems.
cl_show_splashes "1"
cl_sidespeed "400"
cl_smooth "1" // Smooth view/eye origin after prediction errors
cl_smoothtime "0" // Smooth clients view after prediction error over this many seconds
cl_software_cursor "0" // Switches the game to use a larger software cursor instead of the normal OS cursor
cl_soundemitter_flush // Flushes the sounds.txt system (client only)
cl_soundfile "0" // Jingle sound file.
cl_soundscape_flush // Flushes the client side soundscapes
cl_soundscape_printdebuginfo // print soundscapes
cl_spec_mode "6" // spectator mode
cl_sporeclipdistance "512"
cl_sun_decay_rate "0"
cl_team "0" // Default team when joining a game
cl_threaded_bone_setup "0" // Enable parallel processing of C_BaseAnimating::SetupBones()
cl_threaded_client_leaf_system "0"
cl_timeout "30" // the client will disconnect itself
cl_updaterate "20" // Number of packets per second of updates you are requesting from the server
cl_upspeed "320"
cl_view // Set the view entity index.
cl_voice_filter "0" // Filter voice by name substring
cl_vote_ui_active_after_voting "0"
cl_vote_ui_show_notification "0"
cl_winddir "0" // Weather effects wind direction angle
cl_windspeed "0" // Weather effects wind speed scalar
cl_wpn_sway_interp "0"
cl_wpn_sway_scale "1"
cl_yawspeed "210" // Client yaw speed.
cmd // Forward command to server.
collision_shake_amp "0"
collision_shake_freq "0"
collision_shake_time "0"
collision_test // Tests collision system
colorcorrectionui // Show/hide the color correction tools UI.
commentary "0" // Desired commentary mode state.
commentary_available "0" // Automatically set by the game when a commentary file is available for the current map.
commentary_cvarsnotchanging
commentary_finishnode
commentary_firstrun "0"
commentary_showmodelviewer // Display the commentary model viewer. Usage: commentary_showmodelviewer <model name> <optional attached model name>
commentary_testfirstrun
condump // dump the text currently in the console to condumpXX.log
connect // Connect to specified server.
contimes "8" // Number of console lines to overlay for debugging.
con_drawnotify "1" // Disables drawing of notification area (for taking screenshots).
con_enable "1" // Allows the console to be activated.
con_filter_enable "0" // 2 displays filtered text brighter than ot
con_filter_text "0" // Text with which to filter console spew. Set con_filter_enable 1 or 2 to activate.
con_filter_text_out "0" // Text with which to filter OUT of console spew. Set con_filter_enable 1 or 2 to activate.
con_logfile "0" //  Console output gets written to this file
con_notifytime "8" // How long to display recent console text to the upper part of the game window
con_nprint_bgalpha "50" // Con_NPrint background alpha.
con_nprint_bgborder "5" // Con_NPrint border size.
con_timestamp "0" // Prefix console.log entries with timestamps
con_trace "0" // Print console text to low level printout.
coop "0" // Cooperative play.
CreateHairball
CreatePredictionError // Create a prediction error
create_flashlight
creditsdone
crosshair "1"
cs_make_vip // Marks a player as the VIP
cs_ShowStateTransitions "-2" // cs_ShowStateTransitions <ent index or -1 for all>. Show player state transitions.
cvarlist // Show the list of convars/concommands.
c_maxdistance "200"
c_maxpitch "90"
c_maxyaw "135"
c_mindistance "30"
c_minpitch "0"
c_minyaw "-135"
c_orthoheight "100"
c_orthowidth "100"
datacachesize "256" // Size in MB.
dbghist_addline // Add a line to the debug history. Format: <category id> <line>
dbghist_dump // Dump the debug history to the  console. Format: <category id> Categories: 0: Entity I/O 1: AI Decisions 2: Sc
deathmatch "1" // Running a deathmatch server.
debugsystemui // Show/hide the debug system UI.
debug_materialmodifycontrol "0"
debug_materialmodifycontrol_client "0"
debug_physimpact "0"
debug_touchlinks "0" // Spew touch link activity
decalfrequency "10"
default_fov "90"
demolist // Print demo sequence list.
demos // Demo demo file sequence.
demoui // Show/hide the demo player UI.
demoui2 // Show/hide the advanced demo player UI (demoui2).
demo_avellimit "2000" // Angular velocity limit before eyes considered snapped for demo playback.
demo_debug "0" // Demo debug info.
demo_fastforwardfinalspeed "20" // Go this fast when starting to hold FF button.
demo_fastforwardramptime "5" // How many seconds it takes to get to full FF speed.
demo_fastforwardstartspeed "2" // Go this fast when starting to hold FF button.
demo_fov_override "0" // this value will be used to override FOV during demo playback.
demo_gototick // Skips to a tick in demo.
demo_interplimit "4000" // How much origin velocity before its considered to have teleported causing interpolation to reset.
demo_interpolateview "1" // Do view interpolation during dem playback.
demo_legacy_rollback "1" // Use legacy view interpolation rollback amount in demo playback.
demo_pause // Pauses demo playback.
demo_pauseatservertick "0" // Pauses demo playback at server tick
demo_quitafterplayback "0" // Quits  game after demo playback.
demo_recordcommands "1" // Record commands typed at console into .dem files.
demo_resume // Resumes demo playback.
demo_setendtick // Sets end demo playback tick. Set to 0 to disable.
demo_timescale // Sets demo replay speed.
demo_togglepause // Toggles demo playback.
developer "0" // Set developer message level
devshots_nextmap // Used by the devshots system to go to the next map in the devshots maplist.
devshots_screenshot // use the screenshot command instead.
differences // Show all convars which are not at their default values.
disconnect // Disconnect game from server.
dispcoll_drawplane "0"
displaysoundlist "0"
disp_dynamic "0"
dlight_debug // Creates a dlight in front of the player
download_debug "0"
drawcross // Draws a cross at the given location Arguments: x y z
drawline // Draws line between two 3D Points. Green if no collision Red is collides with something Arguments: x1 y1 z1 x2 y2 z2
drawradar // Draws HUD radar
dsp_automatic "0"
dsp_db_min "80"
dsp_db_mixdrop "0"
dsp_dist_max "1440"
dsp_dist_min "0"
dsp_enhance_stereo "0"
dsp_facingaway "0"
dsp_mix_max "0"
dsp_mix_min "0"
dsp_off "0"
dsp_player "0"
dsp_reload
dsp_room "0"
dsp_slow_cpu "0"
dsp_spatial "40"
dsp_speaker "50"
dsp_volume "1"
dsp_vol_2ch "1"
dsp_vol_4ch "0"
dsp_vol_5ch "0"
dsp_water "14"
dti_flush // Write out the datatable instrumentation files (you must run with -dti for this to work).
dtwarning "0" // Print data table warnings?
dtwatchclass "0" // Watch all fields encoded with this table.
dtwatchent "-1" // Watch this entities data table encoding.
dtwatchvar "0" // Watch the named variable.
dt_ShowPartialChangeEnts "0" // (SP only) - show entities that were copied using small optimized lists (FL_EDICT_PARTIAL_CHANGE).
dt_UsePartialChangeEnts "1" // (SP only) - enable FL_EDICT_PARTIAL_CHANGE optimization.
dumpentityfactories // Lists all entity factory names.
dumpeventqueue // Dump the contents of the Entity I/O event queue to the console.
dumpgamestringtable // Dump the contents of the game string table to the console.
dumplongticks // Enables generating minidumps on long ticks.
dumpsavedir // List the contents of the save directory in memory
dumpstringtables // Print string tables to console.
dump_entity_sizes // Print sizeof(entclass)
dump_globals // Dump all global entities/states
dump_panels // Dump Panel Tree
dump_x360_cfg // Dump X360 config files to disk
dump_x360_saves // Dump X360 save  games to disk
echo // Echo text to console.
editdemo // Edit a recorded demo file (.dem ).
editor_toggle // Disables the simulation and returns focus to the editor
enable_debug_overlays "1" // Enable rendering of debug overlays
endmovie // Stop recording movie frames.
endround // End the current round.
engine_no_focus_sleep "50"
english "0" // running the english language set of assets.
ent_absbox // Displays the total bounding box for the given entity(s) in green. Some entites will also display entity specific overlays. Ar
ent_attachments // Displays the attachment points on an entity. Arguments: {entity_name} / {class_name} / no argument picks what player is loo
ent_autoaim // Displays the entitys autoaim radius. Arguments: {entity_name} / {class_name} / no argument picks what player is looking at
ent_bbox // Displays the movement bounding box for the given entity(ies) in orange. Some entites will also display entity specific overlay
ent_cancelpendingentfires // Cancels all ent_fire created outputs that are currently waiting for their delay to expire.
ent_create // Creates an entity of the given type where the player is looking. Additional parameters can be passed in in the form: ent_creat
ent_debugkeys "0"
ent_dump // Usage: ent_dump <entity name>
ent_fire // Usage: ent_fire <target> [action] [value] [delay]
ent_info // Usage: ent_info <class name>
ent_keyvalue // Applies the comma delimited key=value pairs to the entity with the given Hammer ID. Format: ent_keyvalue <entity id> <key1> <v
ent_messages // Toggles input/output message display for the selected entity(ies). The name of the entity will be displayed as well as any mes
ent_messages_draw "0" // Visualizes all entity input/output activity.
ent_name
ent_orient // only orients target entitys YAW. Use the allangles opt
ent_pause // Toggles pausing of input/output message processing for entities. When turned on processing of all message will stop. Any mess
ent_pivot // Displays the pivot for the given entity(ies). (y=up=green, z=forward=blue, x=left=red). Arguments: {entity_name} / {class
ent_rbox // Displays the total bounding box for the given entity(s) in green. Some entites will also display entity specific overlays. Ar
ent_remove // Removes the given entity(s) Arguments: {entity_name} / {class_name} / no argument picks what player is looking at
ent_remove_all // Removes all entities of the specified type Arguments: {entity_name} / {class_name}
ent_rotate // Rotates an entity by a specified # of degrees
ent_setname // Sets the targetname of the given entity(s) Arguments: {new entity name} {entity_name} / {class_name} / no argument picks wh
ent_show_response_criteria // an entitys current criteria set used to select responses. Arguments: {entity_name} / {class_name} /
ent_step // When ent_pause is set this will step through one waiting input / output message at a time.
ent_teleport // Teleport the specified entity to where the player is looking. Format: ent_teleport <entity name>
ent_text // Displays text debugging information about the given entity(ies) on top of the entity (See Overlay Text) Arguments: {entity_
ent_viewoffset // Displays the eye position for the given entity(ies) in red. Arguments: {entity_name} / {class_name} / no argument picks wha
envmap
escape // Escape key pressed.
exec // Execute script file.
exit // Exit the engine.
explode // Kills the player with explosive damage
explodevector // Kills a player applying an explosive force. Usage: explodevector <player> <x value> <y value> <z value>
fadein // fadein {time r g b}: Fades the screen in from black or from the specified color over the given number of seconds.
fadeout // fadeout {time r g b}: Fades the screen to black or to the specified color over the given number of seconds.
fast_fogvolume "0"
filesystem_buffer_size "0" // Size of per file buffers. 0 for none
filesystem_max_stdio_read "16"
filesystem_native "1" // Use native FS or STDIO
filesystem_report_buffered_io "0"
filesystem_unbuffered_io "1"
filesystem_use_overlapped_io "1"
find // Find concommands with the specified string in their name/help text.
findflags // Find concommands by flags.
find_ent // Find and list all entities with classnames or targetnames that contain the specified substring. Format: find_ent <substring>
find_ent_index // Display data for entity matching specified index. Format: find_ent_index <index>
firetarget
fire_absorbrate "3"
fire_dmgbase "1"
fire_dmginterval "1"
fire_dmgscale "0"
fire_extabsorb "5"
fire_extscale "12"
fire_growthrate "1"
fire_heatscale "1"
fire_incomingheatscale "0"
fire_maxabsorb "50"
firstperson // Switch to firstperson camera.
fish_debug "0" // Show debug info for fish
fish_dormant "0" // Turns off interactive fish behavior. Fish become immobile and unresponsive.
flex_expression "0"
flex_looktime "5"
flex_maxawaytime "1"
flex_maxplayertime "7"
flex_minawaytime "0"
flex_minplayertime "5"
flex_rules "1" // Allow flex animation rules to run.
flex_smooth "1" // Applies smoothing/decay curve to flex animation controller changes.
flex_talk "0"
flush // Flush unlocked cache memory.
flush_locked // Flush unlocked and locked cache memory.
fogui // Show/hide fog control UI.
fog_color "-1"
fog_colorskybox "-1"
fog_enable "1"
fog_enableskybox "1"
fog_enable_water_fog "1"
fog_end "-1"
fog_endskybox "-1"
fog_maxdensity "-1"
fog_maxdensityskybox "-1"
fog_override "0"
fog_start "-1"
fog_startskybox "-1"
force_centerview
fov // Change players FOV
fps_max "300" // cannot be set while connected to a server.
free_pass_peek_debug "0"
fs_monitor_read_from_pack "0" // 2:Sync only
fs_printopenfiles // Show all files currently opened by the engine.
fs_report_sync_opens "0" // 2:Not during load
fs_warning_level // Set the filesystem warning level.
fs_warning_mode "0" // 2:Warn other threads
func_breakdmg_bullet "0"
func_breakdmg_club "1"
func_breakdmg_explosive "1"
func_break_max_pieces "15"
func_break_reduction_factor "0"
g15_dumpplayer // Spew player data.
g15_reload // Reloads the Logitech G-15 Keyboard configs.
g15_update_msec "250" // Logitech G-15 Keyboard update interval.
gamemenucommand // Issue  game menu command.
gameui_activate // Shows the game UI
gameui_allowescape // Escape key allowed to hide game UI
gameui_allowescapetoshow // Escape key allowed to show game UI
gameui_hide // Hides the game UI
gameui_hide_dialog // asdf
gameui_preventescape // Escape key doesnt hide game UI
gameui_preventescapetoshow // Escape key doesnt show game UI
gameui_show_dialog // Show an arbitrary Dialog.
gameui_xbox "0"
getpos // dump position and angles to the  console
give // Give item to player. Arguments: <item_name>
givecurrentammo // Give a supply of ammo for current weapon..
global_set // 2 = DEAD).
gl_amd_occlusion_workaround "1"
gl_clear "0"
gl_clear_randomcolor "0" // Clear the back buffer to random colors every frame. Helps spot open seams in geometry.
god // Toggle. Player becomes invulnerable.
groundlist // Display ground entity list <index>
g_debug_angularsensor "0"
g_debug_constraint_sounds "0" // Enable debug printing about constraint sounds.
g_debug_doors "0"
g_debug_npc_vehicle_roles "0"
g_debug_ragdoll_removal "0"
g_debug_ragdoll_visualize "0"
g_debug_trackpather "0"
g_debug_transitions "0" // Set to 1 and restart the map to be warned if the map has no trigger_transition volumes. Set to 2 to see a dump of all entities
g_debug_vehiclebase "0"
g_debug_vehicledriver "0"
g_debug_vehicleexit "0"
g_debug_vehiclesound "0"
g_jeepexitspeed "100"
g_Language "0"
g_ragdoll_fadespeed "600"
g_ragdoll_important_maxcount "2"
g_ragdoll_lvfadespeed "100"
g_ragdoll_maxcount "8"
hammer_update_entity // Updates the entitys position/angles when in edit mode
hammer_update_safe_entities // Updates entities in the map that can safely be updated (dont have parents or are affected by constraints). Also excludes entit
hap_damagescale_game "1"
hap_HasDevice "0" // falcon is connected
hap_melee_scale "0"
hap_noclip_avatar_scale "0"
hap_ui_vehicles "0"
heartbeat // Force heartbeat of master servers
help // Find help about a convar/concommand.
hideconsole // Hide the console.
hidehud "0"
hidepanel // Hides a viewport panel <name>
hideradar // Hides HUD radar
hl2_episodic "0"
hostage_debug "0" // Show hostage AI debug information
hostip "0" // Host game server ip
hostname "0" // Hostname for server.
hostport "27015" // Host game server port
host_flush_threshold "20" // Memory threshold below which the host should flush caches between server instances
host_framerate "0" // Set to lock per-frame time elapse.
host_limitlocal "0" // Apply cl_cmdrate and cl_updaterate to loopback connection
host_map "0" // Current map name.
host_profile "0"
host_runofftime // Run off some time without rendering/updating sounds
host_showcachemiss "0" // Print a debug message when the client or server cache is missed.
host_ShowIPCCallCount "0" // the # of IPC calls is shown every frame.
host_sleep "0" // Force the host to sleep a certain number of milliseconds each frame.
host_speeds "0" // Show general system running times.
host_thread_mode "0" // 2 == force)
host_timer_report // Spew CPU timer jitter for the last 128 frames in microseconds (dedicated only)
host_timer_spin_ms "0" // Use CPU busy-loop for improved timer precision (dedicated only)
host_timescale "1" // Prescale the clock by this amount.
host_writeconfig // Store current settings to config.cfg (or specified .cfg file).
hud_achievement_count "5" // Max number of achievements that can be shown on the HUD
hud_achievement_description "1" // Show full descriptions of achievements on the HUD
hud_achievement_glowtime "2" // Duration of glow effect around incremented achievements
hud_achievement_tracker "1" // Show or hide the achievement tracker
hud_autoaim_method "1"
hud_autoaim_scale_icon "0"
hud_autoreloadscript "0" // Automatically reloads the animation script each time one is ran
hud_classautokill "1" // Automatically kill player after choosing a new playerclass.
hud_deathnotice_time "6"
hud_drawhistory_time "5"
hud_draw_active_reticle "0"
hud_draw_fixed_reticle "0"
hud_fastswitch "0"
hud_freezecamhide "0" // Hide the HUD during freeze-cam
hud_jeephint_numentries "10"
hud_magnetism "0"
hud_reloadscheme // Reloads hud layout and animation scripts.
hud_reticle_alpha_speed "700"
hud_reticle_maxalpha "255"
hud_reticle_minalpha "125"
hud_reticle_scale "1"
hud_saytext_time "12"
hud_showtargetid "1" // Enables display of target names
hud_showtargetpos "0" // 4: lower right
hud_takesshots "0" // Auto-save a scoreboard screenshot at the end of a map.
hurtme // Hurts the player. Arguments: <health to lose>
impulse
incrementvar // Increment specified convar value.
invnext
invprev
in_usekeyboardsampletime "1" // Use keyboard sample time smoothing.
ip "0" // Overrides IP for multihomed hosts
joyadvancedupdate
joystick "0"
joy_accelmax "1"
joy_accelscale "0"
joy_accel_filter "0"
joy_active "-1" // Which of the connected joysticks / gamepads to use (-1 means first found)
joy_advanced "0"
joy_advaxisr "0"
joy_advaxisu "0"
joy_advaxisv "0"
joy_advaxisx "0"
joy_advaxisy "0"
joy_advaxisz "0"
joy_autoaimdampen "0" // How much to scale user stick input when the gun is pointing at a valid target.
joy_autoaimdampenrange "0" // The stick range where autoaim dampening is applied. 0 = off
joy_autosprint "0" // Automatically sprint when moving with an analog joystick
joy_axisbutton_threshold "0" // Analog axis range before a button press is registered.
joy_axis_deadzone "0" // Dead zone near the zero point to not report movement.
joy_diagonalpov "0" // too.
joy_display_input "0"
joy_forwardsensitivity "-1"
joy_forwardthreshold "0"
joy_gamecontroller_config "0" // can also be configured in Steam Big Picture mode.
joy_inverty "0" // Whether to invert the Y axis of the joystick for looking.
joy_inverty_default "0"
joy_lowend "1"
joy_lowmap "1"
joy_movement_stick "0" // Which stick controls movement (0 is left stick)
joy_movement_stick_default "0"
joy_name "0"
joy_pegged "0"
joy_pitchsensitivity "1"
joy_pitchsensitivity_default "-1"
joy_pitchthreshold "0"
joy_response_look "0" // 1=Acceleration Promotion
joy_response_move "1" // 1/sensitivity
joy_response_move_vehicle "6"
joy_sidesensitivity "1"
joy_sidethreshold "0"
joy_vehicle_turn_lowend "0"
joy_vehicle_turn_lowmap "0"
joy_virtual_peg "0"
joy_wingmanwarrior_turnhack "0" // Wingman warrior hack related to turn axes.
joy_xcontroller_cfg_loaded "0" // the 360controller.cfg file will be executed on startup & option changes.
joy_xcontroller_found "0" // Automatically set to 1 if an xcontroller has been detected.
joy_yawsensitivity "-1"
joy_yawsensitivity_default "-1"
joy_yawthreshold "0"
jpeg // Take a jpeg screenshot: jpeg <filename> <quality 1-100>.
jpeg_quality "90" // jpeg screenshot quality.
kdtree_test // Tests spatial partition for entities queries.
key_findbinding // Find key bound to specified command string.
key_listboundkeys // List bound keys with bindings.
key_updatelayout // Updates  game keyboard layout to current windows keyboard setting.
kick // Kick a player by name.
kickall // Kicks everybody connected with a message.
kickid // with a message.
kill // Kills the player with generic damage
killserver // Shutdown the server.
killvector // Kills a player applying force. Usage: killvector <player> <x value> <y value> <z value>
lastinv
lightcache_maxmiss "2"
lightprobe // Samples the lighting environment. Creates a cubemap and a file indicating the local lighting in a subdirectory called material
light_crosshair // Show texture color at crosshair
linefile // Parses map leak data from .lin file
listdemo // List demo file contents.
listid // Lists banned users.
listip // List IP addresses on the ban list.
listissues // List all the issues that can be voted on.
listmodels // List loaded models.
listRecentNPCSpeech // Displays a list of the last 5 lines of speech from NPCs.
load // Load a saved game.
loadcommentary
loader_dump_table
loader_spew_info "0" // -1:All
loader_spew_info_ex "0" // (internal)
lod_TransitionDist "800"
log // and udp < on | off >.
logaddress_add // Set address and port for remote host <ip:port>.
logaddress_del // Remove address and port for remote host <ip:port>.
logaddress_delall // Remove all udp addresses being logged to
logaddress_list // List all addresses currently being used by logaddress.
log_verbose_enable "0" // Set to 1 to enable verbose server log on the server.
log_verbose_interval "3" // Determines the interval (in seconds) for the verbose server log.
lookspring "0"
lookstrafe "0"
lservercfgfile "0"
map // Start playing on specified map.
mapcyclefile "0" // Name of the .txt file used to cycle the maps on multiplayer servers
maps // Displays list of maps.
map_background // Runs a map as the background to the main menu.
map_commentary // on a specified map.
map_edit
map_noareas "0" // Disable area to area connection testing.
map_setbombradius // Sets the bomb radius for the map.
map_showbombradius // Shows bomb radius from the center of each bomb site and planted bomb.
map_showspawnpoints // Shows player spawn points (red=invalid)
matchmakingport "27025" // Host Matchmaking port
mat_aaquality "0"
mat_accelerate_adjust_exposure_down "3"
mat_alphacoverage "1"
mat_antialias "4"
mat_autoexposure_max "2"
mat_autoexposure_min "0"
mat_bloomamount_rate "0"
mat_bloomscale "1"
mat_bloom_scalefactor_scalar "1"
mat_bufferprimitives "1"
mat_bumpbasis "0"
mat_bumpmap "1"
mat_camerarendertargetoverlaysize "256"
mat_clipz "1"
mat_colcorrection_disableentities "0" // Disable map color-correction entities
mat_colorcorrection "0"
mat_color_projection "0"
mat_compressedtextures "1"
mat_configcurrent // show the current video control panel config for the material system
mat_crosshair // Display the name of the material under the crosshair
mat_crosshair_edit // open the material under the crosshair in the editor defined by mat_crosshair_edit_editor
mat_crosshair_explorer // open the material under the crosshair in explorer and highlight the vmt file
mat_crosshair_printmaterial // print the material under the crosshair
mat_crosshair_reloadmaterial // reload the material under the crosshair
mat_debugalttab "0"
mat_debugdepth "0"
mat_debugdepthmode "0"
mat_debugdepthval "128"
mat_debugdepthvalmax "256"
mat_debug_autoexposure "0"
mat_debug_bloom "0"
mat_debug_postprocessing_effects "0" // 2 = only apply post-processing to the centre of the screen
mat_debug_process_halfscreen "0"
mat_depthbias_decal "-262144"
mat_depthbias_normal "0"
mat_depthbias_shadowmap "0"
mat_diffuse "1"
mat_disablehwmorph "0" // Disables HW morphing for particular mods
mat_disable_bloom "0"
mat_disable_d3d9ex "0" // Disables Windows Aero DirectX extensions (may positively or negatively affect performance depending on video drivers)
mat_disable_fancy_blending "0"
mat_disable_lightwarp "0"
mat_disable_ps_patch "0"
mat_drawflat "0"
mat_drawTexture "0" // Enable debug view texture
mat_drawTextureScale "1" // Debug view texture scale
mat_drawTitleSafe "0" // Enable title safe overlay
mat_drawwater "1"
mat_dump_rts "0"
mat_dxlevel "95"
mat_dynamic_tonemapping "1"
mat_edit // Bring up the material under the crosshair in the editor
mat_enable_vrmode // Switches the material system to VR mode (after restart)
mat_envmapsize "128"
mat_envmaptgasize "32"
mat_excludetextures "0"
mat_exposure_center_region_x "0"
mat_exposure_center_region_x_flashlight "0"
mat_exposure_center_region_y "0"
mat_exposure_center_region_y_flashlight "0"
mat_fastclip "0"
mat_fastnobump "0"
mat_fastspecular "1" // Enable/Disable specularity for visual testing. Will not reload materials and will not affect perf.
mat_fillrate "0"
mat_filterlightmaps "1"
mat_filtertextures "1"
mat_forceaniso "8"
mat_forcedynamic "0"
mat_forcehardwaresync "1"
mat_forcemanagedtextureintohardware "0"
mat_force_bloom "0"
mat_force_ps_patch "0"
mat_force_tonemap_scale "0"
mat_framebuffercopyoverlaysize "256"
mat_frame_sync_enable "1"
mat_frame_sync_force_texture "0" // Force frame syncing to lock a managed texture.
mat_fullbright "0"
mat_hdr_enabled // Report if HDR is enabled for debugging
mat_hdr_level "2" // and 2 for full HDR on HDR maps.
mat_hdr_manual_tonemap_rate "1"
mat_hdr_tonemapscale "1" // 16 = eyes wide open.
mat_hdr_uncapexposure "0"
mat_hsv "0"
mat_info // Shows material system info
mat_leafvis "0" // Draw wireframe of current leaf
mat_levelflush "1"
mat_lightmap_pfms "0" // Outputs .pfm files containing lightmap data for each lightmap page when a level exits.
mat_loadtextures "1"
mat_luxels "0"
mat_managedtextures "1" // allows Direct3D to manage texture uploading at the cost of extra system memory
mat_maxframelatency "1"
mat_max_worldmesh_vertices "65536"
mat_measurefillrate "0"
mat_mipmaptextures "1"
mat_monitorgamma "2" // monitor gamma (typically 2.2 for CRT and 1.7 for LCD)
mat_monitorgamma_tv_enabled "0"
mat_monitorgamma_tv_exp "2"
mat_monitorgamma_tv_range_max "255"
mat_monitorgamma_tv_range_min "16"
mat_morphstats "0"
mat_motion_blur_enabled "1"
mat_motion_blur_falling_intensity "1"
mat_motion_blur_falling_max "20"
mat_motion_blur_falling_min "10"
mat_motion_blur_forward_enabled "0"
mat_motion_blur_percent_of_screen_max "4"
mat_motion_blur_rotation_intensity "1"
mat_motion_blur_strength "1"
mat_non_hdr_bloom_scalefactor "0"
mat_norendering "0"
mat_normalmaps "0"
mat_normals "0"
mat_parallaxmap "0"
mat_phong "1"
mat_picmip "0"
mat_postprocessing_combine "1" // software anti-aliasing and color correction into one post-processing pass
mat_postprocess_x "4"
mat_postprocess_y "1"
mat_powersavingsmode "0" // Power Savings Mode
mat_proxy "0"
mat_queue_mode "0" // 2=queued multithreaded
mat_queue_report "0" // Report thread stalls. Positive number will filter by stalls >= time in ms. -1 reports all locks.
mat_reducefillrate "0"
mat_reduceparticles "0"
mat_reloadallmaterials // Reloads all materials
mat_reloadmaterial // Reloads a single material
mat_reloadtextures // Reloads all textures
mat_remoteshadercompile "127"
mat_reporthwmorphmemory // Reports the amount of size in bytes taken up by hardware morph textures.
mat_report_queue_status "0"
mat_reset_rendertargets // Resets all the render targets
mat_reversedepth "0"
mat_savechanges // saves current video configuration to the registry
mat_setvideomode // windowed state of the material system
mat_shadercount // display count of all shaders and reset that count
mat_shadowstate "1"
mat_showcamerarendertarget "0"
mat_showenvmapmask "0"
mat_showframebuffertexture "0"
mat_showlightmappage "-1"
mat_showlowresimage "0"
mat_showmaterials // Show materials.
mat_showmaterialsverbose // Show materials (verbose version).
mat_showmiplevels "0" // 1: everything else
mat_showtextures // Show used textures.
mat_showwatertextures "0"
mat_show_ab_hdr "0"
mat_show_ab_hdr_hudelement "0" // HDR Demo HUD Element toggle.
mat_show_histogram "0"
mat_show_texture_memory_usage "0" // Display the texture memory usage on the HUD.
mat_slopescaledepthbias_decal "0"
mat_slopescaledepthbias_normal "0"
mat_slopescaledepthbias_shadowmap "16"
mat_softwarelighting "0"
mat_softwareskin "0"
mat_software_aa_blur_one_pixel_lines "0" // (1.0 - lots)
mat_software_aa_debug "0" // (2 - show anti-a
mat_software_aa_edge_threshold "1" // Software AA - adjusts the sensitivity of the software AA shaders edge detection (default 1.0 - a lower value will soften more
mat_software_aa_quality "0" // (1 - 9-tap filter)
mat_software_aa_strength "0" // Software AA - perform a software anti-aliasing post-process (an alternative/supplement to MSAA). This value sets the strength o
mat_software_aa_strength_vgui "1" // but forced to this value when called by the post vgui AA pass.
mat_software_aa_tap_offset "1" // Software AA - adjusts the displacement of the taps used by the software AA shader (default 1.0 - a lower value will make the im
mat_specular "1" // Enable/Disable specularity for perf testing. Will cause a material reload upon change.
mat_spewvertexandpixelshaders // Print all vertex and pixel shaders currently loaded to the  console
mat_spew_on_texture_size "0" // Print warnings about vtf content that isnt of the expected size
mat_stub "0"
mat_supportflashlight "1" // 1 - flashlight is supported
mat_surfaceid "0"
mat_surfacemat "0"
mat_texture_limit "-1" // the material system will limit the amount of texture memory it uses in a frame. Useful for identifying
mat_texture_list "0" // show a list of used textures per frame
mat_texture_list_all "0" // then the texture list panel will show all currently-loaded textures.
mat_texture_list_content_path "0" // itll assume your content directory is next to the currently runn
mat_texture_list_txlod // -1 to dec resolution
mat_texture_list_txlod_sync // save - saves all changes to material content files
mat_texture_list_view "1" // then the texture list panel will render thumbnails of currently-loaded textures.
mat_texture_save_fonts // Save all font textures
mat_tonemapping_occlusion_use_stencil "0"
mat_tonemap_algorithm "1" // 0 = Original Algorithm 1 = New Algorithm
mat_tonemap_min_avglum "3"
mat_tonemap_percent_bright_pixels "2"
mat_tonemap_percent_target "60"
mat_trilinear "0"
mat_use_compressed_hdr_textures "1"
mat_viewportscale "1" // Scale down the main viewport (to reduce GPU impact on CPU profiling)
mat_viewportupscale "1" // Scale the viewport back up
mat_visualize_dof "0"
mat_vrmode_adapter "-1"
mat_vsync "0" // Force sync to vertical retrace
mat_wateroverlaysize "256"
mat_wireframe "0"
mat_yuv "0"
maxplayers // Change the maximum number of players allowed on this server.
memory // Print memory stats.
memory_diff // show memory stats relative to snapshot
memory_list // dump memory list (linux only)
memory_mark // snapshot current allocation status
memory_status // show memory stats (linux only)
mem_compact
mem_dump // Dump memory stats to text file.
mem_dumpstats "0" // Dump current and max heap usage info to console at end of frame ( set to 2 for continuous output )
mem_dumpvballocs // Dump VB memory allocation stats.
mem_eat
mem_force_flush "0" // Force cache flush of unlocked resources on every alloc
mem_max_heapsize "256" // Maximum amount of memory to dedicate to engine hunk and datacache (in mb)
mem_max_heapsize_dedicated "64" // for dedicated server (in mb)
mem_min_heapsize "48" // Minimum amount of memory to dedicate to engine hunk and datacache (in mb)
mem_periodicdumps "0" // Write periodic memstats dumps every n seconds.
mem_test
mem_test_each_frame "0" // Run heap check at end of every frame
mem_test_every_n_seconds "0" // Run heap check at a specified interval
mem_vcollide // Dumps the memory used by vcollides
menuselect // menuselect
minisave // Saves  game (for current level only!)
mission_list // List all available tactical missions
mission_show // Show the given mission
mm_add_item // Add a stats item
mm_add_player // Add a player
mm_max_spectators "4" // Max players allowed on the spectator team
mm_message // Send a message to all remote clients
mm_minplayers "2" // Number of players required to start an unranked game
mm_select_session // Select a session
mm_session_info // Dump session information
mm_stats
model_list // Dump model list to file
mod_forcedata "1" // Forces all model file data into cache on model load.
mod_forcetouchdata "1" // Forces all model file data into cache on model load.
mod_load_anims_async "0"
mod_load_fakestall "0" // Forces all ANI file loading to stall for specified ms
mod_load_mesh_async "0"
mod_load_showstall "0" // 2 - show stalls
mod_load_vcollide_async "0"
mod_lock_mdls_on_load "0"
mod_test_mesh_not_available "0"
mod_test_not_available "0"
mod_test_verts_not_available "0"
mod_touchalldata "1" // Touch model data during level startup
mod_trace_load "0"
motdfile "0" // The MOTD file to load.
motdfile_text "0" // The text-only MOTD file to use for clients that have disabled HTML MOTDs.
movie_fixwave // etc.
mp_allowNPCs "1"
mp_allowspectators "1" // toggles whether the server allows spectator mode or not
mp_autocrosshair "1"
mp_autokick "1" // Kick idle/team-killing players
mp_autoteambalance "1"
mp_bonusroundtime "15" // Time after round win until round restarts
mp_buytime "1" // How many minutes after round start players can buy items for.
mp_c4timer "45" // how long from when the C4 is armed until it blows
mp_chattime "10" // amount of time players can chat after the game is over
mp_clan_readyrestart "0" // game will restart once someone from each team gives the ready signal
mp_clan_ready_signal "0" // Text that team leader from each team must speak for the match to begin
mp_decals "200"
mp_defaultteam "0"
mp_disable_autokick // Prevents a userid from being auto-kicked
mp_disable_respawn_times "0"
mp_dump_timers // Prints round timers to the  console for debugging
mp_enableroundwaittime "1" // Enable timers to wait between rounds.
mp_fadetoblack "0" // fade a players screen to black when he dies
mp_falldamage "0"
mp_flashlight "0"
mp_footsteps "1"
mp_forceautoteam "0" // Automatically assign players to teams when joining.
mp_forcecamera "1" // Restricts spectator modes for dead players
mp_forcerespawn "1"
mp_forcerespawnplayers // Force all players to respawn.
mp_forcewin // Forces team to win
mp_fraglimit "0" // The number of kills at which the map ends
mp_freezetime "6" // how many seconds to keep players frozen when the round starts
mp_friendlyfire "0" // Allows team members to injure other members of their team
mp_holiday_nogifts "0" // Set to 1 to prevent holiday gifts from spawning when players are killed.
mp_hostagepenalty "13" // Terrorist are kicked for killing too much hostages
mp_humanteam "0" // T}
mp_ignore_round_win_conditions "0" // Ignore conditions which would end the current round
mp_limitteams "2" // Max # of players 1 team can have over another (0 disables check)
mp_logdetail "0" // 3=both)
mp_mapcycle_empty_timeout_seconds "0" // server will cycle to the next map if it has been empty on the current map for N seconds
mp_match_end_at_timelimit "0" // Allow the match to end when mp_timelimit hits instead of waiting for the end of the current round.
mp_maxrounds "0" // max number of rounds to play before server changes maps
mp_playerid "0" // Controls what information player see in the status bar: 0 all names; 1 team names; 2 no names
mp_playerid_delay "0" // Number of seconds to delay showing information in the status bar
mp_playerid_hold "0" // Number of seconds to keep showing old information in the status bar
mp_respawnwavetime "10" // Time between respawn waves.
mp_restartgame "0" //  game will restart in the specified number of seconds
mp_restartgame_immediate "0" // game will restart immediately
mp_restartround "0" // the current round will restart in the specified number of seconds
mp_roundtime "2" // How many minutes each round takes.
mp_round_restart_delay "5" // Number of seconds to delay before restarting a round after a win
mp_scrambleteams // Scramble the teams and restart the game
mp_scrambleteams_auto "1" // Server will automatically scramble the teams if criteria met. Only works on dedicated servers.
mp_scrambleteams_auto_windifference "2" // Number of round wins a team must lead by in order to trigger an auto scramble.
mp_show_voice_icons "1" // Show overhead player voice icons when players are speaking.
mp_spawnprotectiontime "5" // Kick players who team-kill within this many seconds of a round restart.
mp_stalemate_enable "0" // Enable/Disable stalemate mode.
mp_stalemate_meleeonly "0" // Restrict everyone to melee weapons only while in Sudden Death.
mp_stalemate_timelimit "240" // Timelimit (in seconds) of the stalemate round.
mp_startmoney "800" // amount of money each player gets when they reset
mp_switchteams // Switch teams and restart the game
mp_teamlist "0"
mp_teamoverride "1"
mp_teamplay "0"
mp_teams_unbalance_limit "1" // Teams are unbalanced when one team has this many more players than the other team. (0 disables check)
mp_timelimit "0" // game time per map in minutes
mp_tkpunish "0" // 1=yes}
mp_tournament "0"
mp_tournament_allow_non_admin_restart "1" // Allow mp_tournament_restart command to be issued by players other than admin.
mp_tournament_restart // Restart Tournament Mode on the current level.
mp_usehwmmodels "0" // 0 = based upon GPU)
mp_usehwmvcds "0" // 0 = based upon GPU)
mp_waitingforplayers_cancel "0" // Set to 1 to end the WaitingForPlayers period.
mp_waitingforplayers_restart "0" // Set to 1 to start or restart the WaitingForPlayers period.
mp_waitingforplayers_time "0" // WaitingForPlayers time length in seconds
mp_weaponstay "0"
mp_winlimit "0" // Max score one team can reach before server changes maps
multvar // Multiply specified convar value.
muzzleflash_light "1"
m_customaccel "0" // Custom mouse acceleration: 0: custom accelaration disabled 1: mouse_acceleration = min(m_customaccel_max, pow(raw_mouse_delta,
m_customaccel_exponent "1" // Mouse move is raised to this power before being scaled by scale factor.
m_customaccel_max "0" // 0 for no limit
m_customaccel_scale "0" // Custom mouse acceleration value.
m_filter "0" // Mouse filtering (set this to 1 to average the mouse over 2 frames).
m_forward "1" // Mouse forward factor.
m_mouseaccel1 "0" // Windows mouse acceleration initial threshold (2x movement).
m_mouseaccel2 "0" // Windows mouse acceleration secondary threshold (4x movement).
m_mousespeed "1" // 2 to enable secondary threshold
m_pitch "0" // Mouse pitch factor.
m_rawinput "0" // Use Raw Input for mouse input.
m_side "0" // Mouse side factor.
m_yaw "0" // Mouse yaw factor.
name "0" // Current user name
nav_add_to_selected_set // Add current area to the selected set.
nav_add_to_selected_set_by_id // Add specified area id to the selected set.
nav_analyze // Re-analyze the current Navigation Mesh and save it to disk.
nav_area_bgcolor "0" // RGBA color to draw as the background color for nav areas while editing.
nav_area_max_size "50" // Max area size created in nav generation
nav_avoid // Toggles the avoid this area when possible flag used by the AI system.
nav_begin_area // drag the opposite corner to the desired location and
nav_begin_deselecting // Start continuously removing from the selected set.
nav_begin_drag_deselecting // Start dragging a selection area.
nav_begin_drag_selecting // Start dragging a selection area.
nav_begin_selecting // Start continuously adding to the selected set.
nav_begin_shift_xy // Begin shifting the Selected Set.
nav_build_ladder // Attempts to build a nav ladder on the climbable surface under the cursor.
nav_check_connectivity // Checks to be sure every (or just the marked) nav area can get to every goal area for the map (hostages or bomb site).
nav_check_file_consistency // Scans the maps directory and reports any missing/out-of-date navigation files.
nav_check_floor // Updates the blocked/unblocked status for every nav area.
nav_check_stairs // Update the nav mesh STAIRS attribute
nav_chop_selected // Chops all selected areas into their component 1x1 areas
nav_clear_attribute // Remove given nav attribute from all areas in the selected set.
nav_clear_selected_set // Clear the selected set.
nav_clear_walkable_marks // Erase any previously placed walkable positions.
nav_compress_id // Re-orders area and ladder IDs so they are continuous.
nav_connect // then invoke the connect command. Note that this creates a
nav_coplanar_slope_limit "0"
nav_coplanar_slope_limit_displacement "0"
nav_corner_adjust_adjacent "18" // radius used to raise/lower corners in nearby areas when raising/lowering corners.
nav_corner_lower // Lower the selected corner of the currently marked Area.
nav_corner_place_on_ground // Places the selected corner of the currently marked Area on the ground.
nav_corner_raise // Raise the selected corner of the currently marked Area.
nav_corner_select // Select a corner of the currently marked Area. Use multiple times to access all four corners.
nav_create_area_at_feet "0" // Anchor nav_begin_area Z to editing players feet
nav_create_place_on_ground "0" // nav areas will be placed flush with the ground when created by hand.
nav_crouch // Toggles the must crouch in this area flag used by the AI system.
nav_debug_blocked "0"
nav_delete // Deletes the currently highlighted Area.
nav_delete_marked // Deletes the currently marked Area (if any).
nav_disconnect // then invoke the disconnect command. This will remove all connec
nav_disconnect_outgoing_oneways // disconnect all outgoing one-way connections.
nav_displacement_test "10000" // Checks for nodes embedded in displacements (useful for in-development maps)
nav_dont_hide // Toggles the area is not suitable for hiding spots flag used by the AI system.
nav_drag_selection_volume_zmax_offset "32" // The offset of the nav drag volume top from center
nav_drag_selection_volume_zmin_offset "32" // The offset of the nav drag volume bottom from center
nav_draw_limit "500" // The maximum number of areas to draw in edit mode
nav_dump_selected_set_positions // z) coordinates of the centers of all selected nav areas to a file.
nav_edit "0" // Set to one to interactively edit the Navigation Mesh. Set to zero to leave edit mode.
nav_end_area // Defines the second corner of a new Area or Ladder and creates it.
nav_end_deselecting // Stop continuously removing from the selected set.
nav_end_drag_deselecting // Stop dragging a selection area.
nav_end_drag_selecting // Stop dragging a selection area.
nav_end_selecting // Stop continuously adding to the selected set.
nav_end_shift_xy // Finish shifting the Selected Set.
nav_flood_select // use this command again.
nav_generate // Generate a Navigation Mesh for the current map and save it to disk.
nav_generate_fencetops "1" // Autogenerate nav areas on fence and obstacle tops
nav_generate_fixup_jump_areas "1" // Convert obsolete jump areas into 2-way connections
nav_generate_incremental // Generate a Navigation Mesh for the current map and save it to disk.
nav_generate_incremental_range "2000"
nav_generate_incremental_tolerance "0" // Z tolerance for adding new nav areas.
nav_gen_cliffs_approx // post-processing approximation
nav_jump // Toggles the traverse this area by jumping flag used by the AI system.
nav_ladder_flip // Flips the selected ladders direction.
nav_load // Loads the Navigation Mesh for the current map.
nav_lower_drag_volume_max // Lower the top of the drag select volume.
nav_lower_drag_volume_min // Lower the bottom of the drag select volume.
nav_make_sniper_spots // Chops the marked area into disconnected sub-areas suitable for sniper spots.
nav_mark // Marks the Area or Ladder under the cursor for manipulation by subsequent editing commands.
nav_mark_attribute // Set nav attribute for all areas in the selected set.
nav_mark_unnamed // Mark an Area with no Place name. Useful for finding stray areas missed when Place Painting.
nav_mark_walkable // Mark the current location as a walkable position. These positions are used as seed locations when sampling the map to generate
nav_max_view_distance "6000" // Maximum range for precomputed nav mesh visibility (0 = default 1500 units)
nav_max_vis_delta_list_length "64"
nav_merge // and invoke the merge comm
nav_merge_mesh // Merges a saved selected set into the current mesh.
nav_no_hostages // Toggles the hostages cannot use this area flag used by the AI system.
nav_no_jump // Toggles the dont jump in this area flag used by the AI system.
nav_place_floodfill // and flood-fills the Place to all adjacent Areas. Flood-filli
nav_place_list // Lists all place names used in the map.
nav_place_pick // Sets the current Place to the Place of the Area under the cursor.
nav_place_replace // Replaces all instances of the first place with the second place.
nav_place_set // Sets the Place of all selected areas to the current Place.
nav_potentially_visible_dot_tolerance "0"
nav_precise // Toggles the dont avoid obstacles flag used by the AI system.
nav_quicksave "1" // Set to one to skip the time consuming phases of the analysis. Useful for data collection and testing.
nav_raise_drag_volume_max // Raise the top of the drag select volume.
nav_raise_drag_volume_min // Raise the bottom of the drag select volume.
nav_recall_selected_set // Re-selects the stored selected set.
nav_remove_from_selected_set // Remove current area from the selected set.
nav_remove_jump_areas // replacing them with connections.
nav_restart_after_analysis "1" // but is useful for increm
nav_run // Toggles the traverse this area by running flag used by the AI system.
nav_save // Saves the current Navigation Mesh to disk.
nav_save_selected // Writes the selected set to disk for merging into another mesh via nav_merge_mesh.
nav_selected_set_border_color "100" // Color used to draw the selected set borders while editing.
nav_selected_set_color "255" // Color used to draw the selected set background while editing.
nav_select_blocked_areas // Adds all blocked areas to the selected set
nav_select_damaging_areas // Adds all damaging areas to the selected set
nav_select_half_space // Selects any areas that intersect the given half-space.
nav_select_invalid_areas // Adds all invalid areas to the Selected Set.
nav_select_larger_than // Select nav areas where both dimensions are larger than the given size.
nav_select_obstructed_areas // Adds all obstructed areas to the selected set
nav_select_orphans // Adds all orphan areas to the selected set (highlight a valid area first).
nav_select_overlapping // Selects nav areas that are overlapping others.
nav_select_radius // Adds all areas in a radius to the selection set
nav_select_stairs // Adds all stairway areas to the selected set
nav_set_place_mode // Sets the editor into or out of Place mode. Place mode allows labelling of Area with Place names.
nav_shift // Shifts the selected areas by the specified amount
nav_show_approach_points "0" // Show Approach Points in the Navigation Mesh.
nav_show_area_info "0" // Duration in seconds to show nav area ID and attributes while editing
nav_show_compass "0"
nav_show_continguous "0" // Highlight non-contiguous connections
nav_show_danger "0" // Show current danger levels.
nav_show_dumped_positions // z) coordinate positions of the given dump file.
nav_show_func_nav_avoid "0" // Show areas of designer-placed bot avoidance due to func_nav_avoid entities
nav_show_func_nav_prefer "0" // Show areas of designer-placed bot preference due to func_nav_prefer entities
nav_show_func_nav_prerequisite "0" // Show areas of designer-placed bot preference due to func_nav_prerequisite entities
nav_show_light_intensity "0"
nav_show_nodes "0"
nav_show_node_grid "0"
nav_show_node_id "0"
nav_show_player_counts "0" // Show current player counts in each area.
nav_show_potentially_visible "0" // Show areas that are potentially visible from the current nav area
nav_simplify_selected // Chops all selected areas into their component 1x1 areas and re-merges them together into larger areas
nav_slope_limit "0" // The ground unit normals Z component must be greater than this for nav areas to be generated.
nav_slope_tolerance "0" // The ground unit normals Z component must be this close to the nav areas Z component to be generated.
nav_snap_to_grid "0" // Snap to the nav generation grid when creating new nav areas
nav_solid_props "0" // Make props solid to nav generation/editing
nav_splice // connected area between them.
nav_split // align the split line using your cursor and invoke the split command.
nav_split_place_on_ground "0" // nav areas will be placed flush with the ground when split.
nav_stand // Toggles the stand while hiding flag used by the AI system.
nav_stop // Toggles the must stop when entering this area flag used by the AI system.
nav_store_selected_set // Stores the current selected set for later retrieval.
nav_strip // and Encounter Spots from the current Area.
nav_subdivide // Subdivides all selected areas.
nav_test_node "0"
nav_test_node_crouch "0"
nav_test_node_crouch_dir "4"
nav_test_stairs // Test the selected set for being on stairs
nav_toggle_deselecting // Start or stop continuously removing from the selected set.
nav_toggle_in_selected_set // Remove current area from the selected set.
nav_toggle_place_mode // Toggle the editor into and out of Place mode. Place mode allows labelling of Area with Place names.
nav_toggle_place_painting // pointing at an Area will paint it with the current Place.
nav_toggle_selected_set // Toggles all areas into/out of the selected set.
nav_toggle_selecting // Start or stop continuously adding to the selected set.
nav_transient // Toggles the area is transient and may become blocked flag used by the AI system.
nav_unmark // Clears the marked Area or Ladder.
nav_update_blocked // Updates the blocked/unblocked status for every nav area.
nav_update_lighting // Recomputes lighting values
nav_update_visibility_on_edit "0" // If nonzero editing the mesh will incrementally recompue visibility
nav_use_place // the current Place is set.
nav_walk // Toggles the traverse this area by walking flag used by the AI system.
nav_warp_to_mark // Warps the player to the marked area.
nav_world_center // Centers the nav mesh in the world
nb_allow_avoiding "1"
nb_allow_climbing "1"
nb_allow_gap_jumping "1"
nb_blind "0" // Disable vision
nb_command // Sends a command string to all bots
nb_debug // ERRORS.
nb_debug_climbing "0"
nb_debug_filter // Add items to the NextBot debug filter. Items can be entindexes or part of the indentifier of one or more bots.
nb_debug_history "1" // each bot keeps a history of debug output in memory
nb_debug_known_entities "0" // Show the known entities for the bot that is the current spectator target
nb_delete_all // Delete all non-player NextBot entities.
nb_force_look_at // Force selected bot to look at the local players position
nb_goal_look_ahead_range "50"
nb_head_aim_resettle_angle "100" // the bot pauses to recenter its virtual mouse on its virtual mousepad
nb_head_aim_resettle_time "0" // How long the bot pauses to recenter its virtual mouse on its virtual mousepad
nb_head_aim_settle_duration "0"
nb_head_aim_steady_max_rate "100"
nb_ladder_align_range "50"
nb_last_area_update_tolerance "4" // Distance a character needs to travel in order to invalidate cached area
nb_move_to_cursor // Tell all NextBots to move to the cursor position
nb_path_draw_inc "100"
nb_path_draw_segment_count "100"
nb_path_segment_influence_radius "100"
nb_player_crouch "0" // Force bots to crouch
nb_player_move "1" // Prevents bots from moving
nb_player_move_direct "0"
nb_player_stop "0" // Stop all NextBotPlayers from updating
nb_player_walk "0" // Force bots to walk
nb_saccade_speed "1000"
nb_saccade_time "0"
nb_select // Select the bot you are aiming at for further debug operations.
nb_shadow_dist "400"
nb_speed_look_ahead_range "150"
nb_stop "0" // Stop all NextBots
nb_update_debug "0"
nb_update_framelimit "15"
nb_update_frequency "0"
nb_update_maxslide "2"
nb_warp_selected_here // Teleport the selected bot to your cursor position
net_blockmsg "0" // Discards incoming message: <0|1|name>
net_channels // Shows net channel info
net_chokeloop "0" // Apply bandwidth choke to loopback packets
net_compresspackets "1" // Use compression on  game packets.
net_compresspackets_minsize "1024" // Dont bother compressing packets below this size.
net_compressvoice "0" // Attempt to compress out of band voice payloads (360 only).
net_drawslider "0" // Draw completion slider during signon
net_droppackets "0" // Drops next n packets on client
net_fakejitter "0" // Jitter fakelag packet time
net_fakelag "0" // Lag all incoming network data (including loopback) by this many milliseconds.
net_fakeloss "0" // Simulate packet loss as a percentage (negative means drop 1/n packets)
net_graph "1" // = 3 draws payload legend.
net_graphheight "64" // Height of netgraph panel
net_graphmsecs "400" // The latency graph represents this many milliseconds.
net_graphpos "1"
net_graphproportionalfont "1" // Determines whether netgraph font is proportional or not
net_graphshowinterp "1" // Draw the interpolation graph.
net_graphshowlatency "1" // Draw the ping/packet loss graph.
net_graphsolid "1"
net_graphtext "1" // Draw text fields
net_maxcleartime "4" // Max # of seconds we can wait for next packets to be sent based on rate setting (0 == no limit).
net_maxfilesize "16" // Maximum allowed file size for uploading in MB
net_maxfragments "1260" // Max fragment bytes per packet
net_maxpacketdrop "5000" // Ignore any packets with the sequence number more than this ahead (0 == no limit)
net_maxroutable "1260" // Requested max packet size before packets are split.
net_queued_packet_thread "1" // Use a high priority thread to send queued packets out instead of sending them each frame.
net_queue_trace "0"
net_scale "5"
net_showdrop "0" // Show dropped packets in  console
net_showevents "0" // 2=all).
net_showfragments "0" // Show netchannel fragments
net_showmsg "0" // Show incoming message: <0|1|name>
net_showpeaks "0" // Show messages for large packets only: <size>
net_showsplits "0" // Show info about packet splits
net_showtcp "0" // Dump TCP stream summary to console
net_showudp "0" // Dump UDP packets summary to console
net_showudp_wire "0" // Show incoming packet information
net_splitpacket_maxrate "15000" // Max bytes per second when queueing splitpacket chunks
net_splitrate "1" // Number of fragments for a splitpacket that can be sent per frame
net_start // Inits multiplayer network sockets
net_status // Shows current network status
net_udp_rcvbuf "131072" // Default UDP receive buffer size
net_usesocketsforloopback "0" // Use network sockets layer even for listen server local players packets (multiplayer only).
next "0" // Set to 1 to advance to next frame ( when singlestep == 1 )
nextdemo // Play next demo in sequence.
nextlevel "0" // will trigger a changelevel to the specified map at the end of the round
noclip // Toggle. Player becomes non-solid and flies.
notarget // Toggle. Player becomes hidden to NPCs.
npc_ally_deathmessage "1"
npc_height_adjust "1" // Enable test mode for ik height adjustment
npc_sentences "0"
npc_vphysics "0"
old_radiusdamage "0"
option_duck_method "1"
option_duck_method_default "1"
opt_EnumerateLeavesFastAlgorithm "1" // Use the new SIMD version of CEngineBSPTree::EnumerateLeavesInBox.
overview_alpha "1" // Overview map translucency.
overview_health "1" // Show players health in map overview.
overview_locked "1" // doesnt follow view angle.
overview_mode // large: <0|1|2>
overview_names "1" // Show players names in map overview.
overview_preferred_mode "1" // Preferred overview mode
overview_preferred_view_size "600" // Preferred overview view size
overview_tracks "1" // Show players tracks in map overview.
overview_zoom // Sets overview map zoom: <zoom> [<time>] [rel]
panel_test_title_safe "0" // Test vgui panel positioning with title safe indentation
particle_simulateoverflow "0" // Used for stress-testing particle systems. Randomly denies creation of particles.
particle_sim_alt_cores "2"
particle_test_attach_attachment "0" // Attachment index for attachment mode
particle_test_attach_mode "0" // follow_origin
particle_test_file "0" // Name of the particle system to dynamically spawn
particle_test_start // particle_test_attach_mode and particl
particle_test_stop // Stops all particle systems on the selected entities. Arguments: {entity_name} / {class_name} / no argument picks what playe
password "0" // Current server access password
path // Show the engine filesystem path.
pause // Toggle the server pause state.
perfui // Show/hide the level performance tools UI.
perfvisualbenchmark
perfvisualbenchmark_abort
phonemedelay "0" // Phoneme delay to account for sound system latency.
phonemefilter "0" // Time duration of box filter to pass over phonemes.
phonemesnap "2" // regardless of duration.
physicsshadowupdate_render "0"
physics_budget // Times the cost of each active object
physics_constraints // Highlights constraint system graph for an entity
physics_debug_entity // Dumps debug info for an entity
physics_highlight_active // Turns on the absbox for all active physics objects
physics_report_active // Lists all active physics objects
physics_select // Dumps debug info for an entity
phys_impactforcescale "1"
phys_penetration_error_time "10" // Controls the duration of vphysics penetration error boxes.
phys_pushscale "1"
phys_speeds "0"
phys_stressbodyweights "5"
phys_timescale "1" // Scale time for physics
phys_upimpactforcescale "0"
picker // pivot and debugging text is displayed for whatever entity the play
ping // Display ping to server.
pipeline_static_props "1"
pixelvis_debug // Dump debug info
play // Play a sound.
playdemo // Play a recorded demo file (.dem ).
player_debug_print_damage "0" // print amount and type of all damage received by player to console.
player_old_armor "0"
playflush // reloading from disk in case of changes.
playgamesound // Play a sound from the  game sounds txt file
playsoundscape // Forces a soundscape to play
playvideo // Plays a video: <filename> [width height]
playvideo_exitcommand // Plays a video and fires and exit command when it is stopped or finishes: <filename> <exit command>
playvol // Play a sound at a specified volume.
plugin_load // plugin_load <filename> : loads a plugin
plugin_pause // plugin_pause <index> : pauses a loaded plugin
plugin_pause_all // pauses all loaded plugins
plugin_print // Prints details about loaded plugins
plugin_unload // plugin_unload <index> : unloads a plugin
plugin_unpause // plugin_unpause <index> : unpauses a disabled plugin
plugin_unpause_all // unpauses all disabled plugins
print_colorcorrection // Display the color correction layer information.
progress_enable
props_break_max_pieces "-1" // Maximum prop breakable piece count (-1 = model default)
props_break_max_pieces_perframe "-1" // Maximum prop breakable piece count per frame (-1 = model default)
prop_active_gib_limit "999999"
prop_active_gib_max_fade_time "999999"
prop_crosshair // Shows name for prop looking at
prop_debug // props will show colorcoded bounding boxes. Red means ignore all damage. White means respond phys
prop_dynamic_create // Creates a dynamic prop with a specific .mdl aimed away from where the player is looking. Arguments: {.mdl name}
prop_physics_create // Creates a physics prop with a specific .mdl aimed away from where the player is looking. Arguments: {.mdl name}
pwatchent "-1" // Entity to watch for prediction system changes.
pwatchvar "0" // Entity variable to watch in prediction system for changes.
pyro_max_intensity "0"
pyro_max_rate "0"
pyro_max_side_length "0"
pyro_max_side_width "0"
pyro_min_intensity "0"
pyro_min_rate "0"
pyro_min_side_length "0"
pyro_min_side_width "0"
pyro_vignette "2"
pyro_vignette_distortion "1"
quit // Exit the engine.
radio1 // Opens a radio menu
radio2 // Opens a radio menu
radio3 // Opens a radio menu
ragdoll_sleepaftertime "5" // the ragdoll will go to sleep.
rate "30000" // Max bytes/sec the host can receive data
rcon // Issue an rcon command.
rcon_address "0" // Address of remote server if sending unconnected rcon commands (format x.x.x.x:p)
rcon_password "0" // remote  console password.
rebuy // Attempt to repurchase items with the order listed in cl_rebuy
recompute_speed // Recomputes clock speed (for debugging purposes).
record // Record a demo.
refresh_options_dialog // Refresh the options dialog.
reload // Reload the most recent saved game (add setpos to jump to current view position on reload).
reload_materials "0"
removeid // Remove a user ID from the ban list.
removeip // Remove an IP address from the ban list.
replay_debug "0"
replay_ignorereplayticks "0"
report_entities // Lists all entities
report_simthinklist // Lists all simulating/thinking entities
report_soundpatch // reports sound patch count
report_soundpatch // reports sound patch count
report_touchlinks // Lists all touchlinks
respawn_entities // Respawn all the entities in the map.
restart // Restart the game on the same level (add setpos to jump to current view position on restart).
retry // Retry connection to last server.
room_type "0"
rope_averagelight "1" // Makes ropes use average of cubemap lighting instead of max intensity.
rope_collide "1" // Collide rope with the world
rope_rendersolid "1"
rope_shake "0"
rope_smooth "1" // Do an antialiasing effect on ropes
rope_smooth_enlarge "1" // How much to enlarge ropes in screen space for antialiasing effect
rope_smooth_maxalpha "0" // Alpha for rope antialiasing effect
rope_smooth_maxalphawidth "1"
rope_smooth_minalpha "0" // Alpha for rope antialiasing effect
rope_smooth_minwidth "0" // this is the min screenspace width it lets a rope shrink to
rope_solid_maxalpha "1"
rope_solid_maxwidth "1"
rope_solid_minalpha "0"
rope_solid_minwidth "0"
rope_subdiv "2" // Rope subdivision amount
rope_wind_dist "1000" // Dont use CPU applying small wind gusts to ropes when theyre past this distance.
rr_debugresponses "0" // it will only show response success/failure for np
rr_debugrule "0" // that rules score will be shown whenever a concept is passed into the response rules system.
rr_debug_qa "0" // Set to 1 to see debug related to the Question & Answer system used to create conversations between allied NPCs.
rr_dumpresponses "0" // Dump all response_rules.txt and rules (requires restart)
rr_reloadresponsesystems // Reload all response system scripts.
r_3dnow // Enable/disable 3DNow code
r_3dsky "1" // Enable the rendering of 3d sky boxes
r_AirboatViewDampenDamp "1"
r_AirboatViewDampenFreq "7"
r_AirboatViewZHeight "0"
r_ambientboost "1" // Set to boost ambient term if it is totally swamped by local lights
r_ambientfactor "5" // Boost ambient cube by no more than this factor
r_ambientfraction "0" // Fraction of direct lighting that ambient cube must be below to trigger boosting
r_ambientlightingonly "0" // Set this to 1 to light models with only ambient lighting (and no static lighting).
r_ambientmin "0" // Threshold above which ambient cube will not boost (i.e. its already sufficiently bright
r_aspectratio "0"
r_avglight "1"
r_avglightmap "0"
r_bloomtintb "0"
r_bloomtintexponent "2"
r_bloomtintg "0"
r_bloomtintr "0"
r_cheapwaterend
r_cheapwaterstart
r_cleardecals // Usage r_cleardecals <permanent>.
r_ClipAreaPortals "1"
r_colorstaticprops "0"
r_debugcheapwater "0"
r_debugrandomstaticlighting "0" // Set to 1 to randomize static lighting for debugging. Must restart for change to take affect.
r_decals "2048"
r_decalstaticprops "1" // Decal static props test
r_decal_cover_count "4"
r_decal_cullsize "1"
r_decal_overlap_area "0"
r_decal_overlap_count "3"
r_depthoverlay "0" // Replaces opaque objects with their grayscaled depth values. r_showz_power scales the output.
r_DispBuildable "0"
r_DispDrawAxes "0"
r_DispWalkable "0"
r_dopixelvisibility "1"
r_drawbatchdecals "1" // Render decals batched.
r_DrawBeams "1" // 2=Wireframe
r_drawbrushmodels "1" // 2=Wireframe
r_drawclipbrushes "0" // purple=NPC)
r_drawdecals "1" // Render decals.
r_drawdetailprops "1" // 2=Wireframe
r_DrawDisp "1" // Toggles rendering of displacment maps
r_drawentities "1"
r_drawflecks "1"
r_drawfuncdetail "1" // Render func_detail
r_drawleaf "-1" // Draw the specified leaf.
r_drawlightcache "0" // 0: off 1: draw light cache entries 2: draw rays
r_drawlightinfo "0"
r_drawlights "0"
r_drawmodeldecals "1"
r_DrawModelLightOrigin "0"
r_drawmodelstatsoverlay "0"
r_drawmodelstatsoverlaydistance "500"
r_drawmodelstatsoverlaymax "1" // time in milliseconds beyond which a model overlay is fully red in r_drawmodelstatsoverlay 2
r_drawmodelstatsoverlaymin "0" // time in milliseconds that a model must take to render before showing an overlay in r_drawmodelstatsoverlay 2
r_drawopaquerenderables "1"
r_drawopaquestaticpropslast "0" // Whether opaque static props are rendered after non-npcs
r_drawopaqueworld "1"
r_drawothermodels "1" // 2=Wireframe
r_drawparticles "1" // Enable/disable particle rendering
r_drawpixelvisibility "0" // Show the occlusion proxies
r_DrawPortals "0"
r_DrawRain "1" // Enable/disable rain rendering.
r_drawrenderboxes "0"
r_drawropes "1"
r_drawskybox "1"
r_DrawSpecificStaticProp "-1"
r_drawsprites "1"
r_drawstaticprops "1" // 2=Wireframe
r_drawtranslucentrenderables "1"
r_drawtranslucentworld "1"
r_drawvgui "1" // Enable the rendering of vgui panels
r_drawviewmodel "1"
r_drawworld "1" // Render the world.
r_dscale_basefov "90"
r_dscale_fardist "2000"
r_dscale_farscale "4"
r_dscale_neardist "100"
r_dscale_nearscale "1"
r_dynamic "1"
r_dynamiclighting "1"
r_entityclips "1"
r_eyeglintlodpixels "20" // The number of pixels wide an eyeball has to be before rendering an eyeglint. Is a floating point value.
r_eyegloss "1"
r_eyemove "1"
r_eyes "1"
r_eyeshift_x "0"
r_eyeshift_y "0"
r_eyeshift_z "0"
r_eyesize "0"
r_eyewaterepsilon "10"
r_farz "-1" // Override the far clipping plane. -1 means to use the value in env_fog_controller.
r_fastzreject "0" // Activate/deactivates a fast z-setting algorithm to take advantage of hardware with fast z reject. Use -1 to default to hardware
r_fastzrejectdisp "0" // Activates/deactivates fast z rejection on displacements (360 only). Only active when r_fastzreject is on.
r_flashlightambient "0"
r_flashlightclip "0"
r_flashlightconstant "0"
r_flashlightculldepth "1"
r_flashlightdepthres "1024"
r_flashlightdepthtexture "1"
r_flashlightdrawclip "0"
r_flashlightdrawdepth "0"
r_flashlightdrawfrustum "0"
r_flashlightdrawfrustumbbox "0"
r_flashlightdrawsweptbbox "0"
r_flashlightfar "750"
r_flashlightfov "45"
r_flashlightladderdist "40"
r_flashlightlinear "100"
r_flashlightlockposition "0"
r_flashlightmodels "1"
r_flashlightnear "4"
r_flashlightnodraw "0"
r_flashlightoffsetx "10"
r_flashlightoffsety "-20"
r_flashlightoffsetz "24"
r_flashlightquadratic "0"
r_flashlightrender "1"
r_flashlightrendermodels "1"
r_flashlightrenderworld "1"
r_flashlightscissor "1"
r_flashlightshadowatten "0"
r_flashlightupdatedepth "1"
r_flashlightvisualizetrace "0"
r_flex "1"
r_flushlod // Flush and reload LODs.
r_ForceWaterLeaf "1" // Enable for optimization to water - considers view in leaf under water for purposes of culling
r_frustumcullworld "1"
r_glint_alwaysdraw "0"
r_glint_procedural "0"
r_hunkalloclightmaps "1"
r_hwmorph "1"
r_itemblinkmax "0"
r_itemblinkrate "4"
r_JeepFOV "90"
r_JeepViewBlendTo "1"
r_JeepViewBlendToScale "0"
r_JeepViewBlendToTime "1"
r_JeepViewDampenDamp "1"
r_JeepViewDampenFreq "7"
r_JeepViewZHeight "10"
r_lightaverage "1" // Activates/deactivate light averaging
r_lightcachecenter "1"
r_lightcachemodel "-1"
r_lightcache_numambientsamples "162" // number of random directions to fire rays when computing ambient lighting
r_lightcache_zbuffercache "0"
r_lightinterp "5" // 0 turns off interpolation
r_lightmap "-1"
r_lightstyle "-1"
r_lightwarpidentity "0"
r_lockpvs "0" // Lock the PVS so you can fly around and inspect what is being drawn.
r_lod "-1"
r_mapextents "16384" // Set the max dimension for the map. This determines the far clipping plane
r_maxdlights "32"
r_maxmodeldecal "50"
r_maxnewsamples "6"
r_maxsampledist "128"
r_minnewsamples "3"
r_modelwireframedecal "0"
r_newflashlight "1"
r_nohw "0"
r_norefresh "0"
r_nosw "0"
r_novis "0" // Turn off the PVS.
r_occludeemaxarea "0" // Prevents occlusion testing for entities that take up more than X% of the screen. 0 means use whatever the level said to use.
r_occluderminarea "0" // Prevents this occluder from being used if it takes up less than X% of the screen. 0 means use whatever the level said to use.
r_occludermincount "0" // no matter how big they are.
r_occlusion "1" // Activate/deactivate the occlusion system.
r_occlusionspew "0" // Activate/deactivates spew about what the occlusion system is doing.
r_oldlightselection "0" // Set this to revert to HL2s method of selecting lights
r_overlayfadeenable "0"
r_overlayfademax "2000"
r_overlayfademin "1750"
r_overlaywireframe "0"
r_particle_sim_spike_threshold_ms "5"
r_partition_level "-1" // Displays a particular level of the spatial partition system. Use -1 to disable it.
r_PhysPropStaticLighting "1"
r_pixelfog "1"
r_pixelvisibility_partial "1"
r_pixelvisibility_spew "0"
r_pix_recordframes "0"
r_pix_start "0"
r_portalsopenall "0" // Open all portals
r_PortalTestEnts "1" // Clip entities against portal frustums.
r_printdecalinfo
r_proplightingfromdisk "1" // 2=Show Errors
r_proplightingpooling "-1" // 1 - static prop color meshes are allocated from a single shared vertex buffer (on hardware that supports stream offset
r_propsmaxdist "1200" // Maximum visible distance
r_queued_decals "0" // Offloads a bit of decal rendering setup work to the material system queue when enabled.
r_queued_post_processing "0"
r_queued_ropes "1"
r_radiosity "4" // 0: no radiosity 1: radiosity with ambient cube (6 samples) 2: radiosity with 162 samples 3: 162 samples for static props, 6 sam
r_rainalpha "0"
r_rainalphapow "0"
r_raindensity "0"
r_RainHack "0"
r_rainlength "0"
r_RainProfile "0" // Enable/disable rain profiling.
r_RainRadius "1500"
r_RainSideVel "130" // How much sideways velocity rain gets.
r_RainSimulate "1" // Enable/disable rain simulation.
r_rainspeed "600"
r_RainSplashPercentage "20"
r_rainwidth "0"
r_randomflex "0"
r_renderoverlayfragment "1"
r_rimlight "1"
r_rootlod "0" // Root LOD
r_ropetranslucent "1"
r_screenfademaxsize "0"
r_screenfademinsize "0"
r_screenoverlay // Draw specified material as an overlay
r_sequence_debug "0"
r_shader_srgb "0" // -1 = use hardware caps. 0 = use hardware srgb. 1 = use shader srgb(software lookup)
r_shadowangles // Set shadow angles
r_shadowblobbycutoff // some shadow stuff
r_shadowcolor // Set shadow color
r_shadowdir // Set shadow direction
r_shadowdist // Set shadow distance
r_shadowids "0"
r_shadowmaxrendered "32"
r_shadowrendertotexture "1"
r_shadows "1"
r_shadows_gamecontrol "-1"
r_shadowwireframe "0"
r_showenvcubemap "0"
r_ShowViewerArea "0"
r_showz_power "1"
r_skin "0"
r_skybox "1" // Enable the rendering of sky boxes
r_snapportal "-1"
r_SnowColorBlue "200" // Snow.
r_SnowColorGreen "175" // Snow.
r_SnowColorRed "150" // Snow.
r_SnowDebugBox "0" // Snow Debug Boxes.
r_SnowEnable "1" // Snow Enable
r_SnowEndAlpha "255" // Snow.
r_SnowEndSize "0" // Snow.
r_SnowFallSpeed "1" // Snow fall speed scale.
r_SnowInsideRadius "256" // Snow.
r_SnowOutsideRadius "1024" // Snow.
r_SnowParticles "500" // Snow.
r_SnowPosScale "1" // Snow.
r_SnowRayEnable "1" // Snow.
r_SnowRayLength "8192" // Snow.
r_SnowRayRadius "256" // Snow.
r_SnowSpeedScale "1" // Snow.
r_SnowStartAlpha "25" // Snow.
r_SnowStartSize "1" // Snow.
r_SnowWindScale "0" // Snow.
r_SnowZoomOffset "384" // Snow.
r_SnowZoomRadius "512" // Snow.
r_spray_lifetime "2" // Number of rounds player sprays are visible
r_sse2 // Enable/disable SSE2 code
r_sse_s "1" // sse ins for particle sphere create
r_staticpropinfo "0"
r_staticprop_lod "-1"
r_studio_stats "0"
r_studio_stats_lock "0" // Lock the current studio stats entity selection
r_studio_stats_mode "0" // Sets a mode for r_studio_stats. Modes are as follows: 0 = Entity under your crosshair 1 = Weapon held by player under your cr
r_swingflashlight "1"
r_teeth "1"
r_threaded_client_shadow_manager "0"
r_threaded_particles "1"
r_threaded_renderables "0"
r_unloadlightmaps "0"
r_updaterefracttexture "1"
r_vehicleBrakeRate "1"
r_VehicleViewClamp "1"
r_VehicleViewDampen "1"
r_visambient "0" // Draw leaf ambient lighting samples. Needs mat_leafvis 1 to work
r_visocclusion "0" // Activate/deactivate wireframe rendering of what the occlusion system is doing.
r_visualizelighttraces "0"
r_visualizelighttracesshowfulltrace "0"
r_visualizeproplightcaching "0"
r_visualizetraces "0"
r_WaterDrawReflection "1" // Enable water reflection
r_WaterDrawRefraction "1" // Enable water refraction
r_waterforceexpensive "1"
r_waterforcereflectentities "0"
r_worldlightmin "0"
r_worldlights "4" // number of world lights to use per vertex
r_worldlistcache "1"
save // Saves current  game.
save_async "1"
save_asyncdelay "0" // adds this many milliseconds of delay to the save operation.
save_console "0" // Autosave on the PC behaves like it does on the consoles.
save_disable "0"
save_finish_async
save_history_count "1" // Keep this many old copies in history of autosaves and quicksaves.
save_huddelayframes "1" // Number of frames to defer for drawing the Saving message.
save_in_memory "0" // Set to 1 to save to memory instead of disk (Xbox 360)
save_noxsave "0"
save_screenshot "1" // 2 = always
save_spew "0"
say // Display player message
say_team // Display player message to team
sb_filter_incompatible_versions "1" // Hides servers running incompatible versions from the server browser. (Internet tab only.)
sb_mod_suggested_maxplayers "0"
sb_quick_list_bit_field "-1"
sb_showblacklists "0" // blacklist rules will be printed to the  console as theyre applied.
scene_async_prefetch_spew "0" // Display async .ani file loading info.
scene_clamplookat "1" // Clamp head turns to a max of 20 degrees per think.
scene_clientflex "1" // Do client side flex animation.
scene_flatturn "1"
scene_flush // Flush all .vcds from the cache and reload from disk.
scene_forcecombined "0" // force use of combined .wav files even in english.
scene_maxcaptionradius "1200" // Only show closed captions if recipient is within this many units of speaking actor (0==disabled).
scene_print "0" // print timing and event info to console.
scene_showfaceto "0" // show the directions of faceto events.
scene_showlook "0" // show the directions of look events.
scene_showmoveto "0" // show the end location.
scene_showunlock "0" // Show when a vcd is playing but normal AI is running.
screenshot // Take a screenshot.
scr_centertime "2"
sensitivity "3" // Mouse sensitivity.
servercfgfile "0"
server_game_time // Gives the game time in seconds (servers curtime)
setang // Snap player eyes to specified pitch yaw <roll:optional> (must have sv_cheats).
setang_exact // Snap player eyes and orientation to specified pitch yaw <roll:optional> (must have sv_cheats).
setinfo // Adds a new user info value
setmodel // Changess players model
setpause // Set the pause state of the server.
setpos // Move player to specified origin (must have sv_cheats).
setpos_exact // Move player to an exact specified origin (must have sv_cheats).
shake // Shake the screen.
shake_show "0" // Displays a list of the active screen shakes.
shake_stop // Stops all active screen shakes.
showbudget_texture "0" // Enable the texture budget panel.
showbudget_texture_global_dumpstats // Dump all items in +showbudget_texture_global in a text form
showbudget_texture_global_sum "0"
showconsole // Show the console.
showhitlocation "0"
showinfo // Shows a info panel: <type> <title> <message> [<command number>]
showpanel // Shows a viewport panel <name>
showparticlecounts "0" // Display number of particles drawn per frame
showschemevisualizer // fonts and colors for a particular scheme. The default is ClientScheme.res
showtriggers "0" // Shows trigger brushes
showtriggers_toggle // Toggle show triggers
simple_bot_add // Add a simple bot.
singlestep "0" // Run engine in single step mode ( set next to 1 to advance a frame )
skill "1" //  Game skill level (1-3).
skip_next_map // Skips the next map in the map rotation for the server.
sk_ally_regen_time "0" // Time taken for an ally to regenerate a point of health.
sk_autoaim_mode "1"
sk_npc_arm "1"
sk_npc_chest "1"
sk_npc_head "2"
sk_npc_leg "1"
sk_npc_stomach "1"
sk_player_arm "1"
sk_player_chest "1"
sk_player_head "2"
sk_player_leg "1"
sk_player_stomach "1"
slot0
slot1
slot10
slot2
slot3
slot4
slot5
slot6
slot7
slot8
slot9
smoothstairs "1" // Smooth player eye z coordinate when traversing stairs.
snapto
sndplaydelay // Usage: sndplaydelay delay_in_sec (negative to skip ahead) soundname
snd_async_flush // Flush all unlocked async audio data
snd_async_fullyasync "0" // All playback is fully async (sound doesnt play until data arrives).
snd_async_minsize "262144"
snd_async_showmem // Show async memory stats
snd_async_spew_blocking "0" // Spew message to console any time async sound loading blocks on file i/o.
snd_async_stream_spew "0" // 2=buffers
snd_buildcache // <directory or VPK filename> Rebulds sound cache for a given search path.
snd_cull_duplicates "0" // aggressively cull duplicate sounds during mixing. The number specifies the number of duplicates allowed to be playe
snd_defer_trace "1"
snd_delay_sound_shift "0"
snd_disable_mixer_duck "0"
snd_duckerattacktime "0"
snd_duckerreleasetime "2"
snd_duckerthreshold "0"
snd_ducktovolume "0"
snd_dumpclientsounds // Dump sounds to VXConsole
snd_foliage_db_loss "4"
snd_gain "1"
snd_gain_max "1"
snd_gain_min "0"
snd_legacy_surround "0"
snd_lockpartial "1"
snd_mixahead "0"
snd_mix_async "0"
snd_musicvolume "1" // Music volume
snd_mute_losefocus "1"
snd_noextraupdate "0"
snd_obscured_gain_dB "-2"
snd_pitchquality "1"
snd_profile "0"
snd_refdb "60"
snd_refdist "36"
snd_restart // Restart sound system.
snd_show "0" // Show sounds info
snd_showclassname "0"
snd_showmixer "0"
snd_showstart "0"
snd_ShowThreadFrameTime "0"
snd_soundmixer "0"
snd_spatialize_roundrobin "0" // spatialize only a fraction of sound channels each frame. 1/2^x of channels will be spatialized
snd_surround_speakers "2"
snd_visualize "0" // Show sounds location in world
snd_vox_captiontrace "0" // Shows sentence name for sentences which are set not to show captions.
snd_vox_globaltimeout "300"
snd_vox_sectimetout "300"
snd_vox_seqtimetout "300"
soundfade // Fade client volume.
soundinfo // Describe the current sound device.
soundlist // List all known sounds.
soundpatch_captionlength "2" // How long looping soundpatch captions should display for.
soundscape_debug "0" // red lines show soundscapes that ar
soundscape_dumpclient // Dumps the clients soundscape data.
soundscape_fadetime "3" // Time to crossfade sound effects between soundscapes
soundscape_flush // Flushes the server & client side soundscapes
speak // Play a constructed sentence.
spec_autodirector "1" // Auto-director chooses best view modes while spectating
spec_freeze_distance_max "90" // Maximum random distance from the target to stop when framing them in observer freeze cam.
spec_freeze_distance_min "80" // Minimum random distance from the target to stop when framing them in observer freeze cam.
spec_freeze_time "5" // Time spend frozen in observer freeze cam.
spec_freeze_traveltime "0" // Time taken to zoom in to frame a target in observer freeze cam.
spec_help // Show spectator help screen
spec_menu // Activates spectator menu
spec_mode // Set spectator mode
spec_next // Spectate next player
spec_player // Spectate player by name
spec_pos // dump position and angles to the  console
spec_prev // Spectate previous player
spec_scoreboard "0"
spec_track "0" // Tracks an entity in spec mode
spew_consolelog_to_debugstring "0" // Send console log to PLAT_DebugString()
spike // generates a fake spike
startdemos // Play demos in demo sequence.
startmovie // Start recording movie frames.
startupmenu // and were not in developer
star_memory // Dump memory stats
stats // Prints server performance variables
stats_reset // Resets all player stats
status // Display map and connection status.
step_spline "0"
stop // Finish recording demo.
stopdemo // Stop playing back a demo.
stopsound
stopsoundscape // Stops all soundscape processing and fades current looping sounds
studio_queue_mode "1"
stuffcmds // Parses and stuffs command line + commands to command buffer.
suitvolume "0"
surfaceprop // Reports the surface properties at the cursor
sv_accelerate "10"
sv_airaccelerate "10"
sv_allowdownload "1" // Allow clients to download files
sv_allowminmodels "1" // Allow or disallow the use of cl_minmodels on this server.
sv_allowupload "1" // Allow clients to upload customizations files
sv_allow_color_correction "1" // Allow or disallow clients to use color correction on this server.
sv_allow_voice_from_file "1" // Allow or disallow clients from using voice_inputfromfile on this server.
sv_allow_votes "1" // Allow voting?
sv_allow_wait_command "1" // Allow or disallow the wait command on clients connected to this server.
sv_alltalk "0" // no team restrictions
sv_alternateticks "0" // server only simulates entities on even numbered ticks.
sv_autosave "1" // Set to 1 to autosave  game on level transition. Does not affect autosave triggers.
sv_backspeed "0" // How much to slow down backwards motion
sv_benchmark_autovprofrecord "0" // it will record a vprof file over the duration of the benchmark with filename benchmark.
sv_benchmark_force_start // Force start the benchmark. This is only for debugging. Its better to set sv_benchmark to 1 and restart the level.
sv_benchmark_numticks "3300" // then it only runs the benchmark for this # of ticks.
sv_bonus_challenge "0" // Set to values other than 0 to select a bonus map challenge type.
sv_bonus_map_challenge_update // Updates a bonus map challenge score.
sv_bonus_map_complete // Completes a bonus map.
sv_bonus_map_unlock // Locks a bonus map.
sv_bounce "0" // Bounce multiplier for when physically simulated objects collide with other objects.
sv_cacheencodedents "1" // does an optimization to prevent extra SendTable_Encode calls.
sv_cheats "0" // Allow cheats on server
sv_clearhinthistory // Clear memory of server side hints displayed to the player.
sv_client_cmdrate_difference "20" // cl_cmdrate is moved to within sv_client_cmdrate_difference units of cl_updaterate before it is clamped between sv_mincmdrate an
sv_client_max_interp_ratio "5" // This can be used to limit the value of cl_interp_ratio for connected clients (only while they are connected). If sv_client_min_
sv_client_min_interp_ratio "1" // This can be used to limit the value of cl_interp_ratio for connected clients (only while they are connected). -1
sv_client_predict "-1" // This can be used to force the value of cl_predict for connected clients (only while they are connected). -1 = let clients se
sv_clockcorrection_msecs "60" // The server tries to keep each players m_nTickBase withing this many msecs of the server absolute tickcount
sv_competitive_minspec "0" // Enable to force certain client convars to minimum/maximum values to help prevent competitive advantages: r_drawdetailprops =
sv_compressstringtablebaselines "0" // Enable to compression for string table baselines. (Spend more CPU time to maybe save a little bandwidth.)
sv_consistency "1" // Legacy variable with no effect! This was deleted and then added as a temporary kludge to prevent players from being banned by
sv_contact "0" // Contact email for server sysop
sv_debugmanualmode "0" // Make sure entities correctly report whether or not their network data has changed.
sv_debugroundstats "0" // A temporary variable that will print extra information about stats upload which may be useful in debugging any problems.
sv_debugtempentities "0" // Show temp entity bandwidth usage.
sv_debug_player_use "0" // Green box=radius success
sv_deltaprint "0" // Print accumulated CalcDelta profiling data (only if sv_deltatime is on)
sv_deltatime "0" // Enable profiling of CalcDelta calls
sv_disablefreezecam "0" // Turn on/off freezecam on server
sv_disable_querycache "0" // debug - disable trace query cache
sv_downloadurl "0" // Location from which clients can download missing files
sv_dumpstringtables "0"
sv_enableboost "0" // Allow boost exploits
sv_enablebunnyhopping "0"
sv_enableoldqueries "0" // Enable support for old style (HL1) server queries
sv_filterban "1" // Set packet filtering by IP mode
sv_footsteps "1" // Play footstep sound for players
sv_forcepreload "0" // Force server side preloading.
sv_friction "4" // World friction.
sv_gravity "800" // World gravity.
sv_hudhint_sound "1"
sv_ignoregrenaderadio "0" // Turn off Fire in the hole messages
sv_ladder_angle "0" // Cos of angle of incidence to ladder perpendicular for applying ladder_dampen
sv_ladder_dampen "0" // Amount to dampen perpendicular movement on a ladder
sv_lan "0" // no non-class C addresses )
sv_legacy_grenade_damage "0" // Enable to replicate grenade damage behavior of the original Counter-Strike Source game.
sv_logbans "0" // Log server bans in the server logs.
sv_logblocks "0" // If true when log when a query is blocked (can cause very large log files)
sv_logdownloadlist "1"
sv_logecho "1" // Echo log information to the  console.
sv_logfile "1" // Log server information in the log file.
sv_logflush "0" // Flush the log file to disk on each write (slow).
sv_logsdir "0" // Folder in the  game directory where server logs will be stored.
sv_logsecret "0" // not usual 0x52)
sv_log_onefile "0" // Log server information to only one file.
sv_lowedict_action "0" // 4 - go to the next ma
sv_lowedict_threshold "8" // take the action specified by sv_lowedict_action.
sv_massreport "0"
sv_master_share_game_socket "1" // then it will create a socket on -steamport + 1 to comm
sv_maxcmdrate "66" // this sets the maximum value for cl_cmdrate.
sv_maxrate "0" // 0 == unlimited
sv_maxreplay "0" // Maximum replay time in seconds
sv_maxroutable "1260" // Server upper bound on net_maxroutable that a client can use.
sv_maxspeed "320"
sv_maxupdaterate "66" // Maximum updates per second that the server will allow
sv_maxusrcmdprocessticks "24" // 0 to allow no restrictions
sv_maxusrcmdprocessticks_warning "-1" // negat
sv_maxvelocity "3500" // Maximum speed any ballistically moving object is allowed to attain per axis.
sv_max_connects_sec "2" // Maximum connections per second to respond to from a single IP address.
sv_max_connects_sec_global "0" // Maximum connections per second to respond to from anywhere.
sv_max_connects_window "4" // Window over which to average connections per second averages.
sv_max_queries_sec "3" // Maximum queries per second to respond to from a single IP address.
sv_max_queries_sec_global "3000" // Maximum queries per second to respond to from anywhere.
sv_max_queries_window "30" // Window over which to average queries per second averages.
sv_max_usercmd_future_ticks "8" // Prevents clients from running usercmds too far in the future. Prevents speed hacks.
sv_memlimit "0" // the server wi
sv_mincmdrate "10" // This sets the minimum value for cl_cmdrate. 0 == unlimited.
sv_minrate "3500" // 0 == unlimited
sv_minupdaterate "10" // Minimum updates per second that the server will allow
sv_motd_unload_on_dismissal "0" // the MOTD contents will be unloaded when the player closes the MOTD.
sv_mumble_positionalaudio "1" // Allows players using Mumble to have support for positional audio.
sv_namechange_cooldown_seconds "20" // wait N seconds allowing another name change
sv_netspike // Write network trace if amount of data sent to client exceeds N bytes. Use zero to disable tracing. Note that having this enabl
sv_netspike_on_reliable_snapshot_overflow "0" // the server will dump a netspike trace if a client is dropped due to reliable snapshot overflow
sv_netspike_output "1" // 2=ordinary server log
sv_netspike_sendtime_ms "0" // the server will dump a netspike trace if it takes more than N ms to prepare a snapshot to a single client. This fe
sv_noclipaccelerate "5"
sv_noclipduringpause "0" // etc.).
sv_noclipspeed "5"
sv_nomvp "0" // Disable MVP awards.
sv_nonemesis "0" // Disable nemesis and revenge.
sv_noroundstats "1" // A temporary variable that can be used for disabling upload of round stats.
sv_nostats "0" // Disable collecting statistics and awarding achievements.
sv_nowinpanel "0" // Turn on/off win panel on server
sv_npc_talker_maxdist "1024" // NPCs over this distance from the player wont attempt to speak.
sv_parallel_packentities "1"
sv_parallel_sendsnapshot "1"
sv_password "0" // Server password for entry into multiplayer  games
sv_pausable "0" // Is the server pausable.
sv_playerperfhistorycount "60" // Number of samples to maintain in player perf history
sv_player_display_usercommand_errors "0" // 1 = Display warning when command values are out-of-range. 2 = Spew invalid ranges.
sv_precacheinfo // Show precache info.
sv_pure // Show user data.
sv_pure_consensus "5" // Minimum number of file hashes to agree to form a consensus.
sv_pure_kick_clients "1" // it will issue a warning to the client.
sv_pure_retiretime "900" // Seconds of server idle time to flush the sv_pure file hash cache.
sv_pure_trace "0" // the server will print a message whenever a client is verifying a CRC for a file.
sv_pushaway_clientside "0" // 1=all players)
sv_pushaway_force "30000" // How hard physics objects are pushed away from the players on the server.
sv_pushaway_hostage_force "20000" // How hard the hostage is pushed away from physics objects (falls off with inverse square of distance).
sv_pushaway_max_force "1000" // Maximum amount of force applied to physics objects by players.
sv_pushaway_max_hostage_force "1000" // Maximum of how hard the hostage is pushed away from physics objects.
sv_pushaway_max_player_force "10000" // Maximum of how hard the player is pushed away from physics objects.
sv_pushaway_min_player_speed "75" // dont push away physics objects (enables ducking behind things).
sv_pushaway_player_force "200000" // How hard the player is pushed away from physics objects (falls off with inverse square of distance).
sv_pvsskipanimation "1" // Skips SetupBones when npcs are outside the PVS
sv_querycache_stats // Display status of the query cache (client only)
sv_rcon_banpenalty "0" // Number of minutes to ban users who fail rcon authentication
sv_rcon_log "1" // Enable/disable rcon logging.
sv_rcon_maxfailures "10" // Max number of times a user can fail rcon authentication before being banned
sv_rcon_maxpacketbans "1" // Ban IPs for sending RCON packets exceeding the value specified in sv_rcon_maxpacketsize
sv_rcon_maxpacketsize "1024" // The maximum number of bytes to allow in a command packet
sv_rcon_minfailures "5" // Number of times a user can fail rcon authentication in sv_rcon_minfailuretime before being banned
sv_rcon_minfailuretime "30" // Number of seconds to track failed rcon authentications
sv_region "-1" // The region of the world to report this server in.
sv_restrict_aspect_ratio_fov "1" // This can be used to limit the effective FOV of users using wide-screen resolutions with aspect ratios wider than 1.85:1 (slight
sv_rollangle "0" // Max view roll angle
sv_rollspeed "200"
sv_runcmds "1"
sv_setsteamaccount // token Set  game server account token to use for logging in to a persistent game server account
sv_showimpacts "0" // 3=server-only)
sv_showladders "0" // Show bbox and dismount points for all ladders (must be set before level load.)
sv_showlagcompensation "0" // Show lag compensated hitboxes whenever a player is lag compensated.
sv_showplayerhitboxes "0" // Show lag compensated hitboxes for the specified player index whenever a player fires.
sv_shutdown // Sets the server to shutdown next time its empty
sv_shutdown_timeout_minutes "360" // wait at most N minutes for server to drain before forcing shutdown.
sv_skyname "0" // Current name of the skybox texture
sv_soundemitter_trace "0" // Show all EmitSound calls including their symbolic name and the actual wave file they resolved to
sv_specaccelerate "5"
sv_specnoclip "1"
sv_specspeed "3"
sv_stats "1" // Collect CPU usage stats
sv_steamblockingcheck "0" // 3 >= dr
sv_steamgroup "0" // The ID of the steam group that this server belongs to. You can find your groups ID on the admin profile page in the steam comm
sv_stepsize "18"
sv_stickysprint_default "0"
sv_stopspeed "100" // Minimum stopping speed when on ground.
sv_strict_notarget "0" // notarget will cause entities to never think they are in the pvs
sv_tags "0" // Server tags. Used to provide extra information to clients when theyre browsing for servers. Separate tags with a comma.
sv_teststepsimulation "1"
sv_test_scripted_sequences "0" // Tests for scripted sequences that are embedded in the world. Run through your map with this set to check for NPCs falling throu
sv_thinktimecheck "0" // Check for thinktimes all on same timestamp.
sv_timebetweenducks "0" // Minimum time before recognizing consecutive duck key
sv_timeout "65" // the client is dropped
sv_turbophysics "0" // Turns on turbo physics
sv_unlockedchapters "1" // Highest unlocked game chapter.
sv_use_steam_voice "1" // voice_inputfromfile will no longer function).
sv_vehicle_autoaim_scale "8"
sv_visiblemaxplayers "-1" // Overrides the max players reported to prospective clients
sv_voiceenable "1"
sv_vote_allow_spectators "0" // Allow spectators to vote?
sv_vote_failure_timer "300" // A vote that fails cannot be re-submitted for this long
sv_vote_ui_hide_disabled_issues "1" // Suppress listing of disabled issues in the vote setup screen.
sv_wateraccelerate "10"
sv_waterdist "12" // Vertical view fixup when eyes are near water plane.
sv_waterfriction "1"
systemlinkport "27030" // System Link port
sys_minidumpexpandedspew "1"
sys_minidumpspewlines "500" // Lines of crash dump  console spew to keep.
telemetry_demoend "0" // stop telemetry on tick #
telemetry_demostart "0" // start telemetry on tick #
telemetry_filtervalue "500" // Set Telemetry ZoneFilterVal (MicroSeconds)
telemetry_framecount "0" // Set Telemetry count of frames to capture
telemetry_level "0" // Set Telemetry profile level: 0 being off. Hight bit set for mask: 0x8#######
telemetry_pause "0" // Pause Telemetry
telemetry_resume "0" // Resume Telemetry
telemetry_server "0" // Set Telemetry server
template_debug "0"
testhudanim // Test a hud element animation. Arguments: <anim name>
testscript_debug "0" // Debug test scripts.
Test_CreateEntity
test_dispatcheffect // Test a clientside dispatch effect. Usage: test_dispatcheffect <effect name> <distance away> <flags> <magnitude> <scale> Defau
Test_EHandle
test_entity_blocker // Test command that drops an entity blocker out in front of the player.
test_freezeframe // Test the freeze frame code.
Test_InitRandomEntitySpawner
Test_ProxyToggle_EnableProxy
Test_ProxyToggle_EnsureValue // Test_ProxyToggle_EnsureValue
Test_ProxyToggle_SetValue
Test_RandomizeInPVS
Test_RandomPlayerPosition
Test_RemoveAllRandomEntities
Test_SpawnRandomEntities
texture_budget_background_alpha "128" // how translucent the budget panel is
texture_budget_panel_bottom_of_history_fraction "0" // number between 0 and 1
texture_budget_panel_global "0" // Show global times in the texture budget panel.
texture_budget_panel_height "284" // height in pixels of the budget panel
texture_budget_panel_width "512" // width in pixels of the budget panel
texture_budget_panel_x "0" // number of pixels from the left side of the game screen to draw the budget panel
texture_budget_panel_y "450" // number of pixels from the top side of the  game screen to draw the budget panel
tf_arena_max_streak "3" // Teams will be scrambled if one team reaches this streak
tf_arena_preround_time "10" // Length of the Pre-Round time
tf_arena_round_time "0"
tf_arena_use_queue "1" // Enables the spectator queue system for Arena.
tf_escort_score_rate "1" // in points per second
think_limit "10" // warning is printed if this is exceeded.
thirdperson // Switch to thirdperson camera.
thirdperson_mayamode // Switch to thirdperson Maya-like camera controls.
thirdperson_platformer "0" // Player will aim in the direction they are moving.
thirdperson_screenspace "0" // eg: left means screen-left
threadpool_affinity "1" // Enable setting affinity
timedemo // Play a demo and report performance info.
timedemoquit // and then exit
timedemo_runcount "0" // Runs time demo X number of times.
timeleft // prints the time remaining in the match
timerefresh // Profile the renderer.
toggle // or cycles through a set of values.
toggleconsole // Show/hide the console.
togglescores // Toggles score panel
tracer_extra "1"
trace_report "0"
TrackerAnim // Test animation of the achievement tracker. Parameter is achievement number on HUD to flash
tv_allow_camera_man "1" // Auto director allows spectators to become camera man
tv_allow_static_shots "1" // Auto director uses fixed level cameras for shots
tv_autorecord "0" // Automatically records all  games as SourceTV demos.
tv_autoretry "1" // Relay proxies retry connection after network timeout
tv_chatgroupsize "0" // Set the default chat group size
tv_chattimelimit "8" // Limits spectators to chat only every n seconds
tv_clients // Shows list of connected SourceTV clients.
tv_debug "0" // SourceTV debug info.
tv_delay "30" // SourceTV broadcast delay in seconds
tv_delaymapchange "0" // Delays map change until broadcast is complete
tv_deltacache "2" // Enable delta entity bit stream cache
tv_dispatchmode "1" // 2=always
tv_enable "0" // Activates SourceTV on server.
tv_maxclients "128" // Maximum client number on SourceTV server.
tv_maxrate "8000" // 0 == unlimited
tv_msg // Send a screen message to all clients.
tv_name "0" // SourceTV host name
tv_nochat "0" // Dont receive chat messages from other SourceTV spectators
tv_overridemaster "0" // Overrides the SourceTV master root address.
tv_password "0" // SourceTV password for all clients
tv_port "27020" // Host SourceTV port
tv_record // Starts SourceTV demo recording.
tv_relay // Connect to SourceTV server and relay broadcast.
tv_relaypassword "0" // SourceTV password for relay proxies
tv_relayvoice "1" // 1=on
tv_retry // Reconnects the SourceTV relay proxy.
tv_snapshotrate "16" // Snapshots broadcasted per second
tv_status // Show SourceTV server status.
tv_stop // Stops the SourceTV broadcast.
tv_stoprecord // Stops SourceTV demo recording.
tv_timeout "30" // SourceTV connection timeout in seconds.
tv_title "0" // Set title for SourceTV spectator UI
tv_transmitall "0" // Transmit all entities (not only director view)
ui_posedebug_fade_in_time "0" // Time during which a new pose activity layer is shown in green in +posedebug UI
ui_posedebug_fade_out_time "0" // Time to keep a no longer active pose activity layer in red until removing it from +posedebug UI
unbind // Unbind a key.
unbindall // Unbind all keys.
unbind_mac // Unbind a key on the Mac only.
unpause // Unpause the game.
use // Use a particular weapon Arguments: <weapon_name>
user // Show user data.
users // Show user info for players on server.
user_context // Set a Rich Presence Context: user_context <context id> <context value>
user_property // Set a Rich Presence Property: user_property <property id>
vcollide_wireframe "0" // Render physics collision models in wireframe
vcr_verbose "0" // Write extra information into .vcr file.
vehicle_flushscript // Flush and reload all vehicle scripts
version // Print version info string.
vgui_drawfocus "0" // Report which panel is under the mouse.
vgui_drawtree "0" // Draws the vgui panel hiearchy to the specified depth level.
vgui_drawtree_bounds "0" // Show panel bounds.
vgui_drawtree_clear
vgui_drawtree_draw_selected "0" // Highlight the selected panel
vgui_drawtree_freeze "0" // Set to 1 to stop updating the vgui_drawtree view.
vgui_drawtree_hidden "0" // Draw the hidden panels.
vgui_drawtree_panelalpha "0" // Show the panel alpha values in the vgui_drawtree view.
vgui_drawtree_panelptr "0" // Show the panel pointer values in the vgui_drawtree view.
vgui_drawtree_popupsonly "0" // Draws the vgui popup list in hierarchy(1) or most recently used(2) order.
vgui_drawtree_render_order "0" // List the vgui_drawtree panels in render order.
vgui_drawtree_visible "1" // Draw the visible panels.
vgui_message_dialog_modal "1"
vgui_spew_fonts
vgui_togglepanel // show/hide vgui panel by name.
video_quicktime_decode_gamma "0" // QuickTime Video Playback Gamma Target- 0=no gamma adjust 1=platform default gamma 2 = gamma 1.8 3 = gamma 2.2 4 = gamma 2.5
video_quicktime_encode_gamma "3" // QuickTime Video Encode Gamma Target- 0=no gamma adjust 1=platform default gamma 2 = gamma 1.8 3 = gamma 2.2 4 = gamma 2.5
viewanim_addkeyframe
viewanim_create // viewanim_create
viewanim_load // load animation from file
viewanim_reset // reset view angles!
viewanim_save // Save current animation to file
viewanim_test // test view animation
viewmodel_fov "54"
violence_ablood "1" // Draw alien blood
violence_agibs "1" // Show alien gib entities
violence_hblood "1" // Draw human blood
violence_hgibs "1" // Show human gib entities
voice_avggain "0"
voice_clientdebug "0"
voice_debugfeedback "0"
voice_debugfeedbackfrom "0"
voice_enable "1"
voice_fadeouttime "0"
voice_forcemicrecord "1"
voice_inputfromfile "0" // Get voice input from voice_input.wav rather than from the microphone.
voice_loopback "0"
voice_maxgain "10"
voice_modenable "1" // Enable/disable voice in this mod.
voice_overdrive "2"
voice_overdrivefadetime "0"
voice_printtalkers // voice debug.
voice_profile "0"
voice_recordtofile "0" // Record mic data and decompressed voice data into voice_micdata.wav and voice_decompressed.wav
voice_scale "1"
voice_serverdebug "0"
voice_showchannels "0"
voice_showincoming "0"
voice_steal "2"
voice_writevoices "0" // Saves each speakers voice data into separate .wav files
volume "1" // Sound volume
voxeltree_box // Vector(max)>.
voxeltree_playerview // View entities in the voxel-tree at the player position.
voxeltree_sphere // float(radius)>.
voxeltree_view // View entities in the voxel-tree.
vox_reload // Reload sentences.txt file
vprof // Toggle VProf profiler
vprof_adddebuggroup1 // add a new budget group dynamically for debugging
vprof_cachemiss // Toggle VProf cache miss checking
vprof_cachemiss_off // Turn off VProf cache miss checking
vprof_cachemiss_on // Turn on VProf cache miss checking
vprof_child
vprof_collapse_all // Collapse the whole vprof tree
vprof_counters "0"
vprof_dump_groupnames // Write the names of all of the vprof groups to the  console.
vprof_dump_oninterval "0" // Interval (in seconds) at which vprof will batch up data and dump it to the console.
vprof_dump_spikes "0" // negative to reset after dump
vprof_dump_spikes_budget_group "0" // Budget gtNode to start report from when doing a dump spikes
vprof_dump_spikes_node "0" // Node to start report from when doing a dump spikes
vprof_expand_all // Expand the whole vprof tree
vprof_expand_group // Expand a budget group in the vprof tree by name
vprof_generate_report // Generate a report to the console.
vprof_generate_report_AI // Generate a report to the console.
vprof_generate_report_AI_only // Generate a report to the console.
vprof_generate_report_budget // Generate a report to the console based on budget group.
vprof_generate_report_hierarchy // Generate a report to the console.
vprof_generate_report_map_load // Generate a report to the console.
vprof_graph "0" // Draw the vprof graph.
vprof_graphheight "256"
vprof_graphwidth "512"
vprof_nextsibling
vprof_off // Turn off VProf profiler
vprof_on // Turn on VProf profiler
vprof_parent
vprof_playback_average // Average the next N frames.
vprof_playback_start // Start playing back a recorded .vprof file.
vprof_playback_step // step to the next tick.
vprof_playback_stepback // step to the previous tick.
vprof_playback_stop // Stop playing back a recorded .vprof file.
vprof_prevsibling
vprof_record_start // Start recording vprof data for playback later.
vprof_record_stop // Stop recording vprof data
vprof_remote_start // Request a VProf data stream from the remote server (requires authentication)
vprof_remote_stop // Stop an existing remote VProf data request
vprof_report_oninterval "0" // Interval (in seconds) at which vprof will batch up a full report to the console -- more detailed than vprof_dump_oninterval.
vprof_reset // Reset the stats in VProf profiler
vprof_reset_peaks // Reset just the peak time in VProf profiler
vprof_scope "0" // Set a specific scope to start showing vprof tree
vprof_scope_entity_gamephys "0"
vprof_scope_entity_thinks "0"
vprof_unaccounted_limit "0" // number of milliseconds that a node must exceed to turn red in the vprof panel
vprof_verbose "1" // Set to one to show average and peak times
vprof_vtrace // Toggle whether vprof data is sent to VTrace
vprof_vtune_group // enable vtune for a particular vprof group (disable to disable)
vprof_warningmsec "10" // Above this many milliseconds render the label red to indicate slow code.
vr_activate // Switch to VR mode
vr_activate_default "0" // If this is true the  game will switch to VR mode once startup is complete.
vr_aim_yaw_offset "90" // This value is added to Yaw when returning the vehicle aim angles to Source.
vr_cycle_aim_move_mode // Cycle through the aim & move modes.
vr_deactivate // Switch from VR mode to normal mode
vr_debug_nochromatic "0"
vr_debug_nodistortion "0"
vr_debug_remote_cam "0"
vr_debug_remote_cam_pos_x "150"
vr_debug_remote_cam_pos_y "0"
vr_debug_remote_cam_pos_z "0"
vr_debug_remote_cam_target_x "0"
vr_debug_remote_cam_target_y "0"
vr_debug_remote_cam_target_z "-50"
vr_distortion_enable "1"
vr_first_person_uses_world_model "1" // Causes the third person model to be drawn instead of the view model
vr_force_windowed "0"
vr_hud_axis_lock_to_world "0" // 2=roll
vr_hud_display_ratio "0"
vr_hud_forward "500" // Apparent distance of the HUD in inches
vr_hud_max_fov "60" // Max FOV of the HUD
vr_hud_never_overlay "0"
vr_moveaim_mode "3" // 4=shoot with face+mouse cursor. 5+ are probably not that useful.
vr_moveaim_mode_zoom "3" // 4=shoot with face+mouse cursor. 5+ are probably not that useful.
vr_moveaim_reticle_pitch_limit "30" // the mouse clamps
vr_moveaim_reticle_pitch_limit_zoom "-1" // the mouse clamps
vr_moveaim_reticle_yaw_limit "10" // the mouse drags the torso
vr_moveaim_reticle_yaw_limit_zoom "0" // the mouse drags the torso
vr_projection_znear_multiplier "0" // Allows moving the ZNear plane to deal with body clipping
vr_refresh_distortion_texture
vr_render_hud_in_world "1"
vr_reset_home_pos // Sets the current HMD position as the zero point
vr_stereo_mono_set_eye "0" // 3=middle eye
vr_stereo_swap_eyes "0" // 1=swap eyes.
vr_toggle // Toggles VR mode
vr_track_reinit // Reinitializes HMD tracking
vr_translation_limit "10" // How far the in-game head will translate before being clamped.
vr_use_offscreen_render_target "0" // Experimental: Use larger offscreen render target for pre-distorted scene in VR
vr_viewmodel_offset_forward "-8"
vr_viewmodel_offset_forward_large "-15"
vr_viewmodel_translate_with_head "0" // 1=translate the viewmodel with the head motion.
vr_zoom_multiplier "2" // how big is the scope on your HUD?
vr_zoom_scope_scale "6" // Something to do with the default scope HUD overlay size.
vtune // Controls VTunes sampling.
v_centermove "0"
v_centerspeed "500"
wc_air_edit_further // moves position of air node crosshair and placement location further away from play
wc_air_edit_nearer // moves position of air node crosshair and placement location nearer to from player
wc_air_node_edit // toggles laying down or air nodes instead of ground nodes
wc_create // creates a node where the player is looking if a node is allowed at that location for the currently select
wc_destroy // destroys the node that the player is nearest to looking at. (The node will be highlighted by a red box).
wc_destroy_undo // When in WC edit mode restores the last deleted node
wc_link_edit
weapon_showproficiency "0"
windows_speaker_config "4"
wipe_nav_attributes // Clear all nav attributes of selected area.
writeid // Writes a list of permanently-banned user IDs to banned_user.cfg.
writeip // Save the ban list to banned_ip.cfg.
x360_audio_english "0" // Keeps track of whether were forcing english in a localized language.
x360_resolution_height "480" // This is only used for reference. Changing this value does nothing
x360_resolution_interlaced "0" // This is only used for reference. Changing this value does nothing
x360_resolution_widescreen_mode "0" // This is only used for reference. Changing this value does nothing
x360_resolution_width "640" // This is only used for reference. Changing this value does nothing
xbox_autothrottle "1"
xbox_steering_deadzone "0"
xbox_throttlebias "100"
xbox_throttlespoof "200"
xc_crouch_debounce "0"
xload // Load a saved game from a 360 storage device.
xlook
xmove
xsave // Saves current game to a 360 storage device.
zoom_sensitivity_ratio "1" // Additional mouse sensitivity scale factor applied when FOV is zoomed in.
_autosave // Autosave
_autosavedangerous // AutoSaveDangerous
_bugreporter_restart // Restarts bug reporter .dll
_fov "0" // Automates fov command to server.
_resetgamestats // Erases current game stats and writes out a blank stats file
_restart // Shutdown and restart the engine.
```