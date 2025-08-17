# Level-Specific Values in Otto Matic

This document catalogs all hardcoded level-specific values controlled by `gLevelNum` checks throughout the codebase. These values can be overridden using the level override system.

## Sky System (src/Effects/Sky.c)

### Line 82: Sky configuration lookup
- **Purpose**: Selects sky style from lookup table based on level
- **Current behavior**: `kSkyTable[gLevelNum]` - uses hardcoded sky settings per level
- **Override field**: `skyStyle` (index into sky table)

### Line 220: Apocalypse glow effect
- **Purpose**: Enables special glow rendering for Apocalypse level  
- **Current behavior**: `if (gLevelNum == LEVEL_NUM_APOCALYPSE)` - enables glow blending
- **Override field**: `enableSkyGlow` (boolean)

### Line 240: Sky edge fading
- **Purpose**: Controls whether sky edges fade based on level configuration
- **Current behavior**: `kSkyTable[gLevelNum].fadeEdges` - uses hardcoded fade setting
- **Override field**: `skyFadeEdges` (boolean)

### Line 252: Sky altitude positioning  
- **Purpose**: Sets sky altitude based on level configuration
- **Current behavior**: `kSkyTable[gLevelNum].altitude` - uses hardcoded altitude
- **Override field**: `skyAltitude` (float)

## Player System (src/Player/Player.c)

### Line 129: Saucer level rocket scaling
- **Purpose**: Scales down rocket size for saucer level
- **Current behavior**: `if (gLevelNum == LEVEL_NUM_SAUCER)` - sets scale to 0.4f, else 1.0f
- **Override field**: `rocketScale` (float)

### Lines 150-151: Boss level fuel retention
- **Purpose**: Player keeps full fuel on certain boss levels
- **Current behavior**: `if ((gLevelNum == LEVEL_NUM_BLOBBOSS) || (gLevelNum == LEVEL_NUM_JUNGLEBOSS))` - fuel = 1, else fuel = 0
- **Override field**: `startWithFullFuel` (boolean)

## Camera System (src/3D/Camera.c)

### Saucer level camera behavior
- **Purpose**: Special camera handling for saucer level
- **Override field**: `useSaucerCamera` (boolean)

### Blob boss level camera height  
- **Purpose**: Keeps camera high on blob boss level
- **Override field**: `useBlobBossCamera` (boolean)

## Items System (src/Items/Powerups.c)

### Line: Brain boss sparkle conservation
- **Purpose**: Conserves sparkle effects on brain boss level or non-G4 systems
- **Current behavior**: `if ((gLevelNum == LEVEL_NUM_BRAINBOSS) || (!gG4))` 
- **Override field**: `conserveSparkles` (boolean)

### Jungle level weapon behavior
- **Purpose**: Different weapon behavior in jungle levels
- **Override field**: `jungleWeaponMode` (boolean)

### Cloud level ground effects
- **Purpose**: Special ground effect behavior on cloud level
- **Override field**: `disableGroundEffects` (boolean)

## Triggers System (src/Items/Triggers.c)

### Blob boss trigger behavior
- **Purpose**: Special trigger behavior on blob boss level
- **Override field**: `blobBossTriggers` (boolean)

## Rocket Sled System (src/Items/RocketSled.c)

### Cloud level rocket sled behavior (multiple lines)
- **Purpose**: Special rocket sled mechanics on cloud level
- **Override field**: `cloudRocketMode` (boolean)

## Zip Line System (src/Items/ZipLine.c)

### Apocalypse level zip line behavior (multiple lines)
- **Purpose**: Special zip line mechanics on apocalypse level  
- **Override field**: `apocalypseZipMode` (boolean)

## Human System (src/Items/Humans.c)

### Saucer level human behavior (multiple lines)
- **Purpose**: Special human AI and physics for saucer level
- **Override field**: `saucerHumanMode` (boolean)

## Terrain System (src/Terrain/Fences.c)

### Line: Fence positioning
- **Purpose**: Special fence height behavior for cloud and saucer levels
- **Current behavior**: `if ((gLevelNum != LEVEL_NUM_CLOUD) && (gLevelNum != LEVEL_NUM_SAUCER))` - sinks fences, else flush
- **Override field**: `sinkFences` (boolean)

## Terrain System (src/Terrain/SplineItems.c)

### Line: Level 2 spline behavior
- **Purpose**: Special spline item behavior for level 2 (blob world)
- **Current behavior**: `if (gLevelNum == 2)`
- **Override field**: `blobSplineMode` (boolean)

## Terrain System (src/Terrain/Terrain.c)

### Cloud level terrain behavior (multiple lines)
- **Purpose**: Special terrain collision and rendering for cloud level
- **Override field**: `cloudTerrainMode` (boolean)

## Robot Player System (src/Player/Player_Robot.c)

### Blob boss level robot behavior
- **Purpose**: Special robot physics and interactions for blob boss level
- **Override field**: `blobBossRobotMode` (boolean)

### Cloud level robot behavior
- **Purpose**: Special robot movement for cloud level
- **Override field**: `cloudRobotMode` (boolean)

## Weapon System (src/Player/Player_Weapons.c)

### Jungle level weapon behavior  
- **Purpose**: Special weapon mechanics for jungle levels
- **Current behavior**: `if (gLevelNum == LEVEL_NUM_JUNGLE || gLevelNum == LEVEL_NUM_JUNGLEBOSS)`
- **Override field**: `jungleWeaponMode` (boolean)

## Level Intro System (src/Screens/LevelIntros.c)

### Intro timing and behavior
- **Purpose**: Controls level intro sequences and timing
- **Override field**: `customIntroMode` (boolean)

### Boss level intro skipping
- **Purpose**: Skips intro for certain boss levels
- **Override field**: `skipIntro` (boolean)

## Bonus Screen System (src/Screens/BonusScreen.c)

### Blob level bonus behavior
- **Purpose**: Special bonus screen behavior for blob level
- **Override field**: `blobBonusMode` (boolean)

### Jungle level bonus behavior
- **Purpose**: Special bonus screen behavior for jungle level  
- **Override field**: `jungleBonusMode` (boolean)

## Infobar System (src/Screens/Infobar.c)

### Saucer level infobar
- **Purpose**: Special infobar display for saucer level
- **Override field**: `saucerInfobar` (boolean)

## Enemy Systems

### Flytrap Enemy (src/Enemies/Jungle/Enemy_Flytrap.c)
- **Purpose**: Disables auto-targeting on jungle boss level
- **Override field**: `flyTrapAutoTarget` (boolean)

### Saucer Enemy (src/Enemies/Saucer.c)
- **Purpose**: Controls saucer spawn behavior per level
- **Override field**: `enableSaucers` (boolean)

### Generic Enemy (src/Enemies/Enemy.c)  
- **Purpose**: Cloud level only enemy behavior
- **Override field**: `cloudOnlyEnemies` (boolean)

### Brain Alien Enemy (src/Enemies/Enemy_BrainAlien.c)
- **Purpose**: Level-specific brain alien behavior
- **Override field**: `brainAlienMode` (int32_t)

## File System (src/System/File.c)

### Asset loading per level
- **Purpose**: Loads different terrain, models, sprites, and sound banks per level
- **Override fields**: These are handled through existing asset system, not overrides

## Game Flow (src/System/GameMain.c)

### Level progression and song selection
- **Purpose**: Controls music and game flow progression
- **Override fields**: These control core game flow, should not be overridden

## Summary

Total level-specific checks found: ~60+ individual override points
These can be grouped into approximately 30 distinct override fields covering:
- Visual effects and rendering
- Player mechanics and physics  
- Enemy behavior and AI
- Terrain and environmental behavior
- UI and screen behavior
- Audio and effects systems