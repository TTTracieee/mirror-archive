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
    {"Lia","I'm Lia. This was my grandma Tang's box.\nShe kept this old photo with it.",0,Cue::ShowKeepsakes},
    {"You","Please, take your time.\nTell me about her."},
    {"Lia","She died last month. I miss her.\nI want to know what she kept inside.",1},
    {"Lia","The password is her first home's address.\nThat is what she told me.\nBut I have never been there."},
    {"You","I'll take good care of them.\nLet's find that house number."},
    {"tonton","Brush with Clean, then try Contrast.\nLook for the house number.\nTry the box when you can read it.",1}
};}

inline std::vector<Speech> foundEnvelope(){return {
    {"tonton","An envelope!\nLet's open it.",2}
};}
inline std::vector<Speech> foundMap(){return {
    {"tonton","There's a map inside!\nIt says, 'For little Tang.'\nThat's Lia's grandma!",2},
    {"tonton","Open Photo and compare the houses.\nClick a house for a closer look.\nThen press Start camera."}
};}
inline std::vector<Speech> reachedHome(){return {
    {"tonton","We made it! There's writing on the back.\nClick the folded corner. Let's see.",3}
};}

inline std::vector<Speech> ending(bool peopleRestored=false){return {
    {"Lia","Grandma left a light on for me.\nEven when I came home late.\nNow I know why.",1},
    {"You","You do not have to stop missing her.\nYou can keep her love with you."},
    {"You","Here is the photo I worked on.\nHere. Take it home with the letter.",0,Cue::ReturnPhoto},
    {"Lia",peopleRestored?"I can see them holding hands.\nGrandma held my hand like that, too.":"Thank you for taking care of her photo.\nI will keep it close.",2},
    {"Lia","I'll put them by my bed.\nI'll keep a light on, too.",2},
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
