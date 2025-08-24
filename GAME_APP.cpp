/*



//extern input handling---------------------
extern int16vect globalNavPos; //mouse position
extern int16vect Navlimits_; //how far the mouse is allowed to go
//extern input handling---------------------



//missule command esque game. because why not. one dial makes it trivial
//so esentially left and right aim your cannon, and you press enter to fire. 
//enemies are either orbital rocks(thrown by aliens), requiring lasers, or alien ships, which reflect lasers and require the railgun(fast projectile). nukes blow up an entire area, and can be aimed midair, but are limited
typedef enum{RAILCANNON,LASGUN_ARRAY,ICBM,GOI_FIREMODE_COUNT}GOI_FIREMODE;


void G_ORBITAL_INVADERS_Input(uint16_t key){

switch(key){

key_back:
//exit app, but with a confirm idk
break;

key_enter: 
break;

key_left:

 break;

key_right: 
break;



//switch fire modes
key_up:

break;

key_down:

break;

default: break;

}//e switch

}



void on_Start_G_ORBITAL_INVADERS(){
Navlimits_={GOI_AIM_SUBDIVISIONS,GOI_FIREMODE_COUNT,0}
//GOI_AIM_SUBDIVISIONS is how many segments the sky is divided into from 128(screen width) aiming is GOI_AIM_PIXELS,automatically made via .h file
rst_nav_pos();

}

void on_close_G_ORBITAL_INVADERS(){

}

//do i use an object for the game to spawn it's state? 


void GameTask(void* pvParameters) {
    

    while (true) {
      
      
        
        vTaskDelay(pdMS_TO_TICKS(50));  //go to the h file if you wanna change it
    }
}


*/