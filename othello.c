/**
 * Othello game with A.I. employing alpha-beta principle
 * 
 * by Dan Breckenridge (CS50x Final Project 2016)
 * 
 * A.I. is optimized with one optimization parameter, which is the net number of white (or black) tiles
 * 
 * Currently, program is hard-coded so that the human player is always player black and the A.I. player is always player white
 * (according to the rules of Othello, player black always goes first).  Note that black tiles are represented with X's and white
 * tiles are represented with O's.
 * 
 * The DEPTH_MAX parameter can be increased to make the A.I. more difficult.  It is not recommended to use a depth max setting of 7
 * or higher, as that can lead to the A.I. taking upwards of 2 minutes to make a move
 * 
 * Consider the following improvements for future versions:
 * 
 * 1. Currently, when minimax function detects a leaf node, it may not actually be a leaf node.  It considers a given board to
 * be a leaf node if there are no moves available to the A.I. player, but it really should then check to see if there are moves
 * available for the human player afterwards.
 * 
 * 2. Consider increasing the score of (actual) leaf nodes by some amount if they are winning condition.  I.e., it may be better for the A.I.
 * to path towards an early, guaranteed win condition rather than whichever scenario provides the highest number of white tiles.  This
 * improvement cannot be implemented until the first improvement has been implemented.
 * 
 * 3. Somehow pass the address of the boardLocal variable at each node as a parameter to the subsequent node so that the boardAI variable
 * doesn't have to be a global variable that has to constantly be changed and then changed back.  May be able to reliably have A.I.
 * dig one level deeper if this is implemented.
 * 
 * 4. When freeing nodes from memory, consider only freeing those nodes each turn that are not part of the "winning" branch, instead
 * of freeing the entire tree.  That way, part of the already-loaded tree can be carried over to the A.I.'s next turn.
 */

#include <stdio.h>
#include <stdlib.h>
#include <cs50.h>
#include <ctype.h>

// define dimensions of square board size
#define BOARD_MAX 8

// define limit for depth of A.I. tree (higher depth means stronger A.I. opponent but longer processing time)
#define DEPTH_MAX 6

// declare and initialize board
int board[BOARD_MAX][BOARD_MAX];

// maintain copy of board for A.I.
int boardAI[BOARD_MAX][BOARD_MAX];

// variable for tracking current level on A.I. tree
int z;

// define struct node for A.I. tree
typedef struct node {
    int boardLocal[BOARD_MAX][BOARD_MAX];
    int score;
    int parentScore;
    bool isFirstChild;
    bool* firstChildAssigned;
    struct node* children[BOARD_MAX][BOARD_MAX];
}
node;

// define function prototypes
void printboard(void);
bool isLegal(int i, int j, int alignment, bool flip);
bool isAnyMoveAvailable(int alignment);
int boardCount(void);
void getAlignment(int alignment, char* player);
int minimax(node* currentNode, int i, int j, int parentScore, bool* parentsFirstChildAssigned);
bool isLegalAI(int i, int j, int alignment);
bool isAnyMoveAvailableAI(int alignment);
int boardCountAI(void);

/**
 * main game function
 */
int main(void)
{
    // fill board with zeroes
    for (int i = 0; i < BOARD_MAX; i++)
    {
        for (int j = 0; j < BOARD_MAX; j++)
        {
           board[i][j] = 0;
        }
    }
    
    // put starting tiles at center of board
    board[3][3] = 1;
    board[3][4] = -1;
    board[4][3] = -1;
    board[4][4] = 1;
    
    // print the current board
    printboard();
    
    // declare variables for specifying tile of interest
    int i;
    int j;
    
    // declare variable for player alignment and initialize to -1 ("player black")
    int alignment = -1;
    
    // allocate memory for string to store alignment of current player
    char* player = malloc(6);
    getAlignment(alignment, player);
    
    // main game loop
    while (true)
    {
        
        // check if there are any legal moves available, and if not, break main game loop
        if (!isAnyMoveAvailable(alignment))
        {
            alignment *= -1;
            if (!isAnyMoveAvailable(alignment))
            {
                printf("There are no more legal moves available.  ");
                break;
            }
            printf("There are no legal moves available for player %s.  ", player);
            getAlignment(alignment, player);
            printf("Player %s, it is now your turn.\n", player);
        }
        
        printf("Player %s, enter coordinates: ", player);
        
        // if it is human player's turn (player black), check user input 
        if (alignment == -1)
        {
            // get tile coordinates in form of user-inputted string
            char* coord = GetString();
            printf("\n");
            
            // check length of user-inputted string
            int k;
            for (k = 0; *(coord + k) != '\0'; k++);
            if (k != 2)
            {
                printf("Input should consist of only two characters.  Try again.\n");
                free(coord);
                continue;
            }
            
            // convert user-inputted string to board indices
            i = *(coord + 1) - '1';
            j = toupper(*coord) - 'A';
            free(coord);
            
            // check if column input is appropriate
            if (j < 0 || j >= BOARD_MAX)
            {
                printf("Column must be a letter between A and H.\n");
                continue;
            }
            
            // check if row input is appropriate
            if (i < 0 || i >= BOARD_MAX)
            {
                printf("Row must be a number between 1 and 8.\n");
                continue;
            }
            
        }
        
        // else get input from A.I.
        else
        {
            
            // set depth variable to zero
            z = 0;
            
            // create root node
            node* root = malloc(sizeof(node));
            
             // copy board to boardAI variable and boardLocal
            for (int m = 0; m < BOARD_MAX; m++)
            {
                for (int n = 0; n < BOARD_MAX; n++)
                {
                    boardAI[m][n] = board[m][n];
                    root->boardLocal[m][n] = board[m][n];
                }
            }
            
            // initialize score to 65 (65 arbitrarily chosen as default value because possible range for score variable is -64 to 64)
            root->score = 65;
            
            // malloc variable to store whether first child has been assigned and initialize value pointed to by variable to false (i.e., first child not yet assigned)
            root->firstChildAssigned = malloc(sizeof(bool));
            *(root->firstChildAssigned) = false;
            
            // declare variable for returned score
            int returnScore;
            
            // minimax all child nodes recursively
            for (int m = 0; m < BOARD_MAX; m++)
            {
                for (int n = 0; n < BOARD_MAX; n++)
                {
                    
                    // if return score is not 65 (default value), consider setting score as return score
                    root->children[m][n] = malloc(sizeof(node));
                    returnScore = minimax(root->children[m][n], m, n, root->score, root->firstChildAssigned);
                    free(root->children[m][n]);
                    
                    // change board back to original configuration after running minimax
                    for (int a = 0; a < BOARD_MAX; a++)
                    {
                        for (int b = 0; b < BOARD_MAX; b++)
                        {
                            boardAI[a][b] = root->boardLocal[a][b]; // MAJOR room for improvement here: could check if legal (which flips tiles on boardAI) right before executing minimax-->this would make it so these two for loops don't have to be run after EVERY minimax
                        }
                    }
                    
                    // if returned score is not default value, consider changing score of root node
                    if (returnScore != 65)
                    {
                    
                        // if current score is default or if returned score is higher than current score, set returned score as score or root node and save i and j
                        if (root->score == 65)
                        {
                            root->score = returnScore;
                            i = m;
                            j = n;
                        }
                        else if (returnScore > root->score)
                        {
                            root->score = returnScore;
                            i = m;
                            j = n;
                        }
                        
                    }
                    
                }
            }
            
            free(root->firstChildAssigned);
            free(root);
            
            printf("%c%c\n\n", j + 'A', i + '1');
            
        }
        
        // check if move is legal
        if (isLegal(i, j, alignment, true))
        {
            printboard();
            alignment *= -1;
            getAlignment(alignment, player);
        }

    }
    
    // determine winner of game
    int count = boardCount();
    if (count == 0)
    {
        printf("It's a tie!\n");
    }
    else
    {
        alignment = count / abs(count);
        getAlignment(alignment, player);
        printf("Player %s is the winner!\n", player);
    }

    // free variable for storing player alignment string
    free(player);
    
    // end program
    return 0;
}

/**
 * print current board
 */
void printboard(void)
{
    for (int i = 0; i < BOARD_MAX + 1; i++) // the board has 9 rows including the top row with column letters
    {
        for (int j = 0; j < BOARD_MAX + 1; j++) // the board has 9 columns including the left-most column with row numbers
        {
            
            // if first row, print column letters in first row
            if (i == 0)
            {
                if (j == 0)
                {
                    printf("    ");
                }
                else
                {
                    printf("%c   ", 'A' + j - 1);
                }
            }
            
            // else print row number followed by tiles
            else
            {
                if (j == 0)
                {
                    printf("%d | ", i);
                }
                else
                {
                    switch(board[i - 1][j - 1])
                    {
                        case -1 :
                            printf("X | ");
                            break;
                        case 0 :
                            printf("  | ");
                            break;
                        case 1 :
                            printf("O | ");
                            break;
                        default :
                            printf("Error");
                    }
                }
            }
        }
        
        // print border between rows
        printf("\n  +---+---+---+---+---+---+---+---+ \n");
    }
    
    printf("\n");
    
    return;
}

/**
 * check if move is legal and flip affected pieces if flip argument is set to true
 */
bool isLegal(int i, int j, int alignment, bool flip)
{
    
    // check to see if the tile is already full
    if (board[i][j] != 0)
    {
        if (flip == true)
        {
            printf("Invalid move.  Please choose a blank tile.\n");
        }
        return false;
    }
    
    // declare and initialize variable for returning legality of move
    bool legality = false;
    
    // remember coordinates of tile to be checked
    int i_origin = i;
    int j_origin = j;
    
    // declare variables to store checking direction
    int x;
    int y;
    
    // iterate over possible horizontal checking increments (i.e., left, right or none)
    for (int m = 0; m < 3; m++)
    {
        
        // determine horizontal checking increment
        switch (m)
        {
            case 0 :
                x = -1;
                break;
            case 1 :
                x = 0;
                break;
            case 2 :
                x = 1;
        }
        
        // check if column to be checked is out of bounds
        if ((i_origin + x) < 0 || (i_origin + x) >= BOARD_MAX)
        {
            continue;
        }
        
        // iterate over possible vertical checking increments (i.e., up, down or none)
        for (int n = 0; n < 3; n++)
        {
            
            // determine vertical checking increment
            switch (n)
            {
                case 0 :
                    y = -1;
                    break;
                case 1 :
                    y = 0;
                    break;
                case 2 :
                    y = 1;
            }
            
            if (x == 0 && y == 0)
            {
                continue;
            }
            
            // check if row to be checked is out of bounds
            if ((j_origin + y) < 0 || (j_origin + y) >= BOARD_MAX)
            {
                continue;
            }
            
            i = i_origin;
            j = j_origin;
            
            // iterate outwards in checking direction (as long as next tile has alignment that is opposite of origin tile) to determine if any pieces can be flipped
            while(board[i + x][j + y] == (alignment * (-1)))
            {
                i += x;
                j += y;
                
                // break if next tile is out of bounds
                if ((i + x) < 0 || (i + x) >= BOARD_MAX || (j + y) < 0 || (j + y) >= BOARD_MAX)
                {
                    break;
                }
                
                // break if next tile is zero
                if (board[i + x][j + y] == 0)
                {
                    break;
                }
                
                // either flip affected tiles or return true if next tile has same alignment as origin tile
                if (board[i + x][j + y] == alignment)
                {
                    
                    // if flip argument is set to true, flip affected tiles
                    if (flip == true)
                    {
                        // iterate backwards over tiles to be flipped
                        while (i != i_origin || j != j_origin)
                        {
                            board[i][j] *= -1;
                            i -= x;
                            j -= y;
                        }
                        legality = true;
                        break;
                        
                    }
                    
                    // else return true
                    else
                    {
                        return true;
                    }
                    
                }
            }
        }
    }
    
    // return legality of move (note: will return true if flip argument is true and at least one flip was made) and flip origin tile if legality is true and flip argument is true
    if (legality == false && flip == true)
    {
        printf("Invalid move.  Your move must result in at least one tile being flipped.\n");
    }
    if (legality == true && flip == true)
    {
        board[i_origin][j_origin] = alignment;
    }
    return legality;
    
}

/**
 * iterate over each tile of the game board to check if there are any legal moves available
 */
bool isAnyMoveAvailable(int alignment)
{
    for (int i = 0; i < BOARD_MAX; i++)
    {
        for (int j = 0; j < BOARD_MAX; j++)
        {
            if (isLegal(i, j, alignment, false))
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * determine current score
 */
int boardCount(void)
{
    int count = 0;
    for (int i = 0; i < BOARD_MAX; i++)
    {
        for (int j = 0; j < BOARD_MAX; j++)
        {
            count += board[i][j];
        }
    }
    return count;
}

/**
 * put player alignment into string
 */
void getAlignment(int alignment, char* player)
{
    if (alignment == 1)
        sprintf(player, "white");
    else
        sprintf(player, "black");
}

/**
 * iterate through A.I. decision tree while malloc-ing and freeing nodes as needed
 */
int minimax(node* currentNode, int i, int j, int parentScore, bool* parentsFirstChildAssigned)
{
    
    // set alignment based on tree depth (even-numbered tree depth indicates same alignment as AI, odd indicates opposite alignment)
    int alignment;
    alignment = (z % 2 == 0) ? 1 : -1;
    
    // execute action corresponding to child node if move is legal
    if(isLegalAI(i, j, alignment))
    {
        
        // increase depth by 1 and switch alignment
        z++;
        alignment *= -1;
        
        // store current board in struct for current node
        for (int m = 0; m < BOARD_MAX; m++)
        {
            for (int n = 0; n < BOARD_MAX; n++)
            {
                currentNode->boardLocal[m][n] = boardAI[m][n];
            }
        }
        
        // use function argument to determine whether current node is first child node of parent
        if (*parentsFirstChildAssigned == false)
        {
            *parentsFirstChildAssigned = true;
            currentNode->isFirstChild = true;
        }
        else
        {
            currentNode->isFirstChild = false;
        }
        
        // set current node score to default value
        currentNode->score = 65;
        
        // if max depth has been reached or if there are no more moves available (i.e., leaf node has been reached), determine score of current board
        if (z >= DEPTH_MAX || !isAnyMoveAvailableAI(alignment))
        {
            z--;
            return boardCountAI();
        }
        
        // else return score based on scores of children and parent nodes
        else
        {
            int returnScore;
            
            // malloc variable to store whether first child has been assigned to current node
            currentNode->firstChildAssigned = malloc(sizeof(bool));
            *(currentNode->firstChildAssigned) = false;
        
            for (int m = 0; m < BOARD_MAX; m++)
            {
                for (int n = 0; n < BOARD_MAX; n++)
                {
                    
                    // minimax child node and get returned score
                    currentNode->children[m][n] = malloc(sizeof(node));
                    returnScore = minimax(currentNode->children[m][n], m, n, currentNode->score, currentNode->firstChildAssigned);
                    int debug = z; // DEBUG
                    if (false) // DEBUG
                        printf("%d\n", debug); // DEBUG
                    free(currentNode->children[m][n]);
                    
                    // change board back to original configuration after running minimax
                    for (int a = 0; a < BOARD_MAX; a++)
                    {
                        for (int b = 0; b < BOARD_MAX; b++)
                        {
                            boardAI[a][b] = currentNode->boardLocal[a][b]; // MAJOR room for improvement here: could check if legal (which flips tiles on boardAI) right before executing minimax-->this would make it so these two for loops don't have to be run after EVERY minimax
                        }
                    }
                    
                    // if returned score is not default value, calculate score for current node based on children and parent nodes
                    if (returnScore != 65)
                    {
                        
                        // if current node is not first child, break if constrained by current alignment and parent score so that default value for score is returned (alpha-beta principle)
                        if (!currentNode->isFirstChild)
                        {
                            if (alignment != 1)
                            {
                                if (returnScore < parentScore)
                                {
                                    break;
                                }
                            }
                            else
                            {
                                if (returnScore > parentScore)
                                {
                                    break;
                                }
                            }
                        }
                        
                        // if not constrained (see above), set current score to return score based on alignment
                        if (alignment != 1)
                        {
                            if (currentNode->score == 65 || (returnScore < currentNode->score && currentNode->score != 65))
                            {
                                currentNode->score = returnScore;
                            }
                            
                        }
                        else
                        {
                            if (currentNode->score == 65 || returnScore > currentNode->score)
                            {
                                currentNode->score = returnScore;
                            }
                        }
                        
                    }
                    
                }
            }
            
            // subtract depth level and return score of node
            free(currentNode->firstChildAssigned);
            z--;
            return currentNode->score;
            
        }
        
    }
    
    // else (if not legal) return default value of 65
    else
    {
        return 65;
    }
    
}

/**
 * check if move is legal and flip affected pieces
 */
bool isLegalAI(int i, int j, int alignment)
{
    
    // check to see if the tile is already full
    if (boardAI[i][j] != 0)
    {
        return false;
    }
    
    // declare and initialize variable for returning legality of move
    bool legality = false;
    
    // remember coordinates of tile to be checked
    int i_origin = i;
    int j_origin = j;
    
    // declare variables to store checking direction
    int x;
    int y;
    
    // iterate over possible horizontal checking increments (i.e., left, right or none)
    for (int m = 0; m < 3; m++)
    {
        
        // determine horizontal checking increment
        switch (m)
        {
            case 0 :
                x = -1;
                break;
            case 1 :
                x = 0;
                break;
            case 2 :
                x = 1;
        }
        
        // check if column to be checked is out of bounds
        if ((i_origin + x) < 0 || (i_origin + x) >= BOARD_MAX)
        {
            continue;
        }
        
        // iterate over possible vertical checking increments (i.e., up, down or none)
        for (int n = 0; n < 3; n++)
        {
            
            // determine vertical checking increment
            switch (n)
            {
                case 0 :
                    y = -1;
                    break;
                case 1 :
                    y = 0;
                    break;
                case 2 :
                    y = 1;
            }
            
            if (x == 0 && y == 0)
            {
                continue;
            }
            
            // check if row to be checked is out of bounds
            if ((j_origin + y) < 0 || (j_origin + y) >= BOARD_MAX)
            {
                continue;
            }
            
            i = i_origin;
            j = j_origin;
            
            // iterate outwards in checking direction (as long as next tile has alignment that is opposite of origin tile) to determine if any pieces can be flipped
            while(boardAI[i + x][j + y] == (alignment * (-1)))
            {
                i += x;
                j += y;
                
                // break if next tile is out of bounds
                if ((i + x) < 0 || (i + x) >= BOARD_MAX || (j + y) < 0 || (j + y) >= BOARD_MAX)
                {
                    break;
                }
                
                // break if next tile is zero
                if (boardAI[i + x][j + y] == 0)
                {
                    break;
                }
                
                // either flip affected tiles if next tile has same alignment as origin tile
                if (boardAI[i + x][j + y] == alignment)
                {
                    
                    // iterate backwards over tiles to be flipped
                    while (i != i_origin || j != j_origin)
                    {
                        boardAI[i][j] *= -1;
                        i -= x;
                        j -= y;
                    }
                    legality = true;
                    break;
                    
                }
            }
        }
    }
    
    // if at least one flip was made, "place" origin tile by setting it equal to alignment
    if (legality == true)
    {
        boardAI[i_origin][j_origin] = alignment;
    }
    return legality;
    
}

/**
 * iterate over each tile of the game board to check if there are any legal moves available
 */
bool isAnyMoveAvailableAI(int alignment)
{
    for (int i = 0; i < BOARD_MAX; i++)
    {
        for (int j = 0; j < BOARD_MAX; j++)
        {
            if (isLegalAI(i, j, alignment))
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * determine current score
 */
int boardCountAI(void)
{
    int count = 0;
    for (int i = 0; i < BOARD_MAX; i++)
    {
        for (int j = 0; j < BOARD_MAX; j++)
        {
            count += boardAI[i][j];
        }
    }
    return count;
}