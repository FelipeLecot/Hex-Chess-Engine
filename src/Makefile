all: uci.c
	@gcc uci.c  bitboards.c evaluatemove.c evaluation.c san.c tt.c zobrist.c search.c board.c fen.c utils.c movegen.c magics.c -o chessengine

test: test.c
	@gcc test.c -o test bitboards.c evaluation.c san.c zobrist.c board.c fen.c utils.c movegen.c magics.c -L../libs/unity -lunity
	@./test
	@rm test
