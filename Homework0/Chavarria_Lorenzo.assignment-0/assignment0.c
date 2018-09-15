#include <stdio.h>

int ind = 0;
int visited[25];


//checks if the next move has been visited
int ifVisited(int number, int visited[25]){
  int i = 0;
  for(i = 0; i < 25; i++){
    if(number == visited[i])
      {
	return 1;
      }
  }
  return 0;
}

//prints out the visited tour
void printVisited(int visited[25]){
  int i;
  for(i=0;i<25;i++){
    printf("%d ", visited[i]);
  }
    printf("\n");
}

//checks if the visited array is full, meaning that there is a complete tour
int isFull( int visited[25]){
 int i = 0;
  for(i = 0; i < 25; i++){
    if(visited[i] == 0)
      {
	return 1;
      }
  }
  return 0;
}

//does the knights tour
void tour(int tile, int row, int col, int board[5][5]){

  //checks to see if the visited array is full
  if(isFull(visited) == 0){
    	  printVisited(visited);
	  return;
  }
  else{
  //checks to see if there are any valid moves
  if(row-2 >= 0 && col-1 >= 0)
    {
      if(ind <  25 && ifVisited(board[row-2][col-1],visited) == 0){
	visited[ind] = board[row-2][col-1];
	ind++;
	tour(board[row-2][col-1], row-2, col-1, board);
	ind--;
	visited[ind] = 0;
      }
    }

  if(row-1 >= 0 && col-2 >= 0)
    {
      if(ind <  25 && ifVisited(board[row-1][col-2],visited) == 0){
	visited[ind] = board[row-1][col-2];
	ind++;
	tour(board[row-1][col-2], row-1, col-2, board);
	ind--;
	visited[ind] = 0;
      }
    }

   if(row+1 < 5 && col-2 >= 0)
    {
      if(ind <  25 && ifVisited(board[row+1][col-2],visited) == 0){
	visited[ind] = board[row+1][col-2];
	ind++;
	tour(board[row+1][col-2], row+1, col-2, board);
	ind--;
	visited[ind] = 0;
      }
    }

   if(row+2 < 5  && col-1 >= 0)
    {
     if(ind <  25 && ifVisited(board[row+2][col-1],visited) == 0){
       visited[ind] = board[row+2][col-1];
       ind++;
	tour(board[row+2][col-1], row+2, col-1, board);
	ind--;
	visited[ind] = 0;
      }
    }

   if(row+2 < 5 && col+1 < 5)
    {
     if(ind <  25 && ifVisited(board[row+2][col+1],visited) == 0){
       visited[ind] = board[row+2][col+1];
       ind++;
	tour(board[row+2][col+1], row+2, col+1, board);
	ind--;
	visited[ind] = 0;
      }
    }

   if(row+1 < 5 && col+2 < 5)
    {
     if(ind <  25 && ifVisited(board[row+1][col+2],visited) == 0){
       visited[ind] = board[row+1][col+2];
       ind++;
	tour(board[row+1][col+2], row+1, col+2, board);
	ind--;
	visited[ind] = 0;
      }
    }

   if(row-1 >= 0 && col+2 < 5)
    {
     if(ind <  25 && ifVisited(board[row-1][col+2],visited) == 0){
       visited[ind] = board[row-1][col+2];
       ind++;
	tour(board[row-1][col+2], row-1, col+2, board);
	ind--;
	visited[ind] = 0;
      }
    }

   if(row-2 >= 0 && col+1  < 5) {
      if(ind <  25 && ifVisited(board[row-2][col+1],visited) == 0){
	visited[ind] = board[row-2][col+1];
	ind++;
	tour(board[row-2][col+1], row-2, col+1, board);
	ind--;
	visited[ind] = 0;
    }
   }
  }
 
  if(isFull(visited) == 1){
    visited[ind-1] = 0;
  }

}

//makes a board using a 2d array
void make_board(int board[5][5]){
  int i,j;
  int incr = 1;
  for(j = 0; j<5; j++){
    for(i = 0;i<5;i++){
      board[j][i]= incr;
      incr++;
    }
  }
}


int main(int argc, char *argv[]){
  int board[5][5];

  make_board(board);
  
   int h,l;
   for(l = 0;l<5;l++){
    for(h = 0;h<5;h++){
      if(board[l][h]%2){  
	int v;
	for(v = 0;v<25;v++)
	  {
	    visited[v] = 0;
	  }
	visited[ind]=board[l][h];
	ind++;
	tour(board[l][h],l,h,board);
	ind = 0;
       }
      }
     }

  return 0; 
}
