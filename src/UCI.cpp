#include "UCI.h"
#include "Board.h"
#include <iostream>
#include <sstream>
#include <string>
#include "Search.h"
#include <cstring>
#include <chrono>
#include "TranspositionTable.h"
#include "SearchStats.h"
#include "Evaluation.h"
#include "Perft.h"
namespace UCI
{

Board board;
    
void Loop()
{
    std::string command;

   board.LoadFEN(
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");


    while(std::getline(std::cin, command))
    {
        std::stringstream ss(command);

        std::string token;
        ss >> token;

        if(token == "uci")
        {
            std::cout << "id name KnightShift\n";
            std::cout << "id author ayu shhhh\n";
            std::cout << "uciok\n";
        }

        else if(token == "isready")
        {
            std::cout << "readyok\n";
        }

        else if(token == "ucinewgame")
{
    board.LoadFEN(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    TT::Clear();

    std::memset(
        Search::historyTable,
        0,
        sizeof(Search::historyTable));

    std::memset(
        Search::continuationHistory,
        0,
        sizeof(Search::continuationHistory));

    std::memset(
        Search::killerMoves,
        0,
        sizeof(Search::killerMoves));
}

        else if(token == "position")
        {
            std::string type;
            ss >> type;

            if(type == "startpos")
            {
                board.LoadFEN(
                    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            }
            else if(type == "fen")
            {
                std::string fen;
                std::string part;

                for(int i = 0; i < 6; i++)
                {
                    ss >> part;

                    if(i)
                        fen += " ";

                    fen += part;
                }

                board.LoadFEN(fen);
            }

            std::string word;

            if(ss >> word)
            {
                if(word == "moves")
                {
                    std::string moveText;

                    while(ss >> moveText)
                    {
                        Move move = board.ParseMove(moveText);

                        if(move)
                        {
                            UndoInfo undo;
                            board.MakeMove(move, undo);
                        }
                    }
                }
            }
        }


        else if(token == "eval")
            {
                std::cout
                    << Evaluation::Evaluate(board)
                    << '\n';
            }


            else if(token == "perft")
            {
                int depth = 1;
                if(ss >> depth)
                {
                    auto start = std::chrono::steady_clock::now();
                    U64 nodes = Perft::Run(board, depth);
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start).count();
                    uint64_t nps = elapsed > 0 ? (nodes * 1000) / elapsed : 0;
                    std::cout << "Nodes: " << nodes
                              << " Time: " << elapsed << "ms"
                              << " NPS: " << nps << std::endl;
                }
            }

            else if(token == "bench")
            {
                struct BenchPos { const char* name; const char* fen; U64 expected; };
                const BenchPos positions[] = {
                    { "Pos 1 (Initial)", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 197281ULL },
                    { "Pos 2 (Kiwipete)", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 4085603ULL },
                    { "Pos 3 (Silver)", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 43238ULL },
                    { "Pos 4 (Talkchess)", "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1", 422333ULL },
                    { "Pos 5 (CPW)", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 2103487ULL },
                    { "Pos 6 (Fine 70)", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 3894594ULL }
                };

                int passed = 0;
                U64 totalNodes = 0;
                auto benchStart = std::chrono::steady_clock::now();

                for(const auto& pos : positions)
                {
                    Board b;
                    b.LoadFEN(pos.fen);
                    auto tStart = std::chrono::steady_clock::now();
                    U64 nodes = Perft::Run(b, 4);
                    auto tElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - tStart).count();
                    totalNodes += nodes;
                    bool ok = (nodes == pos.expected);
                    if(ok) passed++;
                    std::cout << pos.name << ": " << (ok ? "PASS" : "FAIL")
                              << " (" << nodes << " nodes, " << tElapsed << "ms)" << std::endl;
                }

                auto totalElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - benchStart).count();
                uint64_t benchNps = totalElapsed > 0 ? (totalNodes * 1000) / totalElapsed : 0;

                std::cout << "========================================" << std::endl;
                std::cout << "Perft Benchmark: " << passed << " / 6 passed" << std::endl;
                std::cout << "Total Nodes: " << totalNodes << std::endl;
                std::cout << "Total Time: " << totalElapsed << "ms" << std::endl;
                std::cout << "NPS: " << benchNps << std::endl;
                std::cout << "========================================" << std::endl;
            }

            else if(token == "d")
            {
                board.Print();
            }

            else if(token == "stop")
            {
                Search::stopSearch = true;
            }


            else if(token == "go")
{
    int depth = 64;

    Search::useTimeControl = false;
    Search::searchTime = 0;

    int wtime = -1;
    int btime = -1;
    int winc = 0;
    int binc = 0;
    int movetime = -1;
    int movestogo = 30;

    std::string word;

    while(ss >> word)
    {
        if(word == "depth")
        {
            ss >> depth;
        }
        else if(word == "movetime")
        {
            ss >> movetime;
        }
        else if(word == "wtime")
        {
            ss >> wtime;
        }
        else if(word == "btime")
        {
            ss >> btime;
        }
        else if(word == "winc")
        {
            ss >> winc;
        }
        else if(word == "binc")
        {
            ss >> binc;
        }
        else if(word == "movestogo")
        {
            ss >> movestogo;

            if(movestogo <= 0)
                movestogo = 30;
        }
    }

    if(movetime != -1)
    {
        Search::searchTime = movetime;
        Search::useTimeControl = true;
    }
    else
    {
        int remaining =
            (board.side == WHITE) ? wtime : btime;

        int increment =
            (board.side == WHITE) ? winc : binc;

        if(remaining > 0)
        {
            Search::searchTime =
                remaining / movestogo
                + increment / 2;

            if(Search::searchTime < 20)
                Search::searchTime = 20;

            if(Search::searchTime > remaining / 2)
                Search::searchTime = remaining / 2;

            Search::useTimeControl = true;
        }
    }

    Search::stopSearch = false;

    Move bestMove =
        Search::FindBestMove(
            board,
            depth);

    std::cout
        << "bestmove "
        << MoveEncoding::ToString(bestMove)
        << std::endl;
}
        else if(token == "quit")
        {
            break;
        }
    }
}

}