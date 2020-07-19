#include <stdio.h>
#include <string.h>

#include <wiringPi.h>
#include <wiringSerial.h>

// 駒の種別
#define OU 1
#define HI 2
#define KA 3
#define KI 4
#define GI 5
#define KE 6
#define KY 7
#define FU 8
#define RY 9
#define UM 10
#define NG 11
#define NK 12
#define NY 13
#define TO 14

// 駒の所持者
#define SENTE 1
#define GOTE  2

// 駒の状態
#define OMOTE 1
#define URA   2
#define MOCHIGOMA 3

// 処理の実行結果
#define SUCCESS 1
#define ERROR	0

// stateの定義
#define FIRST_INPUT	0
#define MOVE_INPUT	1
#define GET_KOMA	2
#define USE_MOCHIGOMA	3

// 駒が持たれた or 置かれたを定義
#define CHANGE_to_0	0 // 駒が持ち上げられた状態(配列「board0」が0に変わるという意味)
#define CHANGE_to_1	1 // 駒が置かれた状態      (配列「board0」が0から1以上に変わるという意味)

// 通信において、データの取得に成功した時に返す値
#define GET_OK 0x78

// 盤面の大きさを決める定数
#define D 9 //D×D盤面

#define JINCHI_GOTE 3//後手の陣地が何列目からか
#define JINCHI_SENTE 7 //先手の陣地が何列目からか

// 盤面にどんな駒が置かれているかを記憶する二次元配列 (初期値は将棋の初期盤面)
short int board0[10][10] = { 	{ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9},
								{ 1, 21, 17, 13,  9,  2, 10, 14, 18, 22},
						  		{ 2,  0,  6,  0,  0,  0,  0,  0,  4,  0},
							  	{ 3, 32, 33, 34, 35, 36, 37, 38, 39, 40},
							  	{ 4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
								{ 5,  0,  0,  0,  0,  0,  0,  0,  0,  0},
								{ 6,  0,  0,  0,  0,  0,  0,  0,  0,  0},
								{ 7, 23, 24, 25, 26, 27, 28, 29, 30, 31},
								{ 8,  0,  3,  0,  0,  0,  0,  0,  5,  0},
								{ 9, 19, 15, 11,  7,  1,  8, 12, 16, 20} };

// 将棋で使われる40個の駒の状態と利きを保存する配列
short int koma_data[40][45] = { { 9, 5, SENTE, OMOTE, OU},
			  					{ 1, 5,  GOTE, OMOTE, OU},
				 				{ 8, 2, SENTE, OMOTE, HI},
							 	{ 2, 8,  GOTE, OMOTE, HI},
							 	{ 8, 8, SENTE, OMOTE, KA},
							 	{ 2, 2,  GOTE, OMOTE, KA},
							 	{ 9, 4, SENTE, OMOTE, KI},
						 		{ 9, 6, SENTE, OMOTE, KI},
							 	{ 1, 4,  GOTE, OMOTE, KI},
						 		{ 1, 6,  GOTE, OMOTE, KI},
							 	{ 9, 3, SENTE, OMOTE, GI},
							 	{ 9, 7, SENTE, OMOTE, GI},
							 	{ 1, 3,  GOTE, OMOTE, GI},
							 	{ 1, 7,  GOTE, OMOTE, GI},
							 	{ 9, 2, SENTE, OMOTE, KE},
							 	{ 9, 8, SENTE, OMOTE, KE},
							 	{ 1, 2,  GOTE, OMOTE, KE},
						 		{ 1, 8,  GOTE, OMOTE, KE},
							 	{ 9, 1, SENTE, OMOTE, KY},
							 	{ 9, 9, SENTE, OMOTE, KY},
							 	{ 1, 1,  GOTE, OMOTE, KY},
							 	{ 1, 9,  GOTE, OMOTE, KY},
							 	{ 7, 1, SENTE, OMOTE, FU},
							 	{ 7, 2, SENTE, OMOTE, FU},
						 		{ 7, 3, SENTE, OMOTE, FU},
							 	{ 7, 4, SENTE, OMOTE, FU},
							 	{ 7, 5, SENTE, OMOTE, FU},
							 	{ 7, 6, SENTE, OMOTE, FU},
							 	{ 7, 7, SENTE, OMOTE, FU},
							 	{ 7, 8, SENTE, OMOTE, FU},
							 	{ 7, 9, SENTE, OMOTE, FU},
							 	{ 3, 1,  GOTE, OMOTE, FU},
						 		{ 3, 2,  GOTE, OMOTE, FU},
							 	{ 3, 3,  GOTE, OMOTE, FU},
							 	{ 3, 4,  GOTE, OMOTE, FU},
							 	{ 3, 5,  GOTE, OMOTE, FU},
							 	{ 3, 6,  GOTE, OMOTE, FU},
							 	{ 3, 7,  GOTE, OMOTE, FU},
							 	{ 3, 8,  GOTE, OMOTE, FU},
							 	{ 3, 9,  GOTE, OMOTE, FU}  };

// 駒の表と裏の関係を定義
short int koma_shurui[14][3] = {{OU, 0 , 0},
						     	{HI, RY, 1},
						     	{KA, UM, 2},
						     	{KI, 0 , 3},
						     	{GI, NG, 4},
						     	{KE, NK, 5},
						     	{KY, NY, 6},
						     	{FU, TO, 7},
						     	{RY, HI, 8},
						     	{UM, KA, 9},
						     	{NG, GI, 3},
						     	{NY, NY, 3},
						     	{TO, FU, 3}  };

// 駒の動けるマス(利き)をその駒を中心として、相対的に定義
short int kiki_data[10][24] = { 	{ 1, 1, 1,   1, 0, 1,   1,-1, 1,   0, 1, 1,   0,-1, 1,  -1, 1, 1,  -1, 0, 1,  -1,-1, 1},
						     	{ 1, 0, 2,   0, 1, 2,   0,-1, 2,  -1, 0, 2},
						     	{ 1, 1, 2,   1,-1, 2,  -1, 1, 2,  -1,-1, 2},
						     	{ 1, 0, 1,   0, 1, 1,   0,-1, 1,  -1, 1, 1,  -1, 0, 1,  -1,-1, 1},
						     	{ 1, 1, 1,   1,-1, 1,  -1, 1, 1,  -1, 0, 1,  -1,-1, 1},
						     	{-2, 1, 1,  -2,-1, 1},
						     	{-1, 0, 2},
						     	{-1, 0, 1},
						     	{ 1, 1, 1,   1, 0, 2,   1,-1, 1,   0, 1, 2,   0,-1, 2,  -1, 1, 1,  -1, 0, 2,  -1,-1, 1},
						     	{ 1, 1, 2,   1, 0, 1,   1,-1, 2,   0, 1, 1,   0,-1, 1,  -1, 1, 2,  -1, 0, 1,  -1,-1, 2}  };

// ターミナル上に将棋の様子を表示する処理
void hyouji(void);
void hyouji_printf(short int a);
void color_printf(short int a, short int font);
void hyouji_kiki(short int a);

// ほぼmain関数みたいなもの。将棋の振る舞いを決める「脳」という意味で命名
short int brain(void);

// データの入力(H8マイコンからシリアル通信データが送られてくることを想定)
void input_data(short int *tate_input, short int *yoko_input, short int *change, short int *koma);

// シリアル通信用処理
void serial( short int *tate_input, short int *yoko_input, short int *change, short int *koma );
void trans(short int a, short int x, short int i); //a:データ送信先(1 or 3) x:1->OK or データ有り　2->NO or データ無し　i:利きを表示する駒

// 一手戻す処理
short int back_one_hand (short int tate_input, short int yoko_input, short int change, short int koma);

// 盤面を進める or 戻す処理
void next_board(short int c, short int d, short int x, short int i, short int order);
void back_board(short a, short b, short c, short d, short x, short i, short *tegoma, short nari, short komatori);

// 駒の成りを判定
short int judge_nari(short int b, short int d, short int i, short int order);

// 駒が一段目(先手) or 9段目(後手)かどうか判定
short int judge_1danme(short int d, short int i, short int order);

// 利きを配列「koma_data」に格納する処理
void store_kiki(void);

// 王に利きが利いているか判定
short int judge_kiki_ou(short int order, short int x);

// 駒が動けるか判定
short int judge_koma_move(short int c, short int d, short int i);

// 駒の利き次第によって、王が動けるマスが制限される。そのため、王が制限されているマスを配列「koma_data」から削除する
void remove_ou_kiki(void);

// 王が積んでいるか判定
short int judge_tsumi(short int order, short int oute);

// Raspberry Pi のブザー音を鳴らす
short int buzzer(short int a);

void koma_test(void) {
	unsigned char receive_data[81] = { };
	unsigned char i, b;

	int fd = serialOpen("/dev/serial0", 115200);
	if ( fd < 0 ) {
		printf("can not open serialport");
	}

	while ( 1 ) {
		b = 0;
		while (  serialDataAvail( fd ) ) {
			for(i=0; i<81; i++) {
				receive_data[ i ]  = serialGetchar( fd );
				//receive_data[ 1 ]  = serialGetchar( fd );
			}
			fflush( stdout );
			for(i=0; i<81; i++) {
				printf(" receive_data : %d \n", receive_data[i]);
				//printf(" receive_data : %d \n", receive_data[1]);
			}
			printf("trans : GET_OK\n");
			serialPutchar( fd, GET_OK);
			b = 1;
		}
		if (b == 1) break;
	}

	return;
}

int main(void){
/*	while(1){
		koma_test();
	}
*/	brain();
	
/*	int fd = serialOpen("/dev/serial0", 115200);
	int i, j;
	if ( fd < 0 ) {
		printf("can not open serialport");
	}
	for(i = 0; i<100000; i++) {
		for (j = 0; j < 10000; j++);
	}
	while(1){
		for(i = 0; i<10000; i++) {
			for (j = 0; j < 10000; j++);
		}
		serialPutchar( fd, 0xF3);
		printf("trans : 0xF3\n");
	}
*/	

	return 0;
}


void hyouji(void) {
	short int i, j;
	printf("\n");
	printf("後手の持ち駒：");
	for (i = 0; i < 40; i++) {
		if (koma_data[i][3] == MOCHIGOMA && koma_data[i][2] == GOTE) {
			color_printf(i, koma_data[i][4]);
		}	
	}
	printf("\n\n");
			
	for(i = 0;  i <= D;  i++) {
		for (j = D;  j >= 0; j--) {
			if (i == 0) {
				if (j == 0) printf("\n");
				else        printf("   %d  ", board0[i][j]);
			} else if (j == 0) {
				printf(" %d\n\n", board0[i][j]);
			} else if (board0[i][j] == 0){
				printf("      ");
			} else { 
				hyouji_printf(board0[i][j]-1);
			}
		}
	}
	printf("先手の持ち駒：");
	for (i = 0; i < 40; i++) {
		if (koma_data[i][3] == MOCHIGOMA && koma_data[i][2] == SENTE) {
			color_printf(i, koma_data[i][4]);
		}	
	}
	printf("\n\n");
}

void hyouji_printf(short int a) {
	short int font;
	if (koma_data[a][3] == OMOTE) {
		font = koma_data[a][4];
	} else if  (koma_data[a][3] == URA) {
		font = *( *(koma_shurui + koma_data[a][4] - 1) + 1 );
	}
	printf(" ");
	color_printf(a, font);
	printf(" ");
}

// 先手と後手の違いによって、表示の色を変えて駒を表示する処理
void color_printf(short int a, short int font) {
	if (koma_data[a][2] == SENTE) {
		printf("\x1b[31m");	
	} else {
		printf("\x1b[34m"); 
	}
	if      (font == OU) printf(" 玉 ");
	else if (font == HI) printf(" 飛 ");
	else if (font == KA) printf(" 角 ");
	else if (font == KI) printf(" 金 ");
	else if (font == GI) printf(" 銀 ");
	else if (font == KE) printf(" 桂 ");
	else if (font == KY) printf(" 香 ");
	else if (font == FU) printf(" 歩 ");
	else if (font == RY) printf(" 竜 ");
	else if (font == UM) printf(" 馬 ");
	else if (font == NG) printf("成銀");
	else if (font == NK) printf("成桂");
	else if (font == NY) printf("成香");
	else if (font == TO) printf(" と ");
	printf("\x1b[39m");
}

// 駒が持たれたときに、利きを表示する処理
void hyouji_kiki(short int a) {
	short int i, j, n, m, font;
	printf("\n");
	printf("後手の持ち駒：");
	for (i = 0; i < 40; i++) {
		if (koma_data[i][3] == MOCHIGOMA && koma_data[i][2] == GOTE) {
			color_printf(i, koma_data[i][4]);
		}	
	}
	printf("\n\n");
			
	for(i = 0;  i <= D;  i++) {
		for (j = D;  j >= 0; j--) {
			printf(" ");
			for (n=5, m=6; n < 45; n+=2, m+=2) {
				if (koma_data[a][n] == i && koma_data[a][m] == j) printf("\x1b[45m");
			}
			if (i == 0) {
				printf("\x1b[49m");
				if (j == 0) printf("\n");
				else        printf("  %d ", board0[i][j]);
			} else if (j == 0) {
				printf("\x1b[49m");
				printf("%d\n\n", board0[i][j]);
			} else if (board0[i][j] == 0){
				printf("    ");
			} else { 
				if (koma_data[board0[i][j]-1][3] == OMOTE) {
					font = koma_data[board0[i][j]-1][4];
				} else if  (koma_data[board0[i][j]-1][3] == URA) {
					font = *( *(koma_shurui + koma_data[board0[i][j]-1][4] - 1) + 1 );
				}
				color_printf(board0[i][j]-1, font);
			}
			printf("\x1b[49m");
			printf(" ");
		}
	}
	printf("先手の持ち駒：");
	for (i = 0; i < 40; i++) {
		if (koma_data[i][3] == MOCHIGOMA && koma_data[i][2] == SENTE) {
			color_printf(i, koma_data[i][4]);
		}	
	}
	printf("\n\n");
}

void input_data(short int *tate_input, short int *yoko_input, short int *change, short int *koma) {
	unsigned char t, y, ch, ko1;
	unsigned short int ko;
	unsigned char t1, y2, ch1, ko2;
	unsigned char receive_data[2] = { };

	printf("tate  = ");	scanf(" %c", &t);
	printf("yoko  = ");	scanf(" %c", &y);
	printf("change= ");	scanf(" %c", &ch);
	printf("koma  = ");	scanf(" %hd", &ko);

	t = t - 48;
	y = y - 48;
	ch = ch - 48;
	ko1 = (char)ko;

	t1 = t << 4;
	ch1 = ch << 7;
	receive_data[0] = t1 + y;
	receive_data[1] = ch1 + ko;

	*tate_input = (short int) (receive_data[0] >> 4);
	y2 = receive_data[0] << 4;
	*yoko_input = (short int) (y2 >> 4);
	*change = (short int) (receive_data[1] >> 7);
	ko2 = receive_data[1] << 1;
	*koma = (short int) (ko2 >> 1);

	printf("tate:%hd, yoko:%hd, change:%hd, koma:%hd \n", *tate_input, *yoko_input, *change, *koma);

}

void serial(short int *tate_input, short int *yoko_input, short int *change, short int *koma) {
	unsigned char receive_data[2] = { };
	unsigned char y2, ko2, b;
//	unsigned char receive_data_past[2];

	int fd = serialOpen("/dev/serial0", 115200);
	if ( fd < 0 ) {
		printf("can not open serialport");
	}

	while ( 1 ) {
		b = 0;
		while (  serialDataAvail( fd ) ) {
			receive_data[ 0 ]  = serialGetchar( fd );
			receive_data[ 1 ]  = serialGetchar( fd );
			fflush( stdout );
			printf(" receive_data : %d \n", receive_data[0]);
			printf(" receive_data : %d \n", receive_data[1]);
			*tate_input = (short int) (receive_data[0] >> 4);
			y2 = receive_data[0] << 4;
			*yoko_input = (short int) (y2 >> 4);
			*change = (short int) (receive_data[1] >> 7);
			ko2 = receive_data[1] << 1;
			*koma = (short int) (ko2 >> 1);

			printf("tate:%hd, yoko:%hd, change:%hd, koma:%hd \n", *tate_input, *yoko_input, *change, *koma);

//			printf("trans : GET_OK\n");
//			serialPutchar( fd, GET_OK);
			b = 1;
		}
		if (b == 1) break;
	}

	return;
}

void trans(short int a, short int x, short int i){ //a:データ送信先(1 or 3) x:1->OK or データ有り　2->NO or データ無し　i:利きを表示する駒
	short int j, data_leng;
	unsigned char trans_data[43];
	int fd = serialOpen("/dev/serial0", 115200);
	if ( fd < 0 ) {
		printf("can not open serialport");
	}

	if (a == 1) {
		if (x == 1) {
			printf("trans : GET_OK\n");
			serialPutchar( fd, 0x78);//0x78を送信
		}
		else {
			printf("trans : Break\n");
			serialPutchar( fd, 0x47);///0x47
		}
	}
	else {
		if (x == 1) {
			printf("trans : LED_data\n");
			trans_data[0] = 0xF8;
			for (j = 1; j < 41; j++) {
				if (koma_data[i][j+4] == 0) break;
				trans_data[j] = (unsigned char)koma_data[i][j+4];
			}
			if (j == 41) {
				trans_data[41] = 0xF3;
				trans_data[42] = 0x00;
				data_leng = 41;
			}
			else {
				trans_data[j+1] = 0xF3;
				trans_data[j+2] = 0x00;
				data_leng = j+1;
			}
			for (j = 0; j <= data_leng; j++) {
				serialPutchar( fd, trans_data[ j ]);///trans_data送信
			}
		}
		else {
			serialPutchar( fd, 0xC7);///0xC7
		}
	}
}

short int buzzer(short int a) {
 	int i;

	if (wiringPiSetup( ) == -1) return 1;
	pinMode(0, OUTPUT);

	if (a == 1){
		for ( i = 0; i < 2; i++) {
			digitalWrite(0, 0);
			delay(10);
			digitalWrite(0, 1);
			delay(200);
		} 
		digitalWrite(0, 0);
	}
	else {
		digitalWrite(0, 1);
		delay(1000);
		digitalWrite(0, 0);
	}
	return 0;
}

// ほぼmain関数みたいなもの。将棋の振る舞いを決める「脳」という意味で命名
short int brain(void) {
	short int state, state_past, order, oute, tsumi, nari, komatori, nifu, mochigoma_i;
	short int tegoma[5];
	short int tate_input, yoko_input, koma, change; //H8から得るデータを格納する変数
	short int tate_past[2] = { }; //縦の過去の入力を記憶するための配列
	short int yoko_past[2] = { }; //横の過去の入力を記憶するための配列
	short int koma_past[2] = { }; //駒の過去の入力を記憶するための配列
	short int i, j, k;

	order = SENTE;
	oute = 0;
	state = FIRST_INPUT;
	state_past = FIRST_INPUT;
	store_kiki();
	tsumi = 0;

	while(1) {

		if (state == FIRST_INPUT) {
			hyouji();
			trans( 3, 2,  0);
		}

		if (order == SENTE) printf("***先手の手番***\n");
		else		    printf("***後手の手番***\n");

		switch (state) {
			case FIRST_INPUT: printf("状態：FIRST_INPUT\n");
						break;
			case MOVE_INPUT: printf("状態：MOVE_INPUT\n");
						break;
			case GET_KOMA: printf("状態：GET_KOMA\n");
						break;
			case USE_MOCHIGOMA: printf("状態：USE_MOCHIGOMA\n");
						break;
		}
		if (oute != 0) printf("相手に王手をかけられています\n");


		//データを受け取る
//		input_data(&tate_input, &yoko_input, &change, &koma);
		serial(&tate_input, &yoko_input, &change, &koma);

		nari = 0;

		if (state == FIRST_INPUT) {

			if (change == CHANGE_to_1) {
				state_past = state;
				state = USE_MOCHIGOMA;
			}
			else if (change == CHANGE_to_0) {
				i = board0[tate_input][yoko_input] - 1;
				if (order == koma_data[i][2]) {
					state_past = state;
					state = MOVE_INPUT;
					hyouji_kiki( i ); //利きを表示
					trans( 3, 1,  i);
				}
				else {
					state_past = state;
					state = GET_KOMA;
					trans( 1, 1,  0);
				}
			}
		}
		else if	(state == MOVE_INPUT) {
			if (state_past == FIRST_INPUT) {

				if (change == CHANGE_to_1) { //普通の駒の移動

					if (tate_input == tate_past[0] && yoko_input == yoko_past[0]) { //上げた駒を元の場所に置く場合
						state_past = state;
						state = FIRST_INPUT;
					}
					else { //上げた駒を移動させる場合
						i = board0[ tate_past[0] ][ yoko_past[0] ] - 1;

						if (judge_koma_move(tate_input, yoko_input, i) == 0 ) { //動けるかどうか判定
							printf("この駒はそのマスへ動かすことはできません！！\n");
							buzzer( 1 );
							trans( 1, 1,  0);
							//元に戻す
							if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
							else									break;
						}

						if (koma == koma_past[0]) {  //成らない場合

							if (judge_1danme(tate_input, i, order) == 0) { //1段目の判定
								printf("そこは動かす場所がなくなるため置けません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}
						}
						else if (koma == koma_shurui[ koma_past[0] -1 ][ 1 ]) { //成る場合
							nari = judge_nari(tate_past[0], tate_input, i, order);
							if (nari == 0) {
								printf("成れません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}
							else if (nari == 2) {
								printf("成っている駒を再び戻すことはできません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}
						}
						else {
							break;//訳分からん
						}

						board0[ tate_past[0] ][ yoko_past[0] ] = 0;
						next_board(tate_input, yoko_input, 0, i, order);
						store_kiki();
						remove_ou_kiki();

						if (judge_kiki_ou(order, 1) != 0) {
							printf("自分の玉が取られます！！玉が取られないように入力しなおし！！\n");
							back_board(yoko_past[0], tate_past[0], yoko_input, tate_input, 0, i, tegoma, nari, 0);
							store_kiki();
							remove_ou_kiki();
							buzzer( 1 );
							trans( 1, 1,  0);
							//元に戻す
							if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
							else								    break;

							continue;
						}
						oute = judge_kiki_ou(order, 2);
						if (oute != 0) printf("相手に王手をかけました！！\n");
						tsumi = judge_tsumi(order, oute);
						if (tsumi == 1) break;

						state_past = state;
						state = FIRST_INPUT;
						if (order == SENTE) order = GOTE;
						else		    order = SENTE;
					}
				}
				else if (change == CHANGE_to_0) { 
					i = board0[ tate_input ][ yoko_input ] - 1;
					if (order == koma_data[i][2]) { //自分の駒を取る
						printf("自分の駒は取れません。\n");
						buzzer( 1 );
						trans( 1, 1,  0);
						//元に戻す
						if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
						else								    break;
					}
					else { //相手の駒を取る
						state_past = state;
						state = GET_KOMA;
						trans( 1, 1,  0);
					}
				}
			
			}
			else if (state_past == GET_KOMA) {
				if (change == CHANGE_to_1) {
					if (tate_input == tate_past[1] && yoko_input == yoko_past[1]) {
						if (koma == koma_past[0]) { //成らない場合
							i = board0[ tate_past[0] ][ yoko_past[0] ] - 1;
							if (judge_koma_move(tate_input, yoko_input, i) == 0 ) { //動けるかどうか判定
								printf("この駒はそのマスへ動かすことはできません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}

							if (judge_1danme(tate_input, i, order) == 0) { //1段目の判定
								printf("そこは動かす場所がなくなるため置けません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}

							if (board0[tate_input][yoko_input] > 0) {
								tegoma[0] = koma_data[board0[tate_input][yoko_input]-1][0];
								tegoma[1] = koma_data[board0[tate_input][yoko_input]-1][1];
								tegoma[2] = koma_data[board0[tate_input][yoko_input]-1][2];
								tegoma[3] = koma_data[board0[tate_input][yoko_input]-1][3];
								tegoma[4] = board0[tate_input][yoko_input] - 1;
							}
							board0[ tate_past[0] ][ yoko_past[0] ] = 0;
							next_board(tate_input, yoko_input, 0, i, order);
							store_kiki();
							remove_ou_kiki();

							if (judge_kiki_ou(order, 1) != 0) {
								printf("自分の玉が取られます！！玉が取られないように入力しなおし！！\n");
								back_board(yoko_past[0], tate_past[0], yoko_input, tate_input, 0, i, tegoma, nari, 1);
								store_kiki();
								remove_ou_kiki();
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;

								continue;
							}
							oute = judge_kiki_ou(order, 2);
							if (oute != 0) printf("相手に王手をかけました！！\n");
							tsumi = judge_tsumi(order, oute);
							if (tsumi == 1) break;

							state_past = state;
							state = FIRST_INPUT;

							if (order == SENTE) order = GOTE;
							else		    order = SENTE;

						}
						else if (koma == koma_shurui[ koma_past[0] -1 ][ 0 ]) { //成る場合
							i = board0[ tate_past[0] ][ yoko_past[0] ] - 1;
							if (judge_koma_move(tate_input, yoko_input, i) == 0 ) { //動けるかどうか判定
								printf("この駒はそのマスへ動かすことはできません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}

							nari = judge_nari(tate_past[0], tate_input, i, order);
							if (nari == 0) {
								printf("成れません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}
							else if (nari == 2) {
								printf("成っている駒を再び戻すことはできません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}

							if (board0[tate_input][yoko_input] > 0) {
								tegoma[0] = koma_data[board0[tate_input][yoko_input]-1][0];
								tegoma[1] = koma_data[board0[tate_input][yoko_input]-1][1];
								tegoma[2] = koma_data[board0[tate_input][yoko_input]-1][2];
								tegoma[3] = koma_data[board0[tate_input][yoko_input]-1][3];
								tegoma[4] = board0[tate_input][yoko_input] - 1;
							}
							board0[ tate_past[0] ][ yoko_past[0] ] = 0;
							next_board(tate_input, yoko_input, 0, i, order);
							store_kiki();
							remove_ou_kiki();

							if (judge_kiki_ou(order, 1) != 0) {
								printf("自分の玉が取られます！！玉が取られないように入力しなおし！！\n");
								back_board(yoko_past[0], tate_past[0], yoko_input, tate_input, 0, i, tegoma, nari, 1);
								store_kiki();
								remove_ou_kiki();
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;

								continue;
							}
							oute = judge_kiki_ou(order, 2);
							if (oute != 0) printf("相手に王手をかけました！！\n");
							tsumi = judge_tsumi(order, oute);
							if (tsumi == 1) break;

							state_past = state;
							state = FIRST_INPUT;

							if (order == SENTE) order = GOTE;
							else		    order = SENTE;

						}
						else if (koma == koma_past[1]) { //動かす自駒と取る駒が同じまたは成っている駒で、相手の駒を持ち上げて戻す行為は判別不可
							state_past = FIRST_INPUT;
							state = MOVE_INPUT;
							tate_input = tate_past[0];
							yoko_input = yoko_past[0];
							koma = koma_past[0];
							trans( 1, 1,  0);
						}
						else {
							break;//訳分からん　余地無し
						}
					}
					else if (tate_input == tate_past[0] && yoko_input == yoko_past[0]) {
						state_past = FIRST_INPUT;
						state = GET_KOMA;
						tate_input = tate_past[1];
						yoko_input = yoko_past[1];
						koma = koma_past[1];

						hyouji();
						trans( 3, 2,  0);
					}
					else {
						break;//訳分からん　余地無し
					}
				}
				else {
					break;//訳分からん　余地無し
					//考える余地あり　他の駒が持ち上げられる。それを元に戻せば続行できる。
				}
			}
	
		}
		else if	(state == GET_KOMA) {
			if (state_past == FIRST_INPUT) {
				i = board0[ tate_input ][ yoko_input ] - 1;
				if (order == koma_data[i][2]) { //自分の駒をあげる
					state_past = state;
					state = FIRST_INPUT;
					hyouji_kiki( i ); //利きを表示
					trans( 3, 1,  i);
				}
				else { //2回相手の駒を取る
					printf("相手の駒は2個取れません。\n");
					buzzer( 1 );
					trans( 1, 1,  0);
					//元に戻す
					if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
					else								    break;
				}
				
			}
			else if (state_past == MOVE_INPUT) {
				if (change == CHANGE_to_1) {
					if (tate_input == tate_past[0] && yoko_input == yoko_past[0]){
						if (koma == koma_past[1]) { //成らない場合
							i = board0[ tate_past[1] ][ yoko_past[1] ] - 1;
							if (judge_koma_move(tate_input, yoko_input, i) == 0 ) { //動けるかどうか判定
								printf("この駒はそのマスへ動かすことはできません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}

							if (judge_1danme(tate_input, i, order) == 0) { //1段目の判定
								printf("そこは動かす場所がなくなるため置けません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}

							if (board0[tate_input][yoko_input] > 0) {
								tegoma[0] = koma_data[board0[tate_input][yoko_input]-1][0];
								tegoma[1] = koma_data[board0[tate_input][yoko_input]-1][1];
								tegoma[2] = koma_data[board0[tate_input][yoko_input]-1][2];
								tegoma[3] = koma_data[board0[tate_input][yoko_input]-1][3];
								tegoma[4] = board0[tate_input][yoko_input] - 1;
							}
							board0[ tate_past[1] ][ yoko_past[1] ] = 0;
							next_board(tate_input, yoko_input, 0, i, order);
							store_kiki();
							remove_ou_kiki();

							if (judge_kiki_ou(order, 1) != 0) {
								printf("自分の玉が取られます！！玉が取られないように入力しなおし！！\n");
								back_board(yoko_past[1], tate_past[1], yoko_input, tate_input, 0, i, tegoma, nari, 1);
								store_kiki();
								remove_ou_kiki();
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;

								continue;
							}
							oute = judge_kiki_ou(order, 2);
							if (oute != 0) printf("相手に王手をかけました！！\n");
							tsumi = judge_tsumi(order, oute);
							if (tsumi == 1) break;

							state_past = state;
							state = FIRST_INPUT;

							if (order == SENTE) order = GOTE;
							else		    order = SENTE;

						}
						else if (koma == koma_shurui[ koma_past[1] -1 ][ 1 ]) { //成る場合
							i = board0[ tate_past[1] ][ yoko_past[1] ] - 1;
							if (judge_koma_move(tate_input, yoko_input, i) == 0 ) { //動けるかどうか判定
								printf("この駒はそのマスへ動かすことはできません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}

							nari = judge_nari(tate_past[1], tate_input, i, order);
							if (nari == 0) {
								printf("成れません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}
							else if (nari == 2) {
								printf("成っている駒を再び戻すことはできません！！\n");
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;
							}

							if (board0[tate_input][yoko_input] > 0) {
								tegoma[0] = koma_data[board0[tate_input][yoko_input]-1][0];
								tegoma[1] = koma_data[board0[tate_input][yoko_input]-1][1];
								tegoma[2] = koma_data[board0[tate_input][yoko_input]-1][2];
								tegoma[3] = koma_data[board0[tate_input][yoko_input]-1][3];
								tegoma[4] = board0[tate_input][yoko_input] - 1;
							}
							board0[ tate_past[1] ][ yoko_past[1] ] = 0;
							next_board(tate_input, yoko_input, 0, i, order);
							store_kiki();
							remove_ou_kiki();

							if (judge_kiki_ou(order, 1) != 0) {
								printf("自分の玉が取られます！！玉が取られないように入力しなおし！！\n");
								back_board(yoko_past[1], tate_past[1], yoko_input, tate_input, 0, i, tegoma, nari, 1);
								store_kiki();
								remove_ou_kiki();
								buzzer( 1 );
								trans( 1, 1,  0);
								//元に戻す
								if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
								else								    break;

								continue;
							}
							oute = judge_kiki_ou(order, 2);
							if (oute != 0) printf("相手に王手をかけました！！\n");
							tsumi = judge_tsumi(order, oute);
							if (tsumi == 1) break;

							state_past = state;
							state = FIRST_INPUT;

							if (order == SENTE) order = GOTE;
							else		    order = SENTE;

						}
						else if (koma == koma_past[0]) { //動かす自駒と取る駒が同じまたは成っている駒で、相手の駒を持ち上げて戻す行為は判別不可
							state_past = FIRST_INPUT;
							state = MOVE_INPUT;
							tate_input = tate_past[1];
							yoko_input = yoko_past[1];
							koma = koma_past[1];
							trans( 1, 1,  0);
						}
						else {
							break;//訳分からん　余地無し
						}

					}
					else if (tate_input == tate_past[1] && yoko_input == yoko_past[1]) {
						state_past = FIRST_INPUT;
						state = GET_KOMA;
						tate_input = tate_past[0];
						yoko_input = yoko_past[0];
						koma = koma_past[0];

						hyouji();
						trans( 3, 2,  0);
					}
					else {
						break;//訳分からん　余地無し
					}
				}
				else {
					break;//訳分からん　余地無し
				}
			}

		}

		if (state == USE_MOCHIGOMA) {
			mochigoma_i = 0;
			for (i = 0; i < 40; i++) {
				if (koma_data[i][3] == MOCHIGOMA) {
					if (koma_data[i][2] == order) {
						if (koma == koma_data[i][4]) {
							mochigoma_i = i;
							break;
						}
					}
				}	
			}
			if (mochigoma_i == 0) { //持っていない持ち駒を使う場合
				printf("そんな持ち駒はありません。\n");
				break;//訳分からん
			}

			if (board0[tate_input][yoko_input] > 0) { //持ち駒を置いた場所にはすでに駒がある場合
				printf("すでに駒が置いてあります。\n");
				break;//訳分からん
			}

			if(koma == FU) { //二歩の判定
				nifu = 0;
				for (j = 1; j <= D; j++) {
					if(j == tate_input) {
						continue;
					} else if (koma_data[board0[j][yoko_input]-1][4] == FU) {
						if (koma_data[board0[j][yoko_input]-1][2] == order && koma_data[board0[j][yoko_input]-1][3] == OMOTE) {
							nifu = 1;
							break;
						}
					}
				}
				if (nifu == 1) {
					printf("二歩です！！\n");
					buzzer( 1 );
					trans( 1, 1,  0);	
					//元に戻す
					if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
					else								    break;
				}
			}
			
			if (judge_1danme(tate_input, mochigoma_i, order) == 0) { //1段目の判定
				printf("そこは動かす場所がなくなるため置けません！！\n");
				buzzer( 1 );
				trans( 1, 1,  0);
				//元に戻す
				if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
				else								    break;
			}

			next_board(tate_input, yoko_input, 1, mochigoma_i, order);
			store_kiki();
			remove_ou_kiki();

			if (judge_kiki_ou(order, 1) != 0) {
				printf("自分の玉が取られます！！玉が取られないように入力しなおし！！\n");
				back_board(0, 0, yoko_input, tate_input, 1, mochigoma_i, tegoma, 0, 0);
				store_kiki();
				remove_ou_kiki();
				buzzer( 1 );
				trans( 1, 1,  0);
				//元に戻す
				if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
				else								    break;

				continue;
			}
			oute = judge_kiki_ou(order, 2);
			if (oute != 0) printf("相手に王手をかけました！！\n");
			tsumi = judge_tsumi(order, oute);
			if (tsumi == 1 && koma == FU) { //打ち歩詰めの判定
				printf("打ち歩詰めです！！１手戻してください。\n");
				back_board(0, 0, yoko_input, tate_input, 1, mochigoma_i, tegoma, 0, 0);
				store_kiki();
				remove_ou_kiki();
				buzzer( 1 );
				trans( 1, 1,  0);
				//元に戻す
				if (SUCCESS == back_one_hand(tate_input, yoko_input, change, koma)) continue;
				else								    break;

				continue;
			}
			else if (tsumi == 1){
				break;
			}

			state_past = state;
			state = FIRST_INPUT;

			if (order == SENTE) order = GOTE;
			else		    order = SENTE;

			trans( 1, 1,  0);
		}

		//過去の入力を記憶
		tate_past[1] = tate_past[0];
		yoko_past[1] = yoko_past[0];
		koma_past[1] = koma_past[0];
		tate_past[0] = tate_input;
		yoko_past[0] = yoko_input;
		koma_past[0] = koma;

	}

	if (tsumi == 1) {
		hyouji();
		trans( 3, 2,  0);
		if (order == SENTE) printf("後手の玉が詰みました！！先手の勝ちです！！\n");
		else		    printf("先手の玉が詰みました！！後手の勝ちです！！\n");
	}
	else {
		trans( 1, 2,  0);
		buzzer( 2 );
		printf("予期しない入力のため、強制終了します！！\n");
	}

	return 0;

}

// 一手戻るための処理
short int back_one_hand (short int tate_input, short int yoko_input, short int change, short int koma) {
	short int tate_back, yoko_back, koma_back, change_back; //エラーを起こして、元に戻すときにH8から得るデータを格納する変数
	while(1) {
//		input_data(&tate_back, &yoko_back, &change_back, &koma_back);
		serial(&tate_input, &yoko_input, &change, &koma);
		if (tate_input == tate_back && yoko_input == yoko_back && change != change_back && koma == koma_back) {
			return SUCCESS;
		}
		else {
			return ERROR;
		}
	}
}

void next_board(short int c, short int d, short int x, short int i, short int order){
	if (x == 0) {
		if (board0[c][d] > 0){
			koma_data[board0[c][d]-1][0] =  0;
			koma_data[board0[c][d]-1][1] =  0;
			koma_data[board0[c][d]-1][2] =  order;
			koma_data[board0[c][d]-1][3] =  MOCHIGOMA;
		}
		board0[c][d] = i + 1;
		koma_data[i][0] = c;
		koma_data[i][1] = d;
	} else {
		board0[c][d] = i + 1;
		koma_data[i][0] = c;
		koma_data[i][1] = d;
		koma_data[i][3] = OMOTE;	
	}
}

void back_board(short a, short b, short c, short d, short x, short i, short *tegoma, short nari, short komatori){
	if (x == 0) {
		if (komatori == 1){
			koma_data[tegoma[4]][0] =  tegoma[0];
			koma_data[tegoma[4]][1] =  tegoma[1];
			koma_data[tegoma[4]][2] =  tegoma[2];
			koma_data[tegoma[4]][3] =  tegoma[3];
			board0[d][c] = tegoma[4] + 1;
		} else {
			board0[d][c] = 0;
		}
		if (nari == 1) koma_data[i][3] = OMOTE;
		board0[b][a] = i + 1;
		koma_data[i][0] = b;
		koma_data[i][1] = a;

	} else {
		board0[d][c] = 0;
		koma_data[i][0] = 0;
		koma_data[i][1] = 0;
		koma_data[i][3] = MOCHIGOMA;	
	}
}

short int judge_nari(short int b, short int d, short int i, short int order){ //戻り値 0:成れない 1:成り可能 2:成った駒が再び裏返った
	short int x = 0;

	if (koma_data[i][3] != URA) {
		if (koma_data[i][4] != OU && koma_data[i][4] != KI) {
			if(order == SENTE) {
				if (b <= JINCHI_GOTE || d <= JINCHI_GOTE) {
					x = 1;
				}
			} else {
				if (b >= JINCHI_SENTE || d >= JINCHI_SENTE) {
					x = 1;
				}
			}
		}
	}
	else {
		return 2;
	}
	if (x == 1) {
		koma_data[i][3] = URA;
		return 1;
	} else {
		return 0;
	}
}

short int judge_1danme(short int d, short int i, short int order) {
	if (koma_data[i][4] == FU || koma_data[i][4] == KY || koma_data[i][4] == KE) {
		if (koma_data[i][3] == OMOTE) {
			if (order == SENTE) {
				if (d == 1) return 0;
				else if (d == 2 && koma_data[i][4] == KE) return 0;
			} else {
				if (d == D) return 0;
				else if (d == D-1 && koma_data[i][4] == KE) return 0;
			}
		}
	}
	return 1;
}

void store_kiki(void){
	short int a, b, i, j, k, x;
	for (i = 0; i < 40; i++) {
		for (j = 5; j < 45; j++){
			koma_data[i][j] = 0;
		}
		if (koma_data[i][3] == OMOTE) {
			for (j=5, x=0; j < 45; j+=2, x+=3){
				if (kiki_data[koma_data[i][4]-1][x+2] == 0 || x >= 24) break;
				else if (kiki_data[koma_data[i][4]-1][x+2] == 2) {
					for (k=1;;k++, j+=2) {
						a = (koma_data[i][2] == SENTE) ? (koma_data[i][0] + (kiki_data[koma_data[i][4]-1][x]*k)) : (koma_data[i][0] - (kiki_data[koma_data[i][4]-1][x]*k));
						b = (koma_data[i][2] == SENTE) ? (koma_data[i][1] + (kiki_data[koma_data[i][4]-1][x+1]*k)) : (koma_data[i][1] - (kiki_data[koma_data[i][4]-1][x+1]*k));
						if(a == 0 || a == D+1 || b == 0 || b == D+1) {
							j -= 2;
							break;
						} else if (board0[a][b] > 0) {
							if (koma_data[board0[a][b]-1][2] == koma_data[i][2]) {
								j -= 2;
								break;
							} else {
								koma_data[i][j]   = a;
								koma_data[i][j+1] = b;
								break;
							}
						}
						koma_data[i][j]   = a;
						koma_data[i][j+1] = b;
					}
				} else {
					a = (koma_data[i][2] == SENTE) ? (koma_data[i][0] + kiki_data[koma_data[i][4]-1][x]) : (koma_data[i][0] - kiki_data[koma_data[i][4]-1][x]);
					b = (koma_data[i][2] == SENTE) ? (koma_data[i][1] + kiki_data[koma_data[i][4]-1][x+1]) : (koma_data[i][1] - kiki_data[koma_data[i][4]-1][x+1]);
					if(a == 0 || a == D+1 || b == 0 || b == D+1) {
						j -= 2;
						continue;
					} else if (board0[a][b] > 0) {
						if (koma_data[board0[a][b]-1][2] == koma_data[i][2]) {
							j -= 2;
							continue;
						}
					}
					koma_data[i][j]   = a;
					koma_data[i][j+1] = b;
				}
			}
		} else if (koma_data[i][3] == URA) {
			for (j=5, x=0; j < 45; j+=2, x+=3){
				if (kiki_data[*(*(koma_shurui+*(*(koma_shurui+koma_data[i][4]-1)+1)-1)+2)][x+2] == 0 || x >= 24) break;
				else if (kiki_data[*(*(koma_shurui+*(*(koma_shurui+koma_data[i][4]-1)+1)-1)+2)][x+2] == 2) {
					for (k=1;;k++, j+=2) {
						a = (koma_data[i][2] == SENTE) ? (koma_data[i][0]+(kiki_data[*(*(koma_shurui+*(*(koma_shurui+koma_data[i][4]-1)+1)-1)+2)][x]*k)) : (koma_data[i][0]-(kiki_data[*(*(koma_shurui+*(*(koma_shurui+koma_data[i][4]-1)+1)-1)+2)][x]*k));
						b = (koma_data[i][2] == SENTE) ? (koma_data[i][1]+(kiki_data[*(*(koma_shurui+*(*(koma_shurui+koma_data[i][4]-1)+1)-1)+2)][x+1]*k)) : (koma_data[i][1]-(kiki_data[*(*(koma_shurui+*(*(koma_shurui+koma_data[i][4]-1)+1)-1)+2)][x+1]*k));
						if(a == 0 || a == D+1 || b == 0 || b == D+1) {
							j -= 2;
							break;
						} else if (board0[a][b] > 0) {
							if (koma_data[board0[a][b]-1][2] == koma_data[i][2]) {
								j -= 2;
								break;
							} else {
								koma_data[i][j]   = a;
								koma_data[i][j+1] = b;
								break;
							}
						}
						koma_data[i][j]   = a;
						koma_data[i][j+1] = b;
					}
				} else {
					a = (koma_data[i][2] == SENTE) ? (koma_data[i][0] + kiki_data[*(*(koma_shurui+*(*(koma_shurui+koma_data[i][4]-1)+1)-1)+2)][x]) : (koma_data[i][0]-kiki_data[*(*(koma_shurui+*(*(koma_shurui+koma_data[i][4]-1)+1)-1)+2)][x]);
					b = (koma_data[i][2] == SENTE) ? (koma_data[i][1] + kiki_data[*(*(koma_shurui+*(*(koma_shurui+koma_data[i][4]-1)+1)-1)+2)][x+1]) : (koma_data[i][1]-kiki_data[*(*(koma_shurui+*(*(koma_shurui+koma_data[i][4]-1)+1)-1)+2)][x+1]);
					if(a == 0 || a == D+1 || b == 0 || b == D+1) {
						j -= 2;
						continue;
					} else if (board0[a][b] > 0) {
						if (koma_data[board0[a][b]-1][2] == koma_data[i][2]) {
							j -= 2;
							continue;
						}
					}
					koma_data[i][j]   = a;
					koma_data[i][j+1] = b;
				}
			}
		}
	}
}

short int judge_kiki_ou(short int order, short int x) { //自分の王への利きを調べる:x=1 相手の王への利きを調べる：x=2
	short int i, j, n, m, l;
	m = 0;
	l = 0;
	if (koma_data[0][2] == order)      (x == 1) ? (n = 0) : (n = 1);
	else if (koma_data[1][2] == order) (x == 1) ? (n = 1) : (n = 0);
	 
	for (i = 0; i < 40; i++) {
		if (koma_data[i][2] == MOCHIGOMA) continue;
		if (koma_data[i][2] == koma_data[n][2])     continue;
		else {
			for(j = 5; j < 45; j += 2) {
				if(koma_data[i][j] == 0) break;
				if (koma_data[n][0] == koma_data[i][j] && koma_data[n][1] == koma_data[i][j+1]) {
					m = i;
					l++;
					if (l == 2) return -1;
				}
			}
		}
	} 
	return m;
}

short int judge_koma_move(short int c, short int d, short int i) {
	short int n;
	for (n = 5; n < 45; n += 2) {
		if (koma_data[i][n] == 0) break;
		if (c == koma_data[i][n] && d == koma_data[i][n+1]) return 1;
	}
	return 0;
}

void remove_ou_kiki(void) {
	short int i, j, k, n, m, x, komatori;
	short int tegoma[5] = { };
	short int koma[40][45] = { };
	for (n = 0; n <= 1; n++) {
		for(m = 5; m < 45; m += 2) {
			if (koma_data[n][m] == 0) break;
			for (i = 0; i < 40; i++) {
				if (koma_data[i][3] == MOCHIGOMA) continue;
				if (koma_data[i][2] == koma_data[n][2]) continue;
				for (j = 5; j < 45; j += 2) {
					if (koma_data[i][j] == 0) break;
					if (koma_data[i][j] == koma_data[n][m] && koma_data[i][j+1] == koma_data[n][m+1]) {
						x = 1;
						break;
					}
				}
				if (x == 1) break;
			}
			if (x == 1) {
				for (k = m; k+2 < 45; k += 2) {
					koma_data[n][k]   = koma_data[n][k+2];
					koma_data[n][k+1] = koma_data[n][k+3];
					koma_data[n][k+2] = 0;
					koma_data[n][k+3] = 0;
					if (koma_data[n][k] == 0) break;
				}
				m -= 2;
				x = 0;
			}
		}
	}

	for (i = 0; i < 40; i++) {
		for (j = 0; j < 45; j++) {
			if (j > 5 && koma_data[i][j] == 0) break;
			koma[i][j] = koma_data[i][j];
		}
	}
	for (n = 0; n <= 1; n++) {
		for (m = 5; m < 45; m += 2) {
			komatori = 0;
			if (koma[n][m] == 0) break;
			board0[koma[n][0]][koma[n][1]] = 0;
			if (board0[koma[n][m]][koma[n][m+1]] > 0) {
					komatori = 1;
					tegoma[0] = koma_data[board0[koma[n][m]][koma[n][m+1]]-1][0];
					tegoma[1] = koma_data[board0[koma[n][m]][koma[n][m+1]]-1][1];
					tegoma[2] = koma_data[board0[koma[n][m]][koma[n][m+1]]-1][2];
					tegoma[3] = koma_data[board0[koma[n][m]][koma[n][m+1]]-1][3];
					tegoma[4] = board0[koma[n][m]][koma[n][m+1]] - 1;
			}
			next_board(koma[n][m], koma[n][m+1], 0, n, koma[n][2]);
			store_kiki();
			if (judge_kiki_ou(koma_data[n][2], 1) != 0) {
				back_board(koma[n][1], koma[n][0], koma[n][m+1], koma[n][m], 0, n, tegoma, 0, komatori);
				for (k = m; k+2 < 45; k += 2) {
					koma[n][k]   = koma[n][k+2];
					koma[n][k+1] = koma[n][k+3];
					koma[n][k+2] = 0;
					koma[n][k+3] = 0;
					if (koma_data[n][k] == 0) break;
				}
				m -= 2;
			} else {
				back_board(koma[n][1], koma[n][0], koma[n][m+1], koma[n][m], 0, n, tegoma, 0, komatori);
			}
		
		}

	}
	for (i = 0; i < 40; i++) {
		for (j = 0; j < 45; j++) {
			koma_data[i][j] = koma[i][j];
		}
	}
}

short int judge_tsumi(short int order, short int oute) { //戻り値が１->詰み　0->詰みじゃない
	short int i, j, k, l, n, tate, yoko, nifu, nifu_i;
	if (oute == 0) return 0;

	if (order == SENTE) n = 1;
	else 		    n = 0;

	if (koma_data[n][5] != 0) return 0;
	if (oute == -1) return 1;
	for (i = 0; i < 40; i++) {
		if (koma_data[i][2] == MOCHIGOMA) continue;
		if (koma_data[i][2] == koma_data[oute][2]) continue;
		else {
			for(j = 5; j < 45; j += 2) {
				if(koma_data[i][j] == 0) break;
				if (koma_data[oute][0] == koma_data[i][j] && koma_data[oute][1] == koma_data[i][j+1]) {
					return 0;
				}
			}
		}
	}
	tate = koma_data[oute][0]-koma_data[n][0];
	yoko = koma_data[oute][1]-koma_data[n][1];
	if (tate >= 2 || tate <= -2 || yoko >= 2 || yoko <= -2) { //飛・角・香などが王手をして、間がある場合
		if (koma_data[oute][4] == KE && koma_data[oute][3] == OMOTE) return 1;
		for (i = 2; i < 40; i++) {
			if (koma_data[i][3] == MOCHIGOMA) { //持ち駒がある場合
				if (koma_data[i][2] != koma_data[n][2]) continue;
				if (koma_data[i][4] != FU) { //歩以外の持ち駒があれば詰みではない
					return 0;
				}
				else { //歩だけ持ち駒として持っている場合					
					if (yoko == 0) {
						nifu = 0;
						for (j = 1; j < 10; j++) {
							nifu_i = board0[ j ][ koma_data[n][1] ] - 1;
							if (koma_data[nifu_i][4] == FU) {
								if (koma_data[nifu_i][2] == koma_data[n][2] && koma_data[nifu_i][3] == OMOTE) {
									nifu = 1;
									break;
								}
							}
						}
						if (nifu == 0) return 0; //二歩にならないで、持ち駒の歩を使用できるため、詰みではない
					} else {
						for ((yoko<0) ? (k=yoko+1) : (k=yoko-1); k != 0; (k<0) ? (k++) : (k--) ) {
							nifu = 0;
							for (j = 1; j < 10; j++) {
								nifu_i = board0[ j ][ koma_data[n][1] + k ] - 1;
								if (koma_data[nifu_i][4] == FU) {
									if (koma_data[nifu_i][2] == koma_data[n][2] && koma_data[nifu_i][3] == OMOTE) {
										nifu = 1;
										break;
									}
								}
							}
							if (nifu == 0) return 0; //二歩にならない列があるため、持ち駒の歩を使用できるため、詰みではない
						}
					} 
				}
			}
		}

		short int t, y, tate_past, yoko_past, *tegoma;
		if (yoko == 0) { //飛・香のような縦に利きがある駒の場合
			y =  koma_data[n][1];
			for ((tate<0) ? (k=tate+1) : (k=tate-1); k != 0; (k<0) ? (k++) : (k--) ) {
				t = koma_data[n][0] + k;
				for (i = 2; i < 40; i++) {
					if (koma_data[i][2] == MOCHIGOMA) continue;
					if (koma_data[i][2] == koma_data[oute][2]) continue;
					else {
						for(j = 5; j < 45; j += 2) {
							if(koma_data[i][j] == 0) break;
							if (t == koma_data[i][j] && y == koma_data[i][j+1]) {
								board0[koma_data[i][0]][koma_data[i][1]] = 0;
								tate_past = koma_data[i][0];
								yoko_past = koma_data[i][1];
								next_board(t, y, 0, i, koma_data[i][2]); //y,t注意
								store_kiki();
								if (judge_kiki_ou(koma_data[i][2], 1) == 0) {
									back_board(yoko_past, tate_past, y, t, 0, i, tegoma, 0, 0);
									store_kiki();
									remove_ou_kiki();
									return 0;
								}
								else {
									back_board(yoko_past, tate_past, y, t, 0, i, tegoma, 0, 0);
									store_kiki();
									//remove_ou_kiki();
									break;
								}
							}
						}
					}
				}
			}
		}
		else if (tate == 0) { //飛のように横に利きがある駒の場合
			t =  koma_data[n][0];
			for ((yoko<0) ? (k=yoko+1) : (k=yoko-1); k != 0; (k<0) ? (k++) : (k--) ) {
				y = koma_data[n][1] + k;
				for (i = 2; i < 40; i++) {
					if (koma_data[i][2] == MOCHIGOMA) continue;
					if (koma_data[i][2] == koma_data[oute][2]) continue;
					else {
						for(j = 5; j < 45; j += 2) {
							if(koma_data[i][j] == 0) break;
							if (t == koma_data[i][j] && y == koma_data[i][j+1]) {
								board0[koma_data[i][0]][koma_data[i][1]] = 0;
								tate_past = koma_data[i][0];
								yoko_past = koma_data[i][1];
								next_board(t, y, 0, i, koma_data[i][2]); //y,t注意
								store_kiki();
								if (judge_kiki_ou(koma_data[i][2], 1) == 0) {
									back_board(yoko_past, tate_past, y, t, 0, i, tegoma, 0, 0);
									store_kiki();
									remove_ou_kiki();
									return 0;
								}
								else {
									back_board(yoko_past, tate_past, y, t, 0, i, tegoma, 0, 0);
									store_kiki();
									//remove_ou_kiki();
									break;
								}
							}
						}
					}
				}
			}
		}
		else { //角のように斜めに利きがある駒の場合
			for ((tate<0) ? (l=tate+1) : (l=tate-1); l != 0; (l<0) ? (l++) : (l--) ) {
				t =  koma_data[n][0] + l;
				for ((yoko<0) ? (k=yoko+1) : (k=yoko-1); k != 0; (k<0) ? (k++) : (k--) ) {
					y = koma_data[n][1] + k;
					for (i = 2; i < 40; i++) {
						if (koma_data[i][2] == MOCHIGOMA) continue;
						if (koma_data[i][2] == koma_data[oute][2]) continue;
						else {
							for(j = 5; j < 45; j += 2) {
								if(koma_data[i][j] == 0) break;
								if (t == koma_data[i][j] && y == koma_data[i][j+1]) {
									board0[koma_data[i][0]][koma_data[i][1]] = 0;
									tate_past = koma_data[i][0];
									yoko_past = koma_data[i][1];
									next_board(t, y, 0, i, koma_data[i][2]); //y,t注意
									store_kiki();
									if (judge_kiki_ou(koma_data[i][2], 1) == 0) {
										back_board(yoko_past, tate_past, y, t, 0, i, tegoma, 0, 0);
										store_kiki();
										remove_ou_kiki();
										return 0;
									}
									else {
										back_board(yoko_past, tate_past, y, t, 0, i, tegoma, 0, 0);
										store_kiki();
										//remove_ou_kiki();
										break;
									}
								}
							}
						}
					}
				}
			}
		}
		remove_ou_kiki();
	}
	
	return 1;
}


