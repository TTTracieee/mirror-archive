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
    {"Lia","I'm Lia.\nI brought my grandma Tang's box and photo.",0,Cue::ShowKeepsakes},
    {"You","Take your time.\nWhat would you like us to help with?"},
    {"Lia","Grandma died last month.\nI miss her so much.",1},
    {"Lia","I want to know what's inside this box.\nI haven't been able to open it.",1},
    {"You","Did she ever mention the password?"},
    {"Lia","She said it was her first house number,\nbut I've never seen that house."},
    {"You","Maybe the number is in this old photo.\nLet's take a closer look."},
    {"tonton","The house number has faded.\nLet's restore the photo first.",1,Cue::None,false,Focus::Ready},
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
    {"tonton","Now let's use the password from the photo\nto open the box."},
    {"tonton","Write one number in each box."},
    {"tonton","Check the numbers above your writing\nbefore pressing Open."},
    {"tonton","Let's see if it opens!",1}
};}

inline std::vector<Speech> foundEnvelope(){return {
    {"tonton","It opened! You did it!",3},
    {"tonton","There's an envelope inside.",2},
    {"tonton","Let's see what's inside!\nClick the envelope to open it.",1}
};}
inline std::vector<Speech> foundMap(){return {
    {"tonton","There's a map inside!",2},
    {"tonton","It says, 'For my sweetheart Tang.'\nThat's Lia's grandma!",1},
    {"tonton","This looks like a little game.\nLet's guide the letter to her first home."},
    {"You","The house in the old photo is our clue."},
    {"tonton","Click Photo to see your restored picture.\nLook for the same house on the map."},
    {"tonton","Let's follow the map!\nPress Start camera when you're ready.",1}
};}
inline std::vector<Speech> cameraGuide(){return {
    {"tonton","Show your hand in the middle of the view."},
    {"tonton","Move your hand to one side.\nHold it there until the ring fills."},
    {"tonton","The letter follows the path and stops\nat the next corner or fork."},
    {"tonton","Bring your hand back to the middle\nbefore choosing the next direction."},
    {"tonton","Let's bring it home!",1}
};}
inline std::vector<Speech> reachedHome(){return {
    {"tonton","You brought it home!\nGreat work!",3},
    {"tonton","Wait... there's writing on the back."},
    {"tonton","Let's turn it over.\nClick the folded corner.",1}
};}

inline std::vector<Speech> wrongHouse(){return {
    {"tonton","This house looks different from the photo.\nLet's check Photo again."}
};}
inline std::vector<Speech> tryPasswordAgain(){return {
    {"tonton","The numbers above show what I read.\nUse Clear if a number looks wrong."},
    {"tonton","If they match your writing,\nlet's check the photo again."}
};}

inline std::vector<Speech> ending(bool peopleRestored=false){return {
    {"You","Your grandma's mother left her a message\non the back of the map."},
    {"Lia","Grandma always left a light on for me.\nNow I know why.",1},
    {"Lia","I wish I could thank her one more time.",1},
    {"You","I know you miss her.\nI hope this helps you feel close to her."},
    {"You","Here is the photo I worked on.\nTake it home with the letter.",0,Cue::ReturnPhoto},
    {"Lia",peopleRestored?"I can see them holding hands.\nGrandma held my hand like that, too.":"Thank you for taking care of her photo.\nI will keep it close.",2},
    {"Lia","I'll put them by my bed.\nI'll keep a light on, too.",2},
    {"You","Take care, Lia.\nYou are always welcome here."},
    {"Lia","Thank you, both of you.\nI will come back.",2},
    {"You","I watch Lia leave with her photo.\nThe room feels quiet again.",0,Cue::LiaLeaves,true},
    {"tonton","I'm glad she came to us.",1},
    {"You","Me too.\nI'm glad we could help."},
    {"tonton","Shall we leave a light on tonight?",1},
    {"You","Yes. Someone else may need our help\ntomorrow."},
    {"You","I put away my tools.\nIt is time to go home.",0,Cue::None,true}
};}

inline const std::string letter=
    "My dear Tang,\n\n"
    "If you get lost, take your time.\n"
    "You will find your way home.\n\n"
    "I will leave the light on for you.\n\n"
    "Love, Mom";
}
