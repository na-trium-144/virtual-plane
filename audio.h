// 初期化
void init_wav();
void init_pa();

// BGM変更
void bgm_change(enum GameState g);
void bgm_stop();
// SE再生
enum se { se_ready, se_coin, se_shoot, se_hit, se_yakan };
void se_play(enum se c);