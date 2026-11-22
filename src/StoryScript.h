#pragma once
#include <string>
#include <vector>

namespace story {
enum class Cue {None,Doorbell,ShowKeepsakes,ReturnPhoto,LiaLeaves};
struct Speech {
    std::string speaker,words;
    int happy=0;
    Cue cue=Cue::None;
    bool narration=false;
};

inline std::vector<Speech> opening(){return {
    {"You","I opened Mirror Archive to restore\nold photos, letters, and memories.",0,Cue::None,true},
    {"You","This is my little archive.\ntonton is my helper and my friend.",0,Cue::None,true},
    {"You","Someone is at the door.",0,Cue::Doorbell,true},
    {"tonton","Hello! Welcome to Mirror Archive.\nCome in. How can we help?",1},
    {"Lia","I'm Lia. This box was my grandma's.\nShe kept this old photo with it.",0,Cue::ShowKeepsakes},
    {"You","Please, take your time.\nTell me about her."},
    {"Lia","She died last month. I miss her.\nI want to know what she kept inside.",1},
    {"Lia","The password is her first home's address.\nThat is what she told me.\nBut I have never been there."},
    {"You","I'll take good care of them.\nLet's find that house number."},
    {"tonton","Drag a brush over the photo.\nClean spots, then try Contrast.\nFind the number, then try the box."}
};}

inline std::vector<Speech> ending(){return {
    {"Lia","Grandma left a light on for me.\nEven when I came home late.\nNow I know why.",1},
    {"You","You do not have to stop missing her.\nYou can keep her love with you."},
    {"You","I finished cleaning your photo.\nHere. Take it home with the letter.",0,Cue::ReturnPhoto},
    {"Lia","Thank you. I'll put them by my bed.\nI'll keep a light on, too.",2},
    {"You","Take care, Lia.\nCome back any time."},
    {"Lia","I will. Thank you, both of you.",2},
    {"You","I watch Lia leave with her photo.\nThe room feels quiet again.",0,Cue::LiaLeaves,true},
    {"tonton","We fixed more than a photo today.",3},
    {"You","We helped her bring a little of\nhome back with her."},
    {"tonton","Another light left on.",1},
    {"You","I put away my tools.\nIt is time to go home.",0,Cue::None,true}
};}

inline const std::string letter=
    "Dear little Tang,\n\n"
    "If you get lost, take your time.\n"
    "You will find your way home.\n\n"
    "I will leave the light on for you.\n\n"
    "Love, Mom";
}
