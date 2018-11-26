# include <iostream>
# include <ncurses.h>
# include <unistd.h>

using namespace std;

int player = 0;
int gameState;
char playerSymbol;

//Functions
void initialize_Game(char board[9]);
void display_Game(char board[9]);
void display_Player(int player);
void handle_Input(char board[9]);
void change_Turn();

//void check_Gameover();
//void make_Move();

void change_Turn(){
  if (player == 0){
    player = 1;
    playerSymbol = 'O';
  }
  else{
    player = 0;
    playerSymbol = 'X';
  }
}


void handle_Input(char board[9]){
  int key;
  
  switch(key = getch()){
  case '1':
    if(board[0] == '1'){
      board[0] = playerSymbol;
      change_Turn();
    }
    break;
  case '2':
    if(board[1] == '2'){
      board[1] = playerSymbol;
      change_Turn();
    }
    break;
  case '3':
    if(board[2] == '3'){
      board[2] = playerSymbol;
      change_Turn();
    }
    break;
  case '4':
    if(board[3] == '4'){
      board[3] = playerSymbol;
      change_Turn();
    }
    break;
  case '5':
    if(board[4] == '5'){
      board[4] = playerSymbol;
      change_Turn();
    }
    break;
  case '6':
    if(board[5] == '6'){
      board[5] = playerSymbol;
      change_Turn();
    }
    break;
  case '7':
    if(board[6] == '7'){
      board[6] = playerSymbol;
      change_Turn();
    }
    break;
  case '8':
    if(board[7] == '8'){
      board[7] = playerSymbol;
      change_Turn();
    }
    break;
  case '9':
    if(board[8] == '9'){
      board[8] = playerSymbol;
      change_Turn();
    }
    break;
  case 'q':
  case 'Q':
    gameState = 1;
    break;
  }

}

void display_Player(int player){
  
  if(player == 0){
    mvprintw(7, 0, "Player 1 Turn : X");
  }
  else{
    mvprintw(7, 0, "Player 2 Turn: O");
  }

  refresh(); 
}


void initialize_Game(char board[9]){

  int i;
  int counter = 1;
  for(i = 0; i < 9; i++){
    board[i] = '0' + counter;
    counter++;
  }

}

void display_Game(char board[9]){
  mvprintw(0,0,"TIC TAC TOE");
  mvprintw(1,0,"|");
  mvprintw(1,2,"%c",board[0]);
  mvprintw(1,4,"|");
  mvprintw(1,6,"%c",board[1]);
  mvprintw(1,8,"|");
  mvprintw(1,10,"%c",board[2]);
  mvprintw(1,12,"|");
  mvprintw(2,0,"------------");
 
  mvprintw(3,0,"|");
  mvprintw(3,2,"%c",board[3]);
  mvprintw(3,4,"|");
  mvprintw(3,6,"%c",board[4]);
  mvprintw(3,8,"|");
  mvprintw(3,10,"%c",board[5]);
  mvprintw(3,12,"|");
  mvprintw(4,0,"------------");

  mvprintw(5,0,"|");
  mvprintw(5,2,"%c",board[6]);
  mvprintw(5,4,"|");
  mvprintw(5,6,"%c",board[7]);
  mvprintw(5,8,"|");
  mvprintw(5,10,"%c",board[8]);
  mvprintw(5,12,"|");

  refresh();
}

int main(){

  char gameboard[9];
  gameState = 0;
  playerSymbol = 'X';
  
  initialize_Game(gameboard);

 
  
  initscr();
  noecho();
  refresh();

  while(gameState != 1){
   
    display_Game(gameboard);
    display_Player(player);
    handle_Input(gameboard);

    clear();//possible bug   
  }
  endwin();
  
  return 0;
}
