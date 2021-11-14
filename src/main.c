#include "board.h"
#include "game.h"

Board board;
Move game_moves[MAX_GAME_LENTH];
int game_position;

/* Test file for code */
int main() {
    /* Bobby Fischer vs. Boris Spassky 1992 */

    load_pgn(
        "1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 4. Ba4 Nf6 5. O-O Be7 6. Re1 b5 7. Bb3 "
        "d6 8. c3 O-O 9. h3 Nb8 10. d4 Nbd7 11. c4 c6 12. cxb5 axb5 13. Nc3 "
        "Bb7 14. Bg5 b4 15. Nb1 h6 16. Bh4 c5 17. dxe5 Nxe4 18. Bxe7 Qxe7 19. "
        "exd6 Qf6 20. Nbd2 Nxd6 21. Nc4 Nxc4 22. Bxc4 Nb6 23. Ne5 Rae8 24. "
        "Bxf7+ Rxf7 25. Nxf7 Rxe1+ 26. Qxe1 Kxf7 27. Qe3 Qg5 28. Qxg5 hxg5 29. "
        "b3 Ke6 30. a3 Kd6 31. axb4 cxb4 32. Ra5 Nd5 33. f3 Bc8 34. Kf2 Bf5 "
        "35. Ra7 g6 36. Ra6+ Kc5 37. Ke1 Nf4 38. g3 Nxh3 39. Kd2 Kb5 40. Rd6 "
        "Kc5 41. Ra6 Nf2 42. g4 Bd3 43. Re6");

    /* Game to test pgn edge cases */
    load_pgn(
        "1. a4 e5 2. a5 e4 3. a6 e3 4. axb7 Nf6 5. bxa8=N exf2+ 6. Kxf2 d6 7. "
        "Nc3 Qd7 8. Nxc7+ Kd8 9. N7d5 Ng8 10. Nf3 Qf5 11. Kg1 Qe4 12. Nf6 Na6 "
        "13. Ng5 Nc7 14. Nh3 Ne7 15. Nf2 Ncd5 16. Nb1 Nc7 17. d3 Bb7 18. Nd2 "
        "Ned5 19. Nf2xe4 Be7 20. c4 Bxf6 21. b3 Ke7 22. Ba3 Bc6 23. Nxf6 Kxf6 "
        "24. e4 Ne3 25. Qf3+ Nf5 26. Qxf5+ Ke7 27. Be2 g5 28. g4 Ba4 29. Kg2 "
        "Bxb3 30. Bc5 Bd1 31. Rhxd1 Nb5 32. Rxa7+ Kf8 33. Rda1 Na3 34. R7xa3 "
        "dxc5 35. Ra8+ Kg7 36. Rxh8 Kxh8 37. Qxh7+ Kxh7 38. Bf3 f5 39. e5 Kg6 "
        "40. e6 Kf6 41. e7 Kxe7 42. Bc6 Kd6 43. Ra7 Kxc6 44. Ra6+ Kb7 45. Rb6+ "
        "Kxb6 46. d4 cxd4 47. Ne4 fxe4 48. c5+ Kxc5 49. h4 gxh4 50. g5 Kd6 51. "
        "g6 Ke7 52. g7 Kf7 53. g8=N Kxg8 54. Kh3 d3 55. Kxh4 e3 56. Kg3 d2 57. "
        "Kg2 d1=Q 58. Kg3 e2 59. Kf4 e1=Q 60. Kf5 Qe8 61. Kf4 Qde1 62. Kg5 "
        "Q1e5+ 63. Kh4 Q8b5 64. Kg4 Qeb2 65. Kf3 Qc4 66. Ke3 Qbd4+ 67. Kf3 "
        "Qcc3+ 68. Kg2 Qdd2+ 69. Kh1 Qcc1");

    /* Mate in 11 if black plays g7h8*/
    load_fen("2b2r2/2p3k1/pp2P3/3B2R1/8/2P4P/PPQ1pq2/2K5 b - - 0 26");
    load_fen("2b1N3/2p5/pp1k4/3B4/4Q3/2P4P/PP2p3/2K5 b - - 2 35");


    start_game(ONE_PLAYER, 6);

    return SUCCESS;
}
