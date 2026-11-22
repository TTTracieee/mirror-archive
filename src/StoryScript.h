#pragma once
#include <string>
#include <vector>

namespace story {
enum class Cue {None,Doorbell,ShowKeepsakes,ReturnPhoto,LiaLeaves};
enum class Focus {None,Clean,Contrast,Detail,Size,Strength,Original,Ready};
inline const std::string dedication="For my sweetheart Tang";

struct Speech {
    std::string speaker,words;
    int happy=0;
    Cue cue=Cue::None;
    bool narration=false;
    Focus focus=Focus::None;
};

inline std::vector<Speech> opening(){return {
    {"You","I opened Mirror Archive to restore\nold photos, letters, and memories.",0,Cue::None,true},
    {"You","This is my little archive.\ntonton is my helper and my friend.",0,Cue::None,true},
    {"You","Someone is at the door.",0,Cue::Doorbell,true},
    {"tonton","Welcome to Mirror Archive!\nHow can we help?",1},
    {"Lia","I'm Lia.\nThese belonged to my grandma Tang.",0,Cue::ShowKeepsakes},
    {"You","Please, take your time.\nTell me about her."},
    {"Lia","Grandma died last month.\nI miss her.",1},
    {"Lia","She said the password was her first\nhome's address, but I've never been there."},
    {"You","I'll take good care of them.\nLet's find that house number."},
    {"tonton","Let's start with Clean.\nDrag over the photo to remove dust.",1,Cue::None,false,Focus::Clean},
    {"tonton","Use Contrast to bring back faded ink.",0,Cue::None,false,Focus::Contrast},
    {"tonton","Use Detail to make small edges clearer.",0,Cue::None,false,Focus::Detail},
    {"tonton","Use Size to make your brush\nwider or smaller.",0,Cue::None,false,Focus::Size},
    {"tonton","Use Strength to make the effect\nstronger or softer.",0,Cue::None,false,Focus::Strength},
    {"tonton","Hold Original to compare your work\nwith the damaged photo.",0,Cue::None,false,Focus::Original},
    {"tonton","Let's find that house number!",1,Cue::None,false,Focus::Ready}
};}

inline std::vector<Speech> foundNumber(){return {
    {"tonton","Nice work!",3},
    {"tonton","Write one number in each box."},
    {"tonton","Check the small windows before\npressing Open."},
    {"tonton","Let's try your number!",1}
};}

inline std::vector<Speech> foundEnvelope(){return {
    {"tonton","It opened! You did it!",3},
    {"tonton","There's an envelope inside.",2},
    {"tonton","Let's open it!",1}
};}
inline std::vector<Speech> foundMap(){return {
    {"tonton","There's a map inside!",2},
    {"tonton","It says, 'For my sweetheart Tang.'\nThat's Lia's grandma!",1},
    {"tonton","Click a map house to see it\nbeside your own photo."},
    {"tonton","Use Photo if you want to keep repairing."},
    {"tonton","Let's find her home!\nPress Start camera when you're ready.",1}
};}
inline std::vector<Speech> cameraGuide(){return {
    {"tonton","Hold your hand in the middle first."},
    {"tonton","Move it to one side and wait\nfor the ring to fill."},
    {"tonton","Bring it back to the middle\nbefore choosing another direction."},
    {"tonton","Let's give it a try!",1}
};}
inline std::vector<Speech> reachedHome(){return {
    {"tonton","You brought it home!\nGreat work!",3},
    {"tonton","Wait... there's writing on the back."},
    {"tonton","Let's turn it over.\nClick the folded corner.",1}
};}

inline std::vector<Speech> ending(bool peopleRestored=false){return {
    {"Lia","Grandma always left a light on for me.\nNow I know why.",1},
    {"You","You do not have to stop missing her.\nYou can keep her love with you."},
    {"You","Here is the photo I worked on.\nTake it home with the letter.",0,Cue::ReturnPhoto},
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
    "My dear Tang,\n\n"
    "If you get lost, take your time.\n"
    "You will find your way home.\n\n"
    "I will leave the light on for you.\n\n"
    "Love, Mom";
}
