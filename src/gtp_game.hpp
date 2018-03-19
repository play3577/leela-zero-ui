#pragma once

#include "gtp_agent.h"
#include <iostream>
#include <algorithm>

struct genmove_stats {
    int move;
    int visits;
    float probs;
    float score;
};

template<class TGTP>
class GameAdvisor : public TGTP {
public:

    function<void()> onResetGame;
    function<void(bool,int, const std::vector<genmove_stats>&, const float)> onThinkMove;
    function<void()> onThinkPass;
    function<void()> onThinkResign;
    function<void()> onThinkBegin;
    function<void()> onThinkEnd;
    function<void(const string&)> onGtpIn;
    function<void(const string&)> onGtpOut;


    GameAdvisor();

    bool next_move_is_black() const {
        return commit_pending_ ? !next_side_ : next_side_;
    }

    void set_init_cmds(const std::vector<string>& cmds) {
        init_cmds = cmds;
    }

    void hint_both();
    void hint_black();
    void hint_white();
    void hint();
    void hint_off();
    void reset();
    

    void place(bool black_move, int pos) {
        play_move(black_move, pos);
    }

    void pop_events() {
        std::vector<string> ev;
        while (events_.try_pop(ev)) {
            if (ev[0] == "reset") {
                if (onResetGame)
                    onResetGame();
            }
            if (ev[0] == "predict") {
                bool black_move = ev[1] == "b";
                int move = stoi(ev[2]);
                if (onThinkMove)
                    onThinkMove(black_move, move, stats_, nn_eval_);
            }
            else if (ev[0] == "pass") {
                if (onThinkPass)
                    onThinkPass();
            }
            else if (ev[0] == "resign") {
                if (onThinkResign)
                    onThinkResign();
            }
            else if (ev[0] == "think") {
                if (onThinkBegin)
                    onThinkBegin();
            }
            else if (ev[0] == "think_end") {
                if (onThinkEnd)
                    onThinkEnd();
            }
            else if (ev[0] == "input") {
                if (onGtpIn)
                    onGtpIn(ev[1]);
            }
            else if (ev[0] == "output") {
                if (onGtpOut)
                    onGtpOut(ev[1]);
            }
        }
    }


protected:
    void play_move(bool black_move, const int pos);
    void execute_next_move();
    void do_think(bool black_move);
    void think();

private:
    void processStderr(const string& output);
    void processStderrOneLine(const string& line);
    bool parseLeelaDumpStatsLine(const string& line);
    bool parseLeelaNNEval(const string& line);

    string buffer_;
    bool eat_stderr{false};


private:
    bool next_side_{true};
    bool hint_black_{false};
    bool hint_white_{false};
    bool resetted_{false};

    safe_queue<std::vector<string>> events_;
    std::atomic<bool> pending_reset_{false};
    std::atomic<bool> commit_pending_{false};
    bool commit_player_;
    int commit_pos_;
    std::vector<string> init_cmds;

    safe_dqueue<GtpState::move_t> to_moves_;

    
    std::vector<genmove_stats> stats_;
    float nn_eval_;
};


