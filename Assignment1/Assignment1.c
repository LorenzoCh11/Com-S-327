#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>




int rCorners[5][2]= {{0,0},{0,0},{0,0},{0,0},{0,0}};

/*Makes a struct that will represent each cell in a map that is 80 x 24
  oType:
       -h = horizontal border- 
       -v = vertical border
       -r = room
       -c = corridors
       -s = rocks
  oPic:
       -h = -
       -v = |
       -r = .
       -c = #
       -s = ' '
 */
struct square{
  char type;
  char pic;
  int x;
  int y;
  char mat;
  char immut;
};

//An array of square structs that represents the whole board for the game
struct square board[24][80];


void fillC(struct square board[24][80], int roomNum){
    int row;
    int col;
    int tempx = rCorners[0][0];
    int tempy = rCorners[0][1];
      int pos;
      int pos2;
      int incr;
      if((rCorners[roomNum][0]-tempx) !=  0){
	incr = (abs(rCorners[roomNum][0]-tempx))/(rCorners[roomNum][0]-tempx);
	for(pos = tempx; pos != rCorners[roomNum][0]; pos += incr){
	  if(board[pos][tempy].type == 's'){
	    board[pos][tempy].type = 'c';
	  }
	}
      }

      if((rCorners[roomNum][1]-tempy) != 0){
	incr = (abs(rCorners[roomNum][1]-tempy))/(rCorners[roomNum][1]-tempy);
	for(pos2 = tempy; pos2 != rCorners[roomNum][1]; pos2 += incr){
	  if(board[rCorners[roomNum][0]][pos2].type == 's'){
	    board[rCorners[roomNum][0]][pos2].type = 'c';
	  }
	}

    }



    for(row = 0; row<24; row++){
      for(col = 0; col<80; col++){
	if(board[row][col].type == 'c'){
	  board[row][col].pic = '#';
	}
      }
    }

}

void fillCorridors(struct square board[24][80]){
   fillC(board, 1);
   fillC(board, 2);
   fillC(board, 3);
   fillC(board, 4);
}


/*
checks to make sure that there is atleast 1 space between each squares of a room and
that each space in the whole room is valid
 */
bool checkSpace(int row, int col, int height, int width){
  int i;
  int j;
  for(i = 0; i < height + 1; i++){
    for(j = 0; j < width + 1; j++){
      if(board[row+i][col+j].type != 's'){
	return false;
      }
    }
  }

  return true;  

}

 

/*
Creates 5 Random Rooms that are atleast 1 cell apart  from each other and cannot be part
 of cells that are in the border
*/ 
void fillRooms(struct square array [24][80]){
  int rn = 0;
 

    while(rn < 5){ 

    srand(time(NULL));
    int r = (rand()% 14) + 2;
    int c = (rand() % 73) + 2;
     int rheight = (rand() % 8) + 3;
     int rwidth = (rand() % 11) + 4;
    int row;
    int col;

    if(checkSpace(r-1, c-1, rheight, rwidth) == true){
      int xs;
      int ys;
      rCorners[rn][0] = r;
      rCorners[rn][1] = c;
      rn++;
      for(xs = 0; xs < rheight-1; xs++){
	for(ys = 0; ys < rwidth-1; ys++){
	  board[r+xs][c+ys].type = 'r';
	}
      }
    }

    for(row = 0; row<24; row++){
      for(col = 0; col<80; col++){
	if(board[row][col].type == 'r'){
	  board[row][col].pic = '.';
	}
      }
    }


    }

}

/*
fills the variables of the cells that are going to be in the boarder of the board
 */
void fillBoarder(struct square array[24][80]){
  int i;
  int j;
  int row;
  int col;

  for(i = 0; i<80; i++){
    board[0][i].type = 'h';
    board[20][i].type = 'h';
  }

  for(j = 1; j<20; j++){
    board[j][0].type = 'v';
    board[j][79].type = 'v';
  }

  for(row = 0; row<24; row++){
    for(col = 0; col<80; col++){
      if(board[row][col].type == 'h'){
	board[row][col].pic = '-';
      }
      else if(board[row][col].type == 'v'){
	board[row][col].pic = '|';
      }
    else{
	board[row][col].pic = ' ';
	board[row][col].type = 's';
      }
    }
  }

}


//initializing all the methods for the Dungeon Game
int main(int argc, char * argv[]){
  int i;
  int j;

  fillBoarder(board);
  fillRooms(board);
  fillCorridors(board);

  for(i=0;i<24;i++){
    for(j=0;j<80;j++){
      printf("%c", board[i][j].pic);
    }
    printf("\n");
  }

  return 0;


}

