#include <stdio.h>
#define BOARDSIZE 15
#define MOVESIZE 36
#define HEIGHT 13
/*
Written by David S. Scott
Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*

This is a serial (recursive) implementation of the triangle puzzle.
See the README file in triangle/refactored/ocr/intel
*/

void makemove(int board[BOARDSIZE], int pmoves[MOVESIZE][3], int * count, int  nummoves ){
    int i;
    if(nummoves == HEIGHT){
        (*count)++;
        return;
    } else {
        for(i=0;i<MOVESIZE;i++) {
            if(board[pmoves[i][0]] && board[pmoves[i][1]] && !board[pmoves[i][2]]) {
//make move
                board[pmoves[i][0]] = 0;
                board[pmoves[i][1]] = 0;
                board[pmoves[i][2]] = 1;
//recurse
		makemove(board, pmoves, count, nummoves+1);
//unmove
                board[pmoves[i][0]] = 1;
                board[pmoves[i][1]] = 1;
                board[pmoves[i][2]] = 0;
            }
        }
    }
}
main()
{
int nummoves;
int count = 0;
int pmoves[MOVESIZE][3] ={
{0,1,3},
{3,1,0},
{0,2,5},
{5,2,0},
{3,4,5},
{5,4,3},
{1,3,6},
{6,3,1},
{1,4,8},
{8,4,1},
{2,4,7},
{7,4,2},
{2,5,9},
{9,5,2},
{6,7,8},
{8,7,6},
{7,8,9},
{9,8,7},
{3,6,10},
{10,6,3},
{3,7,12},
{12,7,3},
{4,7,11},
{11,7,4},
{4,8,13},
{13,8,4},
{5,8,12},
{12,8,5},
{5,9,14},
{14,9,5},
{10,11,12},
{12,11,10},
{11,12,13},
{13,12,11},
{12,13,14},
{14,13,12}
};
int board[BOARDSIZE] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int i, j;
	makemove(board, pmoves, &count, nummoves);
printf("number of solutions %d \n", count);

}
