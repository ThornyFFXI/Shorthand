# Shorthand is only designed for english language users, as the mechanisms it uses to adjust syntax are exclusive to the english language.  It should not be used on the Japanese client, as there are known issues with the text encoding that can cause unintended behavior.

# Shorthand
Shorthand is a plugin that allows you to be less strict when typing actions directly.  To be specific, it provides the following error correction:
* You may omit any symbols or spaces when typing JA, WS, spell, or item names[this only applies to using /item to use items directly, not trades or other plugins].
* You may type normal numerals in place of roman numerals for tier 2+ spells, or use a 1 for tier 1 spells.
* You may type full or partial player, monster, or NPC names as your target.  Shorthand will find the best matching target that your spell can be cast on.
* You may omit your target entirely(/ma honormarch or //honormarch) and shorthand will default to casting on yourself for spells that are self-target only, or your current target for other spells.
* You may prefix any spell, JA, WS, or item with // instead of /ma, /ja, /ws, or /item.  Note that if this is ambiguous, such as in the case of Fire IV(spell) and Fire IV(blood pact), you may not always get the desired result.  Shorthand will prefer Spell > Ability > Weaponskill > Item, and prefer actions you know over actions you don’t, to make the best attempt at selecting the correct action.  If that is not sufficient, you should not use // and instead use a more specific command to ensure the correct action is selected.
* You may also add additional spell/ability/item/ws names and aliases through the config file located at Ashita/config/shorthand/settings.xml : see ‘Ashita/docs/shorthand/XML Layout.htm’.  This config file will be automatically generated on first load if it does not exist.

## Parsing
Shorthand will evaluate any commands beginning with any of these prefixes followed by a space.  (Example: '/item warpring' is valid, but '/itemwarpring' is not.)
* /ra
* /range
* /shoot
* /throw
* /a
* /attack
* /ta
* /target
* /ma
* /magic
* /ja - Note that this also triggers pet commands.
* /jobability - Note that this also triggers pet commands.
* /pet - Note that this also triggers job abilities.
* /ws
* /weaponskill
* /i - Note that this is not a standard game command.  It will be treated the same as /item.
* /item - Note that this only helps when using items.  It will not adjust anything when attempting to trade to a target.
* // - This may be used with or without a space, and will match JA, WS, magic, and items.  (//warpring or // warp ring are both valid.)

### Commands
All commands can be prefixed with /shorthand or /shh.

* /shh export<br>
This exports a settings xml with the currently available spells/abilities/weaponskills to Ashita/config/shorthand/settings-empty.xml.  You can use this to copy/paste abilities easier, or copy it to your live settings.xml.

* /shh reload<br>
This reloads your current settings xml, located at Ashita/config/shorthand/settings.xml.

* /shh packetws [optional: on/off]<br>
When enabled, this sends weaponskills via packets.  This allows you to ws disengaged.  Note that this will not apply to commands using any of the following targeting mechanisms:<br>
bt, ht, ft, st, stpc, stnpc, stpt, stal, lastst, r, scan

* /shh impact [optional: on/off]<br>
When enabled, this modifies the game client to believe you currently know Impact.  This allows you to cast through menu, or by typing the command, without Twilight Cloak on.<br>
Note that the server still requires Twilight Cloak equipped for cast to begin and complete, so you must equip it in the macro prior to the cast line or have Ashitacast configured to equip it in precast and midcast.

* /shh honor [optional: on/off]<br>
When enabled, this modifies the game client to believe you currently know Honor March.  This allows you to cast through menu, or by typing the command, without Marsyas on.<br>
Note that the server still requires Marsyas equipped for cast to begin and complete, so you must equip it in the macro prior to the cast line or have Ashitacast configured to equip it in precast and midcast.

* /shh dispelga [optional: on/off]<br>
When enabled, this modifies the game client to believe you currently know Dispelga.  This allows you to cast through menu, or by typing the command, without Daybreak on.<br>
Note that the server still requires Daybreak equipped for cast to begin and complete, so you must equip it in the macro prior to the cast line or have Ashitacast configured to equip it in precast and midcast.

* /shh debug [optional: on/off]<br>
When enabled, all typed commands will be printed to Ashita’s debug log.  You must have Ashita’s log level set to debug for them to be recorded.

* /raw [required: command]<br>
This sends a command through to the game client without allowing shorthand to evaluate it.  The first 5 characters (/raw and the trailing space) are trimmed, everything else is left untouched.
