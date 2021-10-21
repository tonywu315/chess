#a fun 2-player online game of chess

#the introduction tells you the rules of chess and how to play our game
#you can't actually click the chess pieces; you have to type the move

#Preconditions:
#you need to set the screen to the size of the game
#click options, click Configure IDLE, and change the size of the words to 22

#creates the starting board
board = [["♜","♞","♝","♛","♚","♝","♞","♜"],
         ["♟","♟","♟","♟","♟","♟","♟","♟"],
         [" "," "," "," "," "," "," "," "],
	 [" "," "," "," "," "," "," "," "],
	 [" "," "," "," "," "," "," "," "],
	 [" "," "," "," "," "," "," "," "],
	 ["♙","♙","♙","♙","♙","♙","♙","♙"],
	 ["♖","♘","♗","♕","♔","♗","♘","♖"]]

repetition_check = {}

player = "Player 1"
check_test = False
message = ""
previous_board = [list(x) for x in board]


#converts the letter to a number
def col(letter):
    alphabet = "abcdefgh"
    return alphabet.find(letter)


#checks to see if there is a check
def check():
    global player
    global check_test
    #finds the king’s and the pieces’ positions, and checks if the piece can attack the king
    if player == "Player 1":
        for row in range(1,9):
            for column in "abcdefgh":
                if board[8-row][col(column)] == "♔":
                    king_position = column + str(row)
        for row in range(1,9):
            for column in "abcdefgh":
                if board[8-row][col(column)] in ["♜","♞","♝","♛","♟","♚"]:
                    check_test = True
                    #if a piece can attack the king, it is check
                    if legal(column + str(row) + king_position) == True:
                        check_test = False
                        return True
                    check_test = False
    else:
        for row in range(1,9):
            for column in "abcdefgh":
                if board[8-row][col(column)] == "♚":
                    king_position = column + str(row)
        for row in range(1,9):
            for column in "abcdefgh":
                if board[8-row][col(column)] in ["♖","♘","♗","♕","♙","♔"]:
                    check_test = True
                    #if a piece can attack the king, it is check
                    if legal(column + str(row) + king_position) == True:
                        check_test = False
                        return True
                    check_test = False
    return False

#checks to see if there is a checkmate
def checkmate():
    global player
    global board
    #does every possible move and checks if it will get you out of check
    if check() == True:
        for row in range(1,9):
            for column in "abcdefgh":
                if player == "Player 1":
                    if board[8-row][col(column)] in ["♖","♘","♗","♕","♔","♙"]:
                        for row2 in range(1,9):
                            for column2 in "abcdefgh":
                                #checks if the move is legal
                                if legal(column + str(row) + column2 + str(row2)) == True:
                                    temp_board = [list(x) for x in board]
                                    move_piece(column + str(row) + column2 + str(row2))
                                    #if it gets you out of check, return False
                                    if check() == False:
                                        board = [list(x) for x in temp_board]
                                        return False
                                    else:
                                        board = [list(x) for x in temp_board]
                else:
                    if board[8-row][col(column)] in ["♜","♞","♝","♛","♚","♟"]:
                        for row2 in range(1,9):
                            for column2 in "abcdefgh":
                                #checks if the move is legal
                                if legal(column + str(row) + column2 + str(row2)) == True:
                                    temp_board = [list(x) for x in board]
                                    move_piece(column + str(row) + column2 + str(row2))
                                    #if it gets you out of check, return False
                                    if check() == False:
                                        board = [list(x) for x in temp_board]
                                        return False
                                    else:
                                        board = [list(x) for x in temp_board]
        return True
    return False


#checks to see if there is a stalemate
def stalemate():
    global player
    #checks if a piece can move
    for row in range(1,9):
        for column in "abcdefgh":
            if player == "Player 1":
                if board[8-row][col(column)] in ["♖","♘","♗","♕","♔","♙"]:
                    for row2 in range(1,9):
                        for column2 in "abcdefgh":
                            #if a piece can move, return False
                            if legal(column + str(row) + column2 + str(row2)) == True:
                                return False
            else:
                if board[8-row][col(column)] in ["♜","♞","♝","♛","♚","♟"]:
                    for row2 in range(1,9):
                        for column2 in "abcdefgh":
                            #if a piece can move, return False
                            if legal(column + str(row) + column2 + str(row2)) == True:
                                return False
    return True


piece_num = 32
moves = -1
previous_pawns = [[" "," "," "," "," "," "," "," "],
                  ["♟","♟","♟","♟","♟","♟","♟","♟"],
                  [" "," "," "," "," "," "," "," "],
                  [" "," "," "," "," "," "," "," "],
                  [" "," "," "," "," "," "," "," "],
                  [" "," "," "," "," "," "," "," "],
                  ["♙","♙","♙","♙","♙","♙","♙","♙"],
                  [" "," "," "," "," "," "," "," "]]
#checks to see if it is a draw
def draw():
    global message
    global piece_num
    global previous_pawns
    global moves
    global board
    if stalemate() == True:
        message = "Stalemate"
        return True
    
    #50 move with no capture or pawn move
    #checks if 50 moves have passed with no capture
    chess_pieces = []
    for row in range(8):
        for column in range(8):
            if board[row][column] != " ":
                chess_pieces.append(board[row][column])
    if len(chess_pieces) == piece_num:
        moves = moves + 1
    else:
        moves = 0
    #checks if a pawn moved
    pawns = [list(x) for x in board]
    for row in range(8):
        for column in range(8):
            if pawns[row][column] not in ["♟","♙"]:
                pawns[row][column] = " "
    #if a pawn moved, reset the moves
    if pawns != previous_pawns:
        moves = 0
    previous_pawns = [list(x) for x in pawns]
    #you need 100 moves because 1 move consists of both players moving
    if moves == 100:
        message = "50-Move Rule"
        return True
    
    #3 fold repetition
    global repetition_check
    if str(board) not in repetition_check:
        repetition_check[str(board)] = 1
    else:
        repetition_check[str(board)] = repetition_check[str(board)] + 1
    for position in repetition_check:
        if repetition_check[position] == 3:
            message = "Threefold Repetition"
            return True
    
    #impossible to checkmate
    #sorts the chess pieces in a certain order
    chess_pieces.sort()
    #returns True if the pieces on the board aren't enough to checkmate
    player_1 = chess_pieces[:chess_pieces.index("♚")]
    player_2 = chess_pieces[chess_pieces.index("♚"):]
    if player_1 in [["♔"],["♔","♗"],["♔","♘"]]:
        if player_2 in [["♚"],["♚","♝"],["♚","♞"]]:
            message = "Insufficient Material"
            return True
    return False


#checks to see if the move is legal
def legal(move):
    #makes the variables global
    global player
    global board
    global previous_board
    global king1_moved
    global king2_moved
    global k_rook1_moved
    global k_rook2_moved
    global q_rook1_moved
    global q_rook2_moved
    #if the move isn't a castle, run the code
    if move != "0-0" and move != "0-0-0":
        if len(move) in [4,5]:
            #if the 1st or 3rd letter isn't one of the columns return False
            if move[0] not in "abcdefgh" or move[2] not in "abcdefgh":
                return False
            #if the 2nd or 4th letter isn't one of the rows return False
            if move[1] not in "12345678" or move[3] not in "12345678":
                return False
            
            #if the piece that you want to move is a space return False
            if board[8-int(move[1])][col(move[0])] == " ":
                return False
            
            #if the piece that you want to move is the other player's piece return False
            if check_test == False:
                if player == "Player 1":
                    if board[8-int(move[1])][col(move[0])] in ["♜","♞","♝","♛","♚","♟"]:
                        return False
                else:
                    if board[8-int(move[1])][col(move[0])] in ["♙","♖","♘","♗","♕","♔"]:
                        return False
            
            #checks if moving a rook is legal
            if board[8-int(move[1])][col(move[0])] in ["♜","♖"]:
                #if the move is diagonal return False
                if move[0] != move[2] and move[1] != move[3]:
                    return False
                #checks if there are pieces between the start and the end
                for position in range((8-int(move[1]))+1,8-int(move[3])):
                    if board[position][col(move[0])] != " ":
                        return False
                for position in range((8-int(move[3]))+1,8-int(move[1])):
                    if board[position][col(move[0])] != " ":
                        return False
                for position in range(col(move[2])+1,col(move[0])):
                    if board[8-int(move[1])][position] != " ":
                        return False
                for position in range(col(move[0])+1,col(move[2])):
                    if board[8-int(move[1])][position] != " ":
                        return False
            
            #checks if moving a bishop is legal
            if board[8-int(move[1])][col(move[0])] in ["♝","♗"]:
                #checks if the vertical and horizontal distance traveled is the same
                v_distance = int(move[3])-int(move[1])
                h_distance = col(move[2])-col(move[0])
                if abs(v_distance) != abs(h_distance):
                    return False
                #checks if there are pieces between the start and the end
                if h_distance > 0:
                    for position in range(1,v_distance):
                        if board[(8-int(move[1]))-position][col(move[0])+position] != " ":
                            return False
                    for position in range(v_distance+1,0):
                        if board[(8-int(move[1]))-position][col(move[0])-position] != " ":
                            return False
                else:
                    for position in range(1,v_distance):
                        if board[(8-int(move[1]))-position][col(move[0])-position] != " ":
                            return False
                    for position in range(v_distance+1,0):
                        if board[(8-int(move[1]))-position][col(move[0])+position] != " ":
                            return False
            
            #checks if moving a king is legal
            if board[8-int(move[1])][col(move[0])] in ["♚","♔"]:
                #Checks the distance moved vertically and horizontally by the king
                v_distance = abs(int(move[3])-int(move[1]))
                h_distance = abs(col(move[2])-col(move[0]))
                #Makes sure the king does not move more than one square
                if v_distance > 1 or h_distance > 1:
                    return False
            
            #checks if moving a queen is legal
            if board[8-int(move[1])][col(move[0])] in ["♕","♛"]:
                if move[0] == move[2] or move[1] == move[3]:
                    #checks if there are pieces between the start and the end
                    for position in range((8-int(move[1]))+1,8-int(move[3])):
                        if board[position][col(move[0])] != " ":
                            return False
                    for position in range((8-int(move[3]))+1,8-int(move[1])):
                        if board[position][col(move[0])] != " ":
                            return False
                    for position in range(col(move[2])+1,col(move[0])):
                        if board[8-int(move[1])][position] != " ":
                            return False
                    for position in range(col(move[0])+1,col(move[2])):
                        if board[8-int(move[1])][position] != " ":
                            return False
                #checks if the move if diagonal
                if move[0] != move[2] and move[1] != move[3]:
                    #checks if the vertical and horizontal distance traveled is the same
                    v_distance = int(move[3])-int(move[1])
                    h_distance = col(move[2])-col(move[0])
                    if abs(v_distance) != abs(h_distance):
                        return False
                    #checks if there are pieces between the start and the end
                    if h_distance > 0:
                        for position in range(1,v_distance):
                            if board[(8-int(move[1]))-position][col(move[0])+position] != " ":
                                return False
                        for position in range(v_distance+1,0):
                            if board[(8-int(move[1]))-position][col(move[0])-position] != " ":
                                return False
                    else:
                        for position in range(1,v_distance):
                            if board[(8-int(move[1]))-position][col(move[0])-position] != " ":
                                return False
                        for position in range(v_distance+1,0):
                            if board[(8-int(move[1]))-position][col(move[0])+position] != " ":
                                return False
            
            #checks if moving a knight is legal
            if board[8-int(move[1])][col(move[0])] in ["♞","♘"]:
                #checks if the knight moves 2 squares in one direction and 1 square in the other direction
                v_distance = abs(int(move[3])-int(move[1]))
                h_distance = abs(col(move[2])-col(move[0]))
                if v_distance == 1:
                    if h_distance != 2:
                        return False
                elif v_distance == 2:
                    if h_distance != 1:
                        return False
                else:
                    return False
            #checks if moving a pawn is legal
            if board[8-int(move[1])][col(move[0])] in ["♟","♙"]:
                v_distance = int(move[3])-int(move[1])
                h_distance = abs(col(move[2])-col(move[0]))
                #pawns can't move backward
                if check_test == False:
                    if player == "Player 1":
                        if v_distance < 0:
                            return False
                    else:
                        if v_distance > 0:
                            return False
                #pieces can't attack while moving forward 2
                if abs(v_distance) == 2 and h_distance == 1:
                    return False
                #players can only move forward 2 on the first move of that pawn
                if player == "Player 1":
                    if v_distance == 2 and int(move[1]) != 2:
                        return False
                else:
                    if v_distance == -2 and int(move[1]) != 7:
                        return False
                #if the player doesn't move forward 2, the player has to move forward 1
                if abs(v_distance) not in [1,2]:
                    return False
                #you can't move more than 1 space horizontally
                if h_distance > 1:
                    return False
                #you can't attack by moving forward
                if h_distance != 1 and board[8-int(move[3])][col(move[2])] != " ":
                    return False
                #you can't go over other pieces
                if abs(v_distance) == 2 and board[(8-int(move[3]))+int(v_distance/2)][col(move[2])] != " ":
                    return False
                #checks if the pawn can move diagonally to attack
                if player == "Player 1":
                    if v_distance == 1 and h_distance == 1:
                        if board[8-int(move[3])][col(move[2])] == " ":
                            #checks if it is en passant
                            if int(move[3]) != 1:
                                if previous_board[(8-int(move[3]))-1][col(move[2])] != "♟":
                                    return False
                                if board[8-int(move[3])+1][col(move[2])] != "♟":
                                    return False
                                board[8-int(move[3])+1][col(move[2])] = " "
                else:
                    if v_distance == -1 and h_distance == 1:
                        if board[8-int(move[3])][col(move[2])] == " ":
                            #checks if it is en passant
                            if int(move[3]) != 1:
                                if previous_board[(8-int(move[3]))+1][col(move[2])] != "♙":
                                    return False
                                if board[8-int(move[3])-1][col(move[2])] != "♙":
                                    return False
                                board[8-int(move[3])-1][col(move[2])] = " "
                #pawn promotion
                #you have to promote your pawn when you reach the last row
                if board[8-int(move[1])][col(move[0])] in ["♟","♙"]:
                    if player == "Player 1":
                        if int(move[3]) == 8 and len(move) == 4:
                            return False
                    else:
                        if int(move[3]) == 1 and len(move) == 4:
                            return False
                if len(move) == 5:
                    #checks that the piece is a pawn
                    if board[8-int(move[1])][col(move[0])] not in ["♟","♙"]:
                        return False
                    #checks if the pawn is on the last rank
                    if player == "Player 1":
                        if int(move[3]) != 8:
                            return False
                    else:
                        if int(move[3]) != 1:
                            return False
                    #makes sure that the player changes into a legal piece
                    if move[4] not in ['B','R','N','Q']:
                        return False
                
        else:
            #if the length of move isn't 4 or 5 return False
            return False
    #if the move is a kingside castle
    elif move == "0-0":
        if player == "Player 1":
            #checks if there are pieces between the king and the rook
            if board[7][5] != " " or board[7][6] != " ":
                return False
            #checks if the king and rook have moved
            if king1_moved == True or k_rook1_moved == True:
                return False
            #checks if the king is in, passes, or ends up in check
            temp_board = [list(x) for x in board]
            if check() == True:
                return False
            board[7][5] = "♔"
            if check() == True:
                board = [list(x) for x in temp_board]
                return False
            else:
                board = [list(x) for x in temp_board]
            board[7][6] = "♔"
            if check() == True:
                board = [list(x) for x in temp_board]
                return False
            else:
                board = [list(x) for x in temp_board]
        else:
            #checks if there are pieces between the king and the rook
            if board[0][5] != " " or board[0][6] != " ":
                return False
            #checks if the king and rook have moved
            if king2_moved == True or k_rook2_moved == True:
                return False
            #checks if the king is in, passes, or ends up in check
            temp_board = [list(x) for x in board]
            if check() == True:
                return False
            board[0][5] = "♚"
            if check() == True:
                board = [list(x) for x in temp_board]
                return False
            else:
                board = [list(x) for x in temp_board]
            board[0][6] = "♚"
            if check() == True:
                board = [list(x) for x in temp_board]
                return False
            else:
                board = [list(x) for x in temp_board]
    #if the move is a queenside castle
    elif move == "0-0-0":
        if player == "Player 1":
            if board[7][1] != " " or board[7][2] != " " or board[7][3] != " ":
                return False
            #checks if the king and rook have moved
            if king1_moved == True or q_rook1_moved == True:
                return False
            #checks if the king is in, passes, or ends up in check
            temp_board = [list(x) for x in board]
            if check() == True:
                return False
            board[7][2] = "♔"
            if check() == True:
                board = [list(x) for x in temp_board]
                return False
            else:
                board = [list(x) for x in temp_board]
            board[7][3] = "♔"
            if check() == True:
                board = [list(x) for x in temp_board]
                return False
            else:
                board = [list(x) for x in temp_board]
        else:
            if board[0][1] != " " or board[0][2] != " " or board[0][3] != " ":
                return False
            #checks if the king and rook have moved
            if king2_moved == True or q_rook2_moved == True:
                return False
            #checks if the king is in, passes, or ends up in check
            if check() == True:
                return False
            temp_board = [list(x) for x in board]
            board[0][2] = "♚"
            if check() == True:
                board = [list(x) for x in temp_board]
                return False
            else:
                board = [list(x) for x in temp_board]
            board[0][3] = "♚"
            if check() == True:
                board = [list(x) for x in temp_board]
                return False
            else:
                board = [list(x) for x in temp_board]
            
    if check_test == False and move != "0-0" and move != "0-0-0":
        #if the piece attacks its own color its not legal
        if board[8-int(move[1])][col(move[0])] in ['♙','♖','♘','♗','♕','♔']:
            if board[8-int(move[3])][col(move[2])] in ['♙','♖','♘','♗','♕','♔']:
                return False
        if board[8-int(move[1])][col(move[0])] in ['♜','♞','♝','♛','♚','♟']:
            if board[8-int(move[3])][col(move[2])] in ['♜','♞','♝','♛','♚','♟']:
                return False
    #if the move causes check to your king, return False
    #finds the kings' position
    if move != "0-0" and move != "0-0-0":
        for row in range(1,9):
            for column in "abcdefgh":
                if board[8-row][col(column)] == "♔":
                    king1_position = column + str(row)
                if board[8-row][col(column)] == "♚":
                    king2_position = column + str(row)
        #only runs if a piece is not attacking a king
        if move[2:] not in [king1_position,king2_position]:
            temp_board = [list(x) for x in board]
            move_piece(move)
            if check() == True:
                board = [list(x) for x in temp_board]
                return False
            else:
                board = [list(x) for x in temp_board]
    return True


king1_moved = False
king2_moved = False
k_rook1_moved = False
k_rook2_moved = False
q_rook1_moved = False
q_rook2_moved = False
#moves the chess pieces
def move_piece(move):
    #checks if the king and rook have moved
    global king1_moved
    global king2_moved
    global k_rook1_moved
    global k_rook2_moved
    global q_rook1_moved
    global q_rook2_moved
    if board[0][4] != "♚":
        king2_moved = True
    if board[7][4] != "♔":
        king1_moved = True
    if board[0][7] != "♜":
        k_rook2_moved = True
    if board[7][7] != "♖":
        k_rook1_moved = True
    if board[0][0] != "♜":
        q_rook1_moved = True
    if board[7][0] != "♖":
        q_rook2_moved = True
    
    if move != "0-0" and move != "0-0-0":
        #moves the piece to the destination
        board[8-int(move[3])][col(move[2])] = board[8-int(move[1])][col(move[0])]
        #removes the piece from its starting position
        board[8-int(move[1])][col(move[0])] = " "
    #kingside castling
    elif move == "0-0":
        if player == "Player 1":
            board[7][6] = board[7][4]
            board[7][4] = " "
            board[7][5] = board[7][7]
            board[7][7] = " "
        else:
            board[0][6] = board[0][4]
            board[0][4] = " "
            board[0][5] = board[0][7]
            board[0][7] = " "
    #queenside castling
    elif move == "0-0-0":
        if player == "Player 1":
            board[7][2] = board[7][4]
            board[7][4] = " "
            board[7][3] = board[7][0]
            board[7][0] = " "
        else:
            board[0][2] = board[0][4]
            board[0][4] = " "
            board[0][3] = board[0][0]
            board[0][0] = " "
    if len(move) == 5:
        if player == "Player 1":
            if move[4] == 'B':
                board[8-int(move[3])][col(move[2])] = '♗'
            if move[4] == 'R':
                board[8-int(move[3])][col(move[2])] = '♖'
            if move[4] == 'N':
                board[8-int(move[3])][col(move[2])] = '♘'
            if move[4] == 'Q':
                board[8-int(move[3])][col(move[2])] = '♕'
        if player == "Player 2":
            if move[4] == 'B':
                board[8-int(move[3])][col(move[2])] = '♝'
            if move[4] == 'R':
                board[8-int(move[3])][col(move[2])] = '♜'
            if move[4] == 'N':
                board[8-int(move[3])][col(move[2])] = '♞'
            if move[4] == 'Q':
                board[8-int(move[3])][col(move[2])] = '♛'


#prints the chess board
def print_board():
    global player
    #rotates the board based on who is playing
    if player == "Player 1":
        print("☆ⓐⓑⓒⓓⓔⓕⓖⓗ☆")
        print("⑧"+board[0][0]+board[0][1]+board[0][2]+board[0][3]+board[0][4]+board[0][5]+board[0][6]+board[0][7]+"⑧")
        print("⑦"+board[1][0]+board[1][1]+board[1][2]+board[1][3]+board[1][4]+board[1][5]+board[1][6]+board[1][7]+"⑦")
        print("⑥"+board[2][0]+board[2][1]+board[2][2]+board[2][3]+board[2][4]+board[2][5]+board[2][6]+board[2][7]+"⑥")
        print("⑤"+board[3][0]+board[3][1]+board[3][2]+board[3][3]+board[3][4]+board[3][5]+board[3][6]+board[3][7]+"⑤")
        print("④"+board[4][0]+board[4][1]+board[4][2]+board[4][3]+board[4][4]+board[4][5]+board[4][6]+board[4][7]+"④")
        print("③"+board[5][0]+board[5][1]+board[5][2]+board[5][3]+board[5][4]+board[5][5]+board[5][6]+board[5][7]+"③")
        print("②"+board[6][0]+board[6][1]+board[6][2]+board[6][3]+board[6][4]+board[6][5]+board[6][6]+board[6][7]+"②")
        print("①"+board[7][0]+board[7][1]+board[7][2]+board[7][3]+board[7][4]+board[7][5]+board[7][6]+board[7][7]+"①")
        print("☆ⓐⓑⓒⓓⓔⓕⓖⓗ☆")
    else:
        print("☆ⓗⓖⓕⓔⓓⓒⓑⓐ☆")
        print("①"+board[7][7]+board[7][6]+board[7][5]+board[7][4]+board[7][3]+board[7][2]+board[7][1]+board[7][0]+"①")
        print("②"+board[6][7]+board[6][6]+board[6][5]+board[6][4]+board[6][3]+board[6][2]+board[6][1]+board[6][0]+"②")
        print("③"+board[5][7]+board[5][6]+board[5][5]+board[5][4]+board[5][3]+board[5][2]+board[5][1]+board[5][0]+"③")
        print("④"+board[4][7]+board[4][6]+board[4][5]+board[4][4]+board[4][3]+board[4][2]+board[4][1]+board[4][0]+"④")
        print("⑤"+board[3][7]+board[3][6]+board[3][5]+board[3][4]+board[3][3]+board[3][2]+board[3][1]+board[3][0]+"⑤")
        print("⑥"+board[2][7]+board[2][6]+board[2][5]+board[2][4]+board[2][3]+board[2][2]+board[2][1]+board[2][0]+"⑥")
        print("⑦"+board[1][7]+board[1][6]+board[1][5]+board[1][4]+board[1][3]+board[1][2]+board[1][1]+board[1][0]+"⑦")
        print("⑧"+board[0][7]+board[0][6]+board[0][5]+board[0][4]+board[0][3]+board[0][2]+board[0][1]+board[0][0]+"⑧")
        print("☆ⓗⓖⓕⓔⓓⓒⓑⓐ☆")


def game():
    #keeps running until there is a checkmate or draw
    while checkmate() == False and draw() == False:
        #makes the variables global
        global message
        global player
        global previous_board
        #prints board
        print_board()
        #prints the message
        print(message)
        #prints the current player and if there is a check
        if check() == True:
            print(player,"Check!")
        else:
            print(player)
        #asks for the move
        move = input("What is your move? ")
        #if the move isn't legal it prints everything again and asks for a move
        while legal(move) == False:
            message = "That move is not legal!"
            print_board()
            print(message)
            if check() == True:
                print(player,"Check!")
            else:
                print(player)
            message = ""
            move = input("What is your move? ")
        previous_board = [list(x) for x in board]
        #if the move is legal it moves the piece
        move_piece(move)
        #switches the player
        if player == "Player 1":
            player = "Player 2"
        else:
            player = "Player 1"
    if checkmate() == True:
        print_board()
        print("Checkmate!")
        if player == "Player 1":
            player = "Player 2"
        else:
            player = "Player 1"
        print(player,"won!")
    else:
        print_board()
        print("Draw!")
        print(message)


#prints the introduction
print('''           Chess
---------------------------
Rules of Chess:
The chess board is 8 by 8
and has 64 squares. There
are 2 players. Each player
has 16 pieces.




''')
input("Press enter to continue")
print('''The board looks like this:
☆ⓐⓑⓒⓓⓔⓕⓖⓗ☆
⑧♜♞♝♛♚♝♞♜⑧
⑦♟♟♟♟♟♟♟♟⑦
⑥        ⑥
⑤        ⑤
④        ④
③        ③
②♙♙♙♙♙♙♙♙②
①♖♘♗♕♔♗♘♖①
☆ⓐⓑⓒⓓⓔⓕⓖⓗ☆
''')
input("Press enter to continue")
print('''♔: Each player has 1. The
king is the most important
piece, as checkmating your
enemy’s king results in
your victory.  The king can
move one space vertically,
horizontally, and
diagonally. If another
piece can attack the king,
it is check. If the player
has no way to get out of
check, it is checkmate.''')
input("Press enter to continue")
print('''If the king is not in
check and the player has no
legal move, then it is
stalemate.







''')
input("Press enter to continue")
print('''♕: The queen can move any
number of squares
vertically, horizontally,
and diagonally. Each player
has 1. This is the most
powerful piece.





''')
input("Press enter to continue")
print('''♖: The rook can move any
number of squares
vertically and
horizontally. Each player
has 2.






''')
input("Press enter to continue")
print('''♗: The bishop can move any
number of squares
diagonally. Each player has
2.







''')
input("Press enter to continue")
print('''♘: The knight can move
such as this:
☆ⓐⓑⓒⓓⓔⓕⓖⓗ☆
⑧        ⑧
⑦        ⑦
⑥   ♞ ♞  ⑥
⑤  ♞   ♞ ⑤
④    ♘   ④
③  ♞   ♞ ③
②   ♞ ♞  ②
①        ①
☆ⓐⓑⓒⓓⓔⓕⓖⓗ☆''')
input("Press enter to continue")
print('''Each player has 2.  The
knight can ‘jump’ past
pieces in his path to reach
his destination.







''')
input("Press enter to continue")
print('''♙: The pawn may move
forward 2 on the first move
and can move forward 1 on
the rest of the moves. The
pawn attacks by going
diagonally one square. if
the pawn goes to the other
side of the board, it can
turn into another piece.
Pawns can't go backwards.
Each player has 8.
''')
input("Press enter to continue")
print('''Pieces cannot land on
pieces of its own color.
Except for the knight,
pieces cannot go over other
pieces.






''')
input("Press enter to continue")
print('''Castling: If the king and a
rook have not moved, and no
other pieces are in between
the two, the player may
move the king 2 spaces
towards the rook and the
rook goes to the other side
of the king and is right
next to it. The king can't
be in check, pass through
check, or end up in check
in order to castle.''')
input("Press enter to continue")
print('''En Passant: When a pawn
advances two squares from
its starting position and
there is an opponent's pawn
on an adjacent file next to
its destination square,
then the opponent's pawn
can capture it en passant
(in passing), and move to
the square the pawn passed
over.
''')
input("Press enter to continue")
print('''Draws: If 50 moves have
passed and no pieces have
been captured or no pawns
have been moved, it is a
draw. Or, if the same piece
positions occur 3 times, it
is a draw. Further, should
both players wish for a
draw, one will occur.
Stalemates also results in
a draw.
''')
input("Press enter to continue")
print('''How to Play:
To move in this game, you
have to type in your move
in this format:
piece1 piece2
For example: a2a3 would
move the piece in a2 to a3.

Player 1 is white.
Player 2 is black.

''')
input("Press enter to continue")
print('''Castling:
Type in 0-0 if you want to
castle to the kingside.
Type in 0-0-0 if you want
castle to the queenside.
Pawn Promotion:
Type in the piece that you
want the pawn to become.
Queen=Q, Rook=R, Knight=N,
and Bishop=B. For example,
a7a8Q moves the pawn in a7
to a8 and makes it a queen.''')
input("Press enter to continue")
#starts the game
game()
