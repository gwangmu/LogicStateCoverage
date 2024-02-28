/*
 * lscov - emoji table 
 * -------------------
 *  
 * Absolutely no reason. Just for sh*ts and giggles.
 * Courtesy of ASCIImoji. (https://asciimoji.com)
 */

#pragma once
#include <time.h>

static const char* emoji[] = {
  "( •_•)O*¯`·.¸.·´¯`°Q(•_• )",
  "O=('-'Q)",
  "(ノಠ益ಠ)ノ彡┻━┻",
  "(=^･ｪ･^=))ﾉ彡☆",
  "ò_ô",
  "(◔_◔)",
  "(╯°□°)╯",
  "ε(´סּ︵סּ`)з",
  "( ͡° ʖ̯ ͡°)",
  "╚(•⌂•)╝",
  "¯\\_(ツ)_/¯",
  "ツ",
  "☺︎",
  "¬‿¬",
  "(;´༎ຶД༎ຶ`)",
  "ᕙ(⇀‸↼‶)ᕗ",
  "(๑•́ ヮ •̀๑)",
  "\\_(-_-)_/",
  "(ノ ゜Д゜)ノ ︵ ┻━┻",
  "(ಥ﹏ಥ)",
  "୧༼ಠ益ಠ༽︻╦╤─",
  "\\(^-^)/",
  "( ͡° ͜ʖ ͡°)_/¯",
  "|=-(¤)-=|",
  "(=____=)",
  "ᕦ(òᴥó)ᕥ",
  "(っ^з^)♪♬",
  "(°o•)",
  "＼(＾O＾)／",
  "(⊙＿⊙')",
  "⊙ω⊙",
  "\\( ﾟヮﾟ)/",
  "⊹╰(⌣ʟ⌣)╯⊹",
  "[¬º-°]¬",
  "(-(-_-(-_(-_(-_-)_-)-_-)_-)_-)-)",
  "(◡_◡)ᕤ",
  "(ง •̀_•́)ง",
  "ᕙ(`▽´)ᕗ",
  "ᕕ( ᐛ )ᕗ",
  "._.)/\\(._.",
  "(❍ᴥ❍ʋ)",
  "≧◡≦",
  "(づ ￣ ³￣)づ",
  "( ͡° ͜ʖ ͡°)",
  "(ง ͠° ͟ʖ ͡°)ง",
  "ᕦ( ͡° ͜ʖ ͡°)ᕤ",
  "¯\\_( ͡° ͜ʖ ͡°)_/¯",
  "ಠ_ಠ",
  "ʕ♥ᴥ♥ʔ",
  "ಡ_ಡ",
  "(⌐⊙_⊙)",
  "╚(•⌂•)╝",
  "(•_•) ( •_•)>⌐■-■ (⌐■_■)",
  "(ㆆ _ ㆆ)",
  "⊂(◉‿◉)つ",
  "WHΣИ $HΛLL WΣ †HЯΣΣ MΣΣ† ΛGΛ|И?",
  "вєωαяє, ι αм ƒαη¢у!",
  "Yᵒᵘ Oᶰˡʸ Lᶤᵛᵉ Oᶰᶜᵉ",
  "ꜰꜱᴄᴋ ɢᴇɴᴏᴄɪᴅᴇ",
};

static const char* random_emoji() { 
  return emoji[time(NULL) % (sizeof(emoji)/sizeof(const char *))];
}
