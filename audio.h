void init_wav();
void init_pa();

void bgm_change(enum GameState g);
void bgm_stop();
enum se { se_ready, se_coin, se_shoot, se_hit, se_yakan };
void se_play(enum se c);