/*
 * project5.c
 *
 * Created: 6/6/2019 11:23:29 PM
 * Author : jarro
 */ 

#define __DELAY_BACKWARD_COMPATIBLE__
#include <avr/io.h>
#include "avr.h"
#include "lcd.h"
#include "notes.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <util/delay.h>


int START = 20;
int ISSEEDED = 0;


double notefreq(int n, int oct){
	int p = n+12*oct - 58;	//A + 12*4 = 58
	return 440.0*pow(1.059463094359, p);
}




void play_note(double n, int oct, float time){
	double freq = notefreq(n, oct);
	if(n <= 0){
		avr_wait(time*1000);
		return;
	}
	
	double p = 1.0/freq;
	double r = p/2;
	
	int k;
	
	for(int i=0; i<freq*time; i++){
		SET_BIT(PORTB, 3);
		_delay_ms(r*7800);
		CLR_BIT(PORTB, 3);
		_delay_ms(r*7800);
	}
	_delay_ms(TEMPO*time*1.5);
}


int is_pressed(int r, int c){
	DDRC = 0;
	PORTC = 0;
	SET_BIT(PORTC, r);
	SET_BIT(DDRC, c+4);
	CLR_BIT(PORTC, c+4);
	if(GET_BIT(PINC, r)){
		return 0;
	}
	else{
		return 1;
	}
}

int get_key(){
	int r, c;
	for(r=0; r<4; r++){
		for(c=0; c<4; c++){
			if(is_pressed(r, c)){
				return (r*4+c)+1;
			}
		}
	}
	return -1;
}


int convert_key(int k){
	if(k<=4) return k;
	if(k==16) return START;
	if(k<=7 && k>=5) return k-1;
	if(k<=11 && k>=9) return k-2;
	return -1;
}


void turn_on(int light){
	SET_BIT(PORTA, light);
}

void turn_off(int light){
	CLR_BIT(PORTA, light);
}

void all_off(){
	turn_off(0);
	turn_off(1);
	turn_off(2);
	turn_off(3);
}

int randint(int low, int high){
	return (rand() % (high - low + 1)) + low;
}


struct game{
	int lights[100];
	int len;
	int difficulty;
	int score;
	};

void init_game(struct game *g){
	for(int i=0; i<100; i++){
		g->lights[i] = 0;
	}
	g->len = 0;
	g->difficulty = 1;
}


void gen_light(struct game *g){
	g->lights[g->len] = randint(0,3);
	g->len++;
}


void set_difficulty(struct game *g){
	lcd_clr();
	int t= 0;
	int blink = 0;
	while(1){
		int key = convert_key(get_key());
		if(key>0 && key!=START){
			g->difficulty = key;
			if(ISSEEDED == 0){
				srand(t);
				ISSEEDED = 1;
			}
		}
		else if(key==START){
			if(ISSEEDED == 0){
				srand(t);
				ISSEEDED = 1;
			}
			return;
		}
		char s[16];
		if(t%1000 ==0) blink = 0;
		else if(t%500 == 0) blink = 1;
		if(blink)
			sprintf(s, "difficulty: %d", g->difficulty);
		else sprintf(s, "difficulty:");
		
		lcd_clr();
		lcd_puts2(s);
		t+=50;
		avr_wait(50);
	}
}

double notes[] = {C, F, G, A};

int get_key_timeout(){
	int t = 0;
	int down = 0;
	int pressed = 0;
	int k;
	while(t<2000){
		k = convert_key(get_key());
		if(k!=START && k>4) k=-1;
		
		if((k==1 || k==2 || k==3 || k==4) && pressed==0){
			turn_on(k-1);
			play_note(notes[k-1], 5, SN);
			t = 1500;
			pressed=1;
		}
		else if(k==START){
			return START;
		}
		else if(k!=down && down!=0){
			break;
		}
		
		down = k;
		t+=50;
		avr_wait(50);
	}
	all_off();
	if(k==down){
		avr_wait(500);
	}
	return down;
}




int recall(struct game *g){
	for(int i=0; i<g->len; i++){
		int k;
		do{
			k = get_key_timeout();
			if(k>0) break;
		}while(i==0);
		if(k==START){
			return -1;
		}
		if(k!=g->lights[i]+1){
			return 0;		
		}
	}
	return 1;
}


void gameover(struct game *g, int score){
	avr_wait(500);
	lcd_clr();
	char s1[16];
	char s2[16];
	sprintf(s1, "GAME OVER");
	sprintf(s2, "SCORE: %d", score);
	lcd_puts2(s1);
	lcd_pos(1,0);
	lcd_puts2(s2);
	
	play_note(C, 5, SN);
	play_note(F, 5, SN);
	avr_wait(150);
	play_note(F, 5, SN);
	play_note(F, 5, SN);
	play_note(E, 5, SN);
	play_note(D, 5, SN);
	play_note(C, 5, EN);
	
	lcd_clr();
	for(int i=0; i<5; i++){
		char s1[16];
		char s2[16];
		sprintf(s1, "GAME OVER");
		sprintf(s2, "SCORE: %d", score);
		lcd_puts2(s1);
		lcd_pos(1,0);
		lcd_puts2(s2);
		avr_wait(500);
		lcd_clr();
		avr_wait(500);
	}
}


void gen_next(struct game *g){
	int d = g->difficulty;
	for(int i=0; i<d; i++){		
		gen_light(g);
	}
}

struct note{
	int n;
	int o;
	int t;
	};


void next_round(int r){
	char s1[16];
	lcd_clr();
	sprintf(s1, "NEXT ROUND!");
	lcd_puts2(s1);
	avr_wait(500);
	play_note(G, 5, EN);
	avr_wait(20);
	play_note(G, 5, SN);
	avr_wait(50);
	play_note(G, 5, EN);
	avr_wait(50);
	play_note(E, 5, EN);
	avr_wait(50);
	play_note(C, 5, EN);
	avr_wait(500);
}

void disp_score(int score){
	lcd_clr();
	char s[16];
	sprintf(s, "round: %d", score);
	lcd_puts2(s);
}


void disp_start(){
	lcd_clr();
	lcd_puts2("3!");
	play_note(C, 4, EN+SN);
	avr_wait(100);
	lcd_clr();
	lcd_puts2("2!");
	play_note(C, 4, EN+SN);
	avr_wait(100);
	lcd_clr();
	lcd_puts2("1!");
	play_note(C, 4, EN+SN);
	avr_wait(100);
	lcd_clr();
	lcd_puts2("GO!");
	play_note(F, 4, EN+SN);
	avr_wait(500);
}

void blink_pattern(struct game *g){
	for(int i=0; i<g->len; i++){	
		turn_on(g->lights[i]);
		play_note(notes[g->lights[i]], 5, SN);
		avr_wait(200);
		turn_off(g->lights[i]);
		avr_wait(500);
	}
}

void run(struct game *g){
	disp_start();
	for(int i=1; ; i++){
		gen_next(g);
		disp_score(i);
		blink_pattern(g);
		int correct = recall(g);
		if(correct == -1){
			avr_wait(500);
			return;
		}
		if(correct==0){
			gameover(g, i);
			return;
		}
		next_round(i);
	}
}


void start_game(struct game *g){
	set_difficulty(g);
	run(g);
}


int main(void)
{
	struct game *g = malloc(sizeof(struct game));
	g->len = 0;
	g->difficulty = 1;
    lcd_init();
	SET_BIT(DDRA, 0);
	SET_BIT(DDRA, 1);
	SET_BIT(DDRA, 2);
	SET_BIT(DDRA, 3);
	
	SET_BIT(DDRB, 3);
	
    while (1) 
    {
		init_game(g);
		start_game(g);
		
    }
}

