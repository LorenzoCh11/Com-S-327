# include <iostream>

# include <ncurses.h>
# include <unistd.h>

using namespace std;

//Functions
void initialize_Game(char board[9]);
void display_Game(char board[9]);
//void check_Gameover();
//void make_Move();

void initialize_Game(char board[9]){

  int i;
  for(i = 0; i < 9;i++){
    board[i] = ' ';
  }

}

void display_Game(char board[9]){

  cout << " TIC" << " TAC" << " TOE" << endl; 
  cout << " " << endl;
  cout << " " << board[0] << " | " << board[1] << " | " << board[2] << endl;
  cout << "------------" << endl;
  cout << " " << board[3] << " | " << board[4] << " | " << board[5] << endl;
  cout << "------------" << endl;
  cout << " " << board[6] << " | " << board[7] << " | " << board[8] << endl;
  cout << endl;
}

int main(){

  char gameboard[9];
  
  int key = 'a'; 
  initialize_Game(gameboard);

  display_Game(gameboard);
  
  initscr();
  noecho();
  while(key!='q'){
    key = getch();
    display_Game(gameboard);
  }
  endwin();
  
  return 0;


}
