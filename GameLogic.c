#include "GameLogic.h"

/***************
* Function Name: initializeMove
* Input:
* Output: move
* Function Operation: sets all values to NULL or 0
*****************/
Move initializeMove() {
	Move move;
	move.srcPiece = '\0';
	move.promotionPiece = '\0';
	move.srcCol = '\0';
	move.srcRow = '\0';
	move.destCol = '\0';
	move.destRow = '\0';
	move.iSrc = 0;
	move.jSrc = 0;
	move.iDest = 0;
	move.jDest = 0;
	move.isWhite = 0;
	move.isCapture = 0;
	move.isPromotion = 0;
	move.isCheck = 0;
	move.isMate = 0;
	move.haveiSrc = 0;
	move.havejSrc = 0;
	move.couldntFindSrc = 0;
	return move;
}

int toDigit(char c) {
	return c - '0';
}

int charToDigit(char c) {
	return c - 'a';
}

/***************
* Function Name: isWhite
* Input: char that represents current space on board
* Output: 1 if white, 0 if not
* Function Operation: checks if piece is white or not
*****************/
int isWhite(char piece) {
	if (piece == 'K' || piece == 'Q' || piece == 'N' || piece == 'R' || piece == 'B' || piece == 'P')
		return 1;
	return 0;
}

/***************
* Function Name: createBoard
* Input: board, fen
* Output:
* Function Operation: initializes board from recieved fen
*****************/
void createBoard(char board[][SIZE], char fen[]) {
	int k = 0;
	while (k < FENSIZE) {
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				if (isdigit(fen[k])) {
					int spaces = toDigit(fen[k]);
					for (int l = 0; l < spaces; l++) {
						board[i][j] = ' ';
						j++;
					}
					k++;
				}
				if (fen[k] == '/') {
					k++;
					j--;
				} else {
					board[i][j] = fen[k];
					k++;
				}
			}
		}
	}
}

/***************
* Function Name: isInCheck
* Input: bard, move
* Output: 1 if white in check, 2 if black in check, 0 if neither
* Function Operation: finds kings and checks if theyre in danger
*****************/
int isInCheck(char board[][SIZE], Move move) {
	//finds kings and sets their locations in new structs to send to canReach
	Move whiteKing;
	Move blackKing;
	Move* whitePointer = &whiteKing, * blackPointer = &blackKing;
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (board[i][j] == 'K') {
				whiteKing.iDest = i;
				whiteKing.jDest = j;
			}
			if (board[i][j] == 'k') {
				blackKing.iDest = i;
				blackKing.jDest = j;
			}
		}
	}
	//checks if white king is in check
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			//if white king is in check
			if (!isWhite(board[i][j]) && board[i][j] != ' ') {
				//sets the source piece not to the king, but to the potential attacker
				whiteKing.iSrc = i;
				whiteKing.jSrc = j;
				whiteKing.srcPiece = board[i][j];
				if (canReach(board, *whitePointer) && !isBlocked(board, *whitePointer))
					return white;
			}
			//if black king is in check
			if (isWhite(board[i][j])) {
				blackKing.iSrc = i;
				blackKing.jSrc = j;
				blackKing.srcPiece = board[i][j];
				if (canReach(board, *blackPointer) && !isBlocked(board, *blackPointer))
					return black;
			}
		}
	}
	return 0;
}

/***************
* Function Name: willBeInCheck
* Input: board, move, isWhiteTurn
* Output: 1 if white will be in check, 2 if black will be in check, 0 if neither will be next turn
* Function Operation: creates a copy of the current board and performs the current move on it, then calls isInCheck
*****************/
int willBeInCheck(char board[][SIZE], Move move, int isWhiteTurn) {
	Move* movePointer = &move;
	char possibleBoard[SIZE][SIZE];
	//creates a second board to check if in that case will be in check
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++)
			possibleBoard[i][j] = board[i][j];
	}
	//performs the action on the possible board
	possibleBoard[move.iDest][move.jDest] = move.srcPiece;
	possibleBoard[move.iSrc][move.jSrc] = ' ';
	if (isWhiteTurn && (isInCheck(possibleBoard, *movePointer) == white))
		return white;
	if (!isWhiteTurn && (isInCheck(possibleBoard, *movePointer) == black))
		return black;
	//returns 0 if wont be in check
	return 0;
}

/***************
* Function Name: readMove
* Input: pgn, isWhiteturn
* Output: move
* Function Operation: counts amount of letters and numbers in the string and assigns them to 'move' accordingly
*****************/
Move readMove(char pgn[], int isWhiteTurn) {
	int countLowerLetters = 0;
	int countUpperLetters = 0;
	int countNumbers = 0;
	//sends move to function thats sets all values to NULL or 0
	Move move = initializeMove();
	//reads pgn and keeps track of amount of which chars
	for (int i = 0; i < strlen(pgn); i++) {
		if (pgn[i] >= 'a' && pgn[i] <= COLSIZE)
			countLowerLetters++;
		if (isdigit(pgn[i])) {
			int rowSize = toDigit(pgn[i]);
			if (rowSize >= 0 && rowSize <= SIZE)
				countNumbers++;
		}
		if (pgn[i] == 'K' || pgn[i] == 'Q' || pgn[i] == 'N' || pgn[i] == 'R' || pgn[i] == 'B')
			countUpperLetters++;
		if (pgn[i] == '+')
			move.isCheck = 1;
		if (pgn[i] == '=')
			move.isPromotion = 1;
		if (pgn[i] == 'x')
			move.isCapture = 1;
		if (pgn[i] == '#')
			move.isMate = 1;
	}
	//if there are 2 lowercase letters in the pgn, the 1st will be the src, the 2nd the dest
	if (countLowerLetters == 2) {
		for (int i = strlen(pgn); move.destCol == '\0'; i--) {
			if (pgn[i] >= 'a' && pgn[i] <= COLSIZE)
				move.destCol = pgn[i];
		}
		for (int i = 0; move.srcCol == '\0'; i++) {
			if (pgn[i] >= 'a' && pgn[i] <= COLSIZE)
				move.srcCol = pgn[i];
		}
	}
	else
		for (int i = 0; move.destCol == '\0'; i++) {
			if (pgn[i] >= 'a' && pgn[i] <= COLSIZE)
				move.destCol = pgn[i];
		}
	//assigns srcRow and destRow if given, if 2 numbers are given-the 1st is the src, 2nd is the dest
	if (countNumbers == 2) {
		for (int i = strlen(pgn); move.destRow == '\0'; i--) {
			if (isdigit(pgn[i])) {
				int rowSize = toDigit(pgn[i]);
				if (rowSize >= 0 && rowSize <= SIZE)
					move.destRow = pgn[i];
			}
		}
		for (int i = 0; move.srcRow == '\0'; i++) {
			if (isdigit(pgn[i])) {
				int rowSize = toDigit(pgn[i]);
				if (rowSize >= 0 && rowSize <= SIZE)
					move.srcRow = pgn[i];
			}
		}
	}
	//if only one number is given in the pgn, its the dest
	else
		for (int i = 0; move.destRow == '\0'; i++) {
			if (isdigit(pgn[i])) {
				int rowSize = toDigit(pgn[i]);
				if (rowSize >= 0 && rowSize <= SIZE)
					move.destRow = pgn[i];
			}
		}
	//changes dest from char to int
	move.jDest = charToDigit(move.destCol);
	move.iDest = SIZE - toDigit(move.destRow);
	//changes src for char to int if exists
	if (move.srcRow != '\0') {
		move.iSrc = SIZE - toDigit(move.srcRow);
		move.haveiSrc = 1;
	}
	if (move.srcCol != '\0') {
		move.jSrc = charToDigit(move.srcCol);
		move.havejSrc = 1;
	}

	//if pgn has upper letters and the first element is one, thats the srcPiece. If has uppers and not the first element,
	//thats the promotion. If it doesnt have upper letters, sets Pawn.
	if (countUpperLetters) {
		if (pgn[0] == 'K' || pgn[0] == 'Q' || pgn[0] == 'N' || pgn[0] == 'R' || pgn[0] == 'B')
			move.srcPiece = pgn[0];
		else
			if (!move.isCheck && !move.isMate) {
				move.promotionPiece = pgn[strlen(pgn) - 1];
				move.srcPiece = 'P';
			}
			else {
				move.srcPiece = 'P';
				move.promotionPiece = pgn[strlen(pgn) - 2];
			}
	}
	else
		move.srcPiece = 'P';

	if (!isWhiteTurn)
		move.srcPiece = tolower(move.srcPiece);
	return move;
}

/***************
* Function Name: canReach
* Input: board, move
* Output: 1 if can reach, 0 if not
* Function Operation: checks according to the rules of the piece if it can reach that location
*****************/
int canReach(char board[][SIZE], Move move) {
	//makes sure is actually moving 
	if (move.iSrc == move.iDest && move.jSrc == move.jDest)
		return 0;
	//rook move
	if (move.srcPiece == 'R' || move.srcPiece == 'r')
		return ((move.iSrc == move.iDest) || (move.jSrc == move.jDest));
	//bishop move
	if (move.srcPiece == 'B' || move.srcPiece == 'b')
		return (abs(move.iSrc - move.iDest) == abs(move.jSrc - move.jDest));
	//knight move
	if (move.srcPiece == 'N' || move.srcPiece == 'n')
		return ((abs(move.iSrc - move.iDest) == 2 && abs(move.jSrc - move.jDest) == 1) || \
		(abs(move.iSrc - move.iDest) == 1 && abs(move.jSrc - move.jDest) == 2));
	//queen move
	if (move.srcPiece == 'Q' || move.srcPiece == 'q')
		return (((move.iSrc == move.iDest) || (move.jSrc == move.jDest)) || \
		(abs(move.iSrc - move.iDest) == abs(move.jSrc - move.jDest)));
	//king move
	if (move.srcPiece == 'K' || move.srcPiece == 'k')
		return ((abs(move.iSrc - move.iDest) == 1 && abs(move.jSrc - move.jDest) == 0) || \
		((abs(move.jSrc - move.jDest) == 1) && abs(move.iSrc - move.iDest) == 0) || \
			((abs(move.jSrc - move.jDest) == 1) && abs(move.iSrc - move.iDest) == 1));
	//pawn move if not capturing
	if ((move.srcPiece == 'P' || move.srcPiece == 'p') && !move.isCapture) {
		if (isWhite(board[move.iSrc][move.jSrc])) {
			if (move.iSrc == 6) {
				if (((move.iSrc - move.iDest) == 1 || (move.iSrc - move.iDest) == 2) && (move.jSrc == move.jDest))
					return 1;
			}
			if (((move.iSrc - move.iDest) == 1) && (move.jSrc == move.jDest))
				return 1;
		}
		else {
			if (move.iSrc == 1) {
				if (((move.iDest - move.iSrc) == 1 || (move.iDest - move.iSrc) == 2) && (move.jSrc == move.jDest))
					return 1;
			}
			if (((move.iDest - move.iSrc) == 1) && (move.jSrc == move.jDest))
				return 1;
		}
		return 0;
	}
	//pawn move if capturing
	if ((move.srcPiece == 'P' || move.srcPiece == 'p') && move.isCapture) {
		if (isWhite(move.srcPiece)) {
			if ((move.iSrc - move.iDest) == 1 && abs(move.jDest - move.jSrc) == 1)
				return 1;
		}
		else
			if ((move.iDest - move.iSrc) == 1 && (abs(move.jDest - move.jSrc) == 1))
				return 1;
		return 0;
	}
	return 0;
}

/***************
* Function Name: findSrc
* Input: board, move, isWhiteTurn
* Output: move
* Function Operation: sends pieces to canReach() in different for loops depending on given information
*****************/
Move findSrc(char board[][SIZE], Move move, int isWhiteTurn) {
	Move* movePointer = &move;
	//if have neither isrc or jsrc
	if (!move.haveiSrc && !move.havejSrc) {
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				if (board[i][j] == move.srcPiece) {
					move.iSrc = i;
					move.jSrc = j;
					if (canReach(board, *movePointer) && !willBeInCheck(board, *movePointer, isWhiteTurn)\
						&& !isBlocked(board, *movePointer))
						return move;
				}
			}
		}
	}
	//if have isrc but not jsrc
	if (move.haveiSrc && !move.havejSrc) {
		for (int j = 0; j < SIZE; j++) {
			if (board[move.iSrc][j] == move.srcPiece) {
				move.jSrc = j;
				if (canReach(board, *movePointer) && !willBeInCheck(board, *movePointer, isWhiteTurn)\
					&& !isBlocked(board, *movePointer))
					return move;
			}
		}
	}
	//if have jsrc but not isrc
	if (!move.haveiSrc && move.havejSrc) {
		for (int i = 0; i < SIZE; i++) {
			if (board[i][move.jSrc] == move.srcPiece) {
				move.iSrc = i;
				if (canReach(board, *movePointer) && !willBeInCheck(board, *movePointer, isWhiteTurn)\
					&& !isBlocked(board, *movePointer))
					return move;
			}
		}
	}
	move.couldntFindSrc = 1;
	return move;
}

/***************
* Function Name: doPromotion
* Input: board, move
* Output:
* Function Operation: converts pawn to desired piece
*****************/
int doPromotion(char board[][SIZE], Move move) {
	//if pawn is white
	if (move.srcPiece == 'P' && move.iDest == 0) {
		board[move.iDest][move.jDest] = move.promotionPiece;
		board[move.iSrc][move.jSrc] = ' ';
		return 1;
	}
	//if pawn is black
	else
		if (move.srcPiece == 'p' && move.iDest == SIZE - 1) {
			board[move.iDest][move.jDest] = tolower(move.promotionPiece);
			board[move.iSrc][move.jSrc] = ' ';
			return 1;
		}
	return 0;
}

/***************
* Function Name: straightPath
* Input: board, move
* Output: 1 if free, 0 if not
* Function Operation: checks if the piece is blocked in straight lines from its dest
*****************/
int straightPath(char board[][SIZE], Move move) {
	//moving up
	if (move.iSrc - move.iDest > 0) {
		for (int i = move.iSrc - 1; i > (move.iDest); i--) {
			if (board[i][move.jSrc] != ' ')
				return 0;
		}
	}
	//moving down
	if (move.iSrc - move.iDest < 0) {
		for (int i = move.iSrc + 1; i < (move.iDest); i++) {
			if (board[i][move.jSrc] != ' ')
				return 0;
		}
	}
	//moving left
	if (move.jSrc - move.jDest > 0) {
		for (int j = move.jSrc - 1; j > (move.jDest); j--) {
			if (board[move.iSrc][j] != ' ')
				return 0;
		}
	}
	//moving right
	if (move.jSrc - move.jDest < 0) {
		for (int j = move.jSrc + 1; j < (move.jDest); j++) {
			if (board[move.iSrc][j] != ' ')
				return 0;
		}
	}
	return 1;
}

/***************
* Function Name: diagonalPath
* Input: board, move
* Output: 1 if free, 0 if not
* Function Operation: checks if the piece if blocked in diagonal lines from its dest
*****************/
int diagonalPath(char board[][SIZE], Move move) {
	//up right
	if (move.iDest - move.iSrc < 0 && move.jDest - move.jSrc > 0) {
		for (int i = move.iSrc - 1, j = move.jSrc + 1; i > (move.iDest) && j < (move.jDest); i--, j++) {
			if (board[i][j] != ' ')
				return 0;
		}
	}
	//up left
	if (move.iDest - move.iSrc < 0 && move.jDest - move.jSrc < 0) {
		for (int i = move.iSrc - 1, j = move.jSrc - 1; i > move.iDest&& j > move.jDest; i--, j--) {
			if (board[i][j] != ' ')
				return 0;
		}
	}
	//down right
	if (move.iDest - move.iSrc > 0 && move.jDest - move.jSrc > 0) {
		for (int i = move.iSrc + 1, j = move.jSrc + 1; i < (move.iDest) && j < (move.jDest); i++, j++) {
			if (board[i][j] != ' ')
				return 0;
		}
	}
	//down left
	if (move.iDest - move.iSrc > 0 && move.jDest - move.jSrc < 0) {
		for (int i = move.iSrc + 1, j = move.jSrc - 1; i < (move.iDest) && j >(move.jDest); i++, j--) {
			if (board[i][j] != ' ')
				return 0;
		}
	}
	return 1;
}

/***************
* Function Name: isBlocked
* Input: board, move
* Output: 0 if not, 1 if is
* Function Operation: calls diagonalPath() or straightPath() depending on the piece
*****************/
int isBlocked(char board[][SIZE], Move move) {
	Move* movePointer = &move;
	//rook move
	if (move.srcPiece == 'R' || move.srcPiece == 'r')
		if (!straightPath(board, *movePointer))
			return 1;
	//bishop move
	if (move.srcPiece == 'B' || move.srcPiece == 'b')
		if (!diagonalPath(board, *movePointer))
			return 1;
	//queen move
	if (move.srcPiece == 'Q' || move.srcPiece == 'q') {
		if ((move.iSrc == move.iDest) || (move.jSrc == move.jDest)) {
			if (!straightPath(board, *movePointer))
				return 1;
		}
		else
			if (!diagonalPath(board, *movePointer))
				return 1;
		//pawn move (only relevant for moving 2 spaces)
	}
	if (move.srcPiece == 'P' || move.srcPiece == 'p')
		if (!straightPath(board, *movePointer))
			return 1;
	return 0;
}

/***************
* Function Name: doMove
* Input: board, move, isWhiteTurn
* Output: 1 if successful, 0 if not
* Function Operation: checks if the move is legal (blocked, will be in check etc) and if so, performs move
*****************/
int doMove(char board[][SIZE], Move move, int isWhiteTurn) {
	Move* movePointer = &move;
	int inCheck = isInCheck(board, *movePointer);
	int inCheckNextTurn = willBeInCheck(board, *movePointer, isWhiteTurn);
	int willPutInCheck = willBeInCheck(board, *movePointer, !isWhiteTurn);
	//if not in check/mate but is declared, or if in check/mate but not declared, illegal.
	if ((!willPutInCheck && (move.isCheck || move.isMate)) || (willPutInCheck && !(move.isCheck || move.isMate)))
		return 0;
	//if is in check and still will be in check next move, illegal.
	if (inCheck) {
		if ((inCheck == white && inCheckNextTurn == white) || (inCheck == black && inCheckNextTurn == black))
			return 0;
	}

	//checks if move is capture if is legal capture
	if (move.isCapture) {
		if ((isWhite(board[move.iSrc][move.jSrc]) && !isWhite(board[move.iDest][move.jDest]) \
			&& board[move.iDest][move.jDest] != ' ') || (!isWhite(board[move.iSrc][move.jSrc]) \
				&& isWhite(board[move.iDest][move.jDest]) && board[move.iDest][move.jDest] != ' ')) {
			//makes sure correct syntax for pawn capturing
			if (tolower(move.srcPiece) == 'p' && move.srcCol == '\0')
				return 0;

			//if is promotion, checks if legal and if so, promotes.
			if (move.isPromotion) {
				if (doPromotion(board, *movePointer))
					return 1;
				else
					return 0;
			}
			else {
				board[move.iDest][move.jDest] = move.srcPiece;
				board[move.iSrc][move.jSrc] = ' ';
				return 1;
			}
		}
	}
	//if isn't capture
	else
		//makes sure spot is unoccupied if not capture
		if (board[move.iDest][move.jDest] == ' ') {
			if (move.isPromotion) {
				if (doPromotion(board, *movePointer))
					return 1;
				else
					return 0;
			}
			else {
				board[move.iDest][move.jDest] = move.srcPiece;
				board[move.iSrc][move.jSrc] = ' ';
				return 1;
			}
		}
	return 0;
}

/***************
* Function Name: makeMove
* Input: board, pgn, isWhiteTurn
* Output: 0 if successful, 0 if not
* Function Operation: calls readMove() and findSrc() to get all the information before sending it all to doMove()
*****************/
int makeMove(char board[][SIZE], char pgn[], int isWhiteTurn) {
	Move move = readMove(pgn, isWhiteTurn);		//parses pgn
	Move* movePointer = &move;
	move = findSrc(board, *movePointer, isWhiteTurn);		//finds source piece if not given
	if (move.couldntFindSrc)
		return 0;						//returns 0 if no available src piece (illegal move)
	if (!doMove(board, *movePointer, isWhiteTurn))
		return 0;					//returns 0 if illegal move
	return 1;						// returns 1 if legal move
}