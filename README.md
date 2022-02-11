# minewall
An anti-cheat mod for Minecraft Bedrock Dedicated Server, supports BDS version 1.18.11.01.

This tweak hooks CraftResults_DEPRECATEDASKTYLAING (marked deprecated by mojang but haven't been deprecated for a long period and still taking an important position) and revalidate it by comparing it with the recipe id sent in the previous action (CraftRecipe).

Note: only linux is supported since this tweak used many raw mangled symbols, but porting to Windows is possible.

This mod should be used with [patched server-modloader](https://github.com/LNSSPsd/server-modloader) project because the latest version of BDS in linux didn't export its symbols shared.

## Handled cheating behaviors
* Furnace swapping item cloning
* ExperienceOrb client spawning (Simple check only)
* Unlimited item creation by trading with villagers

## Building Requirements
* Node.JS
* cmake
* server-modloader's sdk

## LICENSE
[MIT License](LICENSE)