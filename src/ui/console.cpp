#include "mainwindow.h"
#include "../../ui/ui_mainwindow.h"
#include "../protocol/protocol.h"
#include "../protocol/deck.h"
#include "../protocol/functions.h"
#include "../other/stringfunc.h"
#include <bitset>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <QScrollBar>

std::vector<std::string> MainWindow::parse_cmd ( std::string& command ) {
    std::vector<std::string> res = {"SUCCESS"};
    std::bitset<8> flags = {0};
    enum console_parse_flags {
        QUOTE = 0,
        VAR   = 1,
        SPACE = 2,
        SQUOTED=3,
        QUOTED= 4,
        PARTED= 5,
        SQUOTE= 6,
        ESC   = 7,
    };

    int s = 0;
    int v;
    int i = 0;
    for (; i < command.size() ; i++ ) {
        if ( flags[ESC] ) {
            command.erase(command.begin()+i-1, command.begin()+i);
            i--;
            flags[ESC] = false;
            continue;
        }

        if ( command[i] == ' ' ) {
            if ( flags[SPACE] || i == 0 ) {
                command.erase(i, 1);
                i--;
                continue;
            }
            flags[SPACE] = true;
            if ( flags[VAR] ) {
                if ( flags[QUOTE] ) {
                    flags[VAR] = false;
                    std::string var = conVars[command.substr(v, i-v)];
                    int rsize = i-v+1;
                    command.replace(v-1, rsize, var);
                    i += var.size() - rsize + 1;
                } else {
                    flags[VAR] = false;
                    std::string var = command.substr(v, i-v);
                    res.push_back(conVars[var]);
                    s = i+1;
                }
            } else if ( !( flags[QUOTE] || flags[SQUOTE] || flags[QUOTED] || flags[SQUOTED] )) {
                res.push_back(command.substr(s, i-s));
                s = i+1;
            } else if ( flags[QUOTED] || flags[SQUOTED] ) {
                s = i+1;
                flags[QUOTED] = false;
                flags[SQUOTED] = false;
            }
        } else if ( command[i] == '"' && ! flags[SQUOTE] ) {
            if ( flags[QUOTE] ) {
                flags[QUOTE] = false;
                if ( i > s+1 ) {
                    std::string quoted = command.substr(s+1, i-s-1);
                    if ( flags[VAR] ) {
                        int b = v-s-1;
                        std::string var = quoted.substr(b, quoted.size()-b);
                        quoted = quoted.substr(0, b-1) + conVars[var];
                    }
                    res.push_back(quoted);
                    flags[QUOTED] = true;
                }
            } else if ( flags[SPACE] ) {
                flags[QUOTE] = true;
                flags[QUOTED] = false;
            } else {
                res[0] = "PARSE ERROR: Unexpected '\"' token";
                break;
            }
            flags[SPACE] = false;
        } else if ( command[i] == '\'' && ! flags[QUOTE] ) {
            if ( flags[SQUOTE] && !flags[QUOTE] ) {
                flags[SQUOTE] = false;
                if ( i > s+1 ) {
                    std::string quoted = command.substr(s+1, i-s-1);
                    res.push_back(quoted);
                    flags[SQUOTED] = true;
                }
            } else if ( flags[SPACE] ) {
                flags[SQUOTE] = true;
                flags[SQUOTED] = false;
            } else {
                res[0] = "PARSE ERROR: Unexpected \"'\" token";
                break;
            }
        } else if ( command[i] == '$' && !flags[SQUOTE] ) {
            if ( flags[SPACE] || command[i-1] == '"' ) {
                flags[SPACE] = false;
                flags[VAR] = true;
                v = i+1;
            } else {
                res[0] = "PARSE ERROR: Unexpected '$' token";
                return res;
            }
        } else if ( command[i] == ';' && !( flags[SQUOTE] || flags[QUOTE] ) ) {
            flags[PARTED] = true;
            if ( command.size() > i+1 && command[i+1] == ' ' && flags[SPACE] ) {
                command = command.substr(i, command.size()-i+1);
            } else {
                res[0] = "PARSE ERROR: Unexpected ';' token";
                return res;
            }
            break;
        } else if ( ( command[i] == '&' || command[i] == '|' ) && !( flags[SQUOTE] || flags[QUOTE] ) )  {
            flags[PARTED] = true;
            if ( command.size() > i+2 && command[i+1] == command[i] && command[i+2] == ' ' && flags[SPACE] ) {
                command = command.substr(i+1, command.size()-i);
            } else {
                res[0] = "PARSE ERROR: Unexpected '"+std::string(1,command[i])+"' token";
            }
            break;
        } else if ( command[i] == '\\' ) {
            flags[ESC] = true;
        } else {
            std::string r = "PARSE ERROR: Expected ' ', got '";
            if ( flags[QUOTED] || flags[SQUOTED] ) res[0] = r+command[i]+'\'';
            flags[SPACE] = false;
        }
    }
    if ( flags[QUOTE] || flags[SQUOTE] ) res[0] = "PARSE ERROR: Unterminated quote";
    else if ( flags[VAR] ) {
        if ( i != s+1 ) {
            std::string var = command.substr(s+1, i-s-1);
            res.push_back(conVars[var]);
        }
    } else if ( !( flags[QUOTED] || flags[SQUOTED] || flags[PARTED] ) ) res.push_back(command.substr(s, i-s));

    if ( ! flags[PARTED] ) command = "";
    return res;
}

std::string MainWindow::conInterpret ( std::string command, bool* succ, void* result, size_t rsize ) {
    int cmdsize = -1;
    std::string ans;
    bool success;
    bool store_res = ( result != nullptr );
    if ( store_res ) {
        (*(char*)result)--;
        store_res = ( *(char*)result == 0 );
    }

    std::vector<std::string> cmd = parse_cmd(command);
    if ( cmd.size() == 1 || cmd[1] == "" ) return "";
    std::cout << command << std::endl;
    for ( const auto& s : cmd ) std::cout << s << ' '; std::cout << std::endl;

    if ( cmd[0] != "SUCCESS" ) {
        std::string ans = cmd[0];
        success = false;
        return ans;
    } else if ( cmd.size() >= 4 && cmd[2] == "=" ) { // [var] = [val]
        cmdsize = 4;

        if ( conVars.find(cmd[1]) == conVars.end() )
            conVars.insert({cmd[1], cmd[3]});
        else
            conVars[cmd[1]] = cmd[3];
        ans = "";
        success = true;
    } else if ( cmd.size() >= 3 && cmd[1] == "print" ) { // print [var]
        cmdsize = 3;

        ans = conVars[cmd[2]];
        success = (ans != "");

        if ( success && store_res && rsize > 1 ) {
            size_t copy_len = std::min(ans.size(), rsize-1);
            std::memcpy(result, ans.data(), copy_len);
            ((char*)result)[copy_len] = 0;
        }
    } else if ( cmd.size() >= 2 && cmd[1] == "printall" ) { // printall
        cmdsize = 2;

        std::vector<std::string> vars;
        for ( const auto& var : conVars ) {
            if ( var.second != "" )
                vars.push_back( var.first + ": " + var.second );
        }
        ans = join(vars, "\n");
        ans = ans.substr(1, ans.size()-1);
        success = true;
    } else if ( cmd.size() >= 2 && cmd[1] == "history" ) { // history
        cmdsize = 2;

        std::ifstream conHistory(CONHISTORY);
        int asize = historyGetPointerMax+1;
        char answer[asize];
        conHistory.read(answer, asize-1);
        conHistory.close();
        answer[asize-1] = 0;

        ans = answer;
        success = true;
    } else if ( cmd.size() >= 2 && cmd[1] == "clear" ) { // clear
        cmdsize = 2;

        ans = "CLEAR";
        success = true;
    } else if ( cmd.size() >= 2 && cmd[1] == "clear-his" ) { // clear-his
        cmdsize = 2;
        std::ofstream conHistory(CONHISTORY, std::ios::trunc);
        conHistory.close();

        ans = "Cleared history";
        success = true;
    } else if ( cmd.size() >= 2 && cmd[1] == "disconnect" ) { // disconnect
        cmdsize = 2;

        protocol::ErrorCode res = protocol::disconnect();
        if ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "Disconnected";
                success = true;
                break;
            case protocol::INVALID_SOCK:
                ans = "ERROR: Not connected";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
        }
    } else if ( cmd.size() >= 4 && cmd[1] == "connect" ) { // connect [addr] [port]
        cmdsize = 4;

        try {
            protocol::ErrorCode res = protocol::connect(cmd[2].data(), (unsigned short)std::strtoul(cmd[3].c_str(), NULL, 0));
            if  ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
            switch ( res ) {
                case protocol::_NOERROR:
                    ans = "Connected";
                    success = true;
                    break;
                case protocol::SEND_ERROR:
                    ans = "ERROR: Failed to connect";
                    success = false;
                    break;
                case protocol::PROTOCOL_ERR:
                    ans = "ERROR: Invalid server";
                    success = false;
                    break;
                case protocol::INVALID_SOCK:
                    ans = "ERROR: Already connected";
                    success = false;
                    break;
                default:
                    ans = "ERROR: Unexpected error";
                    success = false;
            }
        } catch ( std::out_of_range ) {
            ans = "Invalid port";
            success = false;
        } catch ( std::invalid_argument ) {
            ans = "Invalid port";
            success = false;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "join" ) { // join [name]
        cmdsize = 3;

        char pass[8] = {0};
        ui->ChatOut->clear();
        protocol::ErrorCode res = protocol::join(cmd[2].data(), pass);
        if  ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "";
                ans.append(pass, 8);
                success = true;
                break;
            case protocol::RENAME:
                ans = "ERROR: RENAME";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Already joined";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "rejoin" ) { // rejoin [pass]
        cmdsize = 3;

        protocol::ErrorCode res = protocol::rejoin(cmd[2].data());
        if  ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "Rejoined successfully";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: No player with given pass";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Already joined";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 2 && cmd[1] == "ping" ) { // ping
        cmdsize = 2;

        int time;
        protocol::ErrorCode res = protocol::ping(time);
        if  ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "Received data. Elapsed time: "+std::to_string(time)+"ms";
                success = true;
                break;
            case protocol::TIMEOUT:
                ans = "ERROR: Didn't receive data. Timeout exceeded: "+std::to_string(time)+"ms";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send data";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Not joined";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "chat" ) { // chat [msg]
        cmdsize = 3;

        protocol::ErrorCode res = protocol::chat(cmd[2]); 
        if  ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Not joined";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "act" ) { // act [action]
        cmdsize = 3;

        protocol::ErrorCode res = protocol::action(cmd[2]);
        if  ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Not joined";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 4 && cmd[1] == "whisper" ) { // whisper [player] [msg]
        cmdsize = 4;

        protocol::ErrorCode res = protocol::whisper(cmd[2], cmd[3]);
        if  ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "Whispered to "+cmd[2];
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: No such player named \""+cmd[2]+"\"";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Not joined";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 4 && cmd[1] == "roll" ) { // roll [dice] [num]
        cmdsize = 4;

        protocol::ErrorCode res = protocol::roll(cmd[2], cmd[3]);
        if  ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_A_NUM:
                ans = "ERROR: Invalid number";
                success = false;
                break;
            case protocol::OUT_OF_RNG:
                ans = "ERROR: Out of Range";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Not joined";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 6 && cmd[1] == "deck" ) { // deck [src] [dest] [x] [y]
        cmdsize = 6;
        Deck* deck = deckByName(cmd[2]);
        if ( deck == nullptr ) {
            ans = "ERROR: Not found deck";
            success = false;
            if  ( store_res && rsize >= sizeof(int) ) *(int*)result = protocol::NOT_FOUND+27;
        } else if ( deck->empty() ) {
            ans = "ERROR: Local deck is empty. Try running \"cards "+cmd[2]+"\"";
            success = false;
            if  ( store_res && rsize >= sizeof(int) ) *(int*)result = protocol::EMPTY+27;
        } else {
            protocol::ErrorCode res = protocol::deck(cmd[2], cmd[3], cmd[4], cmd[5]);
            if  ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
            switch ( res ) {
                case protocol::_NOERROR:
                    ans = "";
                    success = true;
                    break;
                case protocol::EMPTY:
                    ans = "ERROR: The deck is empty";
                    success = false;
                    break;
                case protocol::NOT_FOUND:
                    ans = "ERROR: Not found destination or server version mismatched with client version";
                    success = false;
                    break;
                case protocol::NOT_A_NUM:
                    ans = "ERROR: Invalid number";
                    success = false;
                    break;
                case protocol::OUT_OF_RNG:
                    ans = "ERROR: Out of Range";
                    success = false;
                    break;
                case protocol::SEND_ERROR:
                    ans = "ERROR: Failed to send";
                    success = false;
                    break;
                case protocol::PROTOCOL_ERR:
                    ans = "ERROR: Not joined";
                    success = false;
                    break;
                default:
                    ans = "ERROR: Unexpected error";
                    success = false;
                    break;
            }
        }
    } else if ( cmd.size() >= 7 && cmd[1] == "move" ) { // move [src] [index] [dest] [x] [y]
        cmdsize = 7;
        CardContainer* spatial;
        if ( spatial == nullptr ) {
            ans = "ERROR: Not found source";
            success = false;
            if ( store_res && rsize >= sizeof(int) ) *(int*)result = protocol::NOT_FOUND+27;
        }
        protocol::ErrorCode res = protocol::move(cmd[2], cmd[3], cmd[4], cmd[5], cmd[6]);
        if ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: Not found source or destination";
                success = false;
                break;
            case protocol::NOT_A_NUM:
                ans = "ERROR: Invalid number";
                success = false;
                break;
            case protocol::OUT_OF_RNG:
                ans = "ERROR: Out of Range";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Not joined";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 5 && cmd[1] == "rotate" ) { // rotate [container] [index] [rot]
        cmdsize = 5;

        protocol::ErrorCode res = protocol::rotate(cmd[2], cmd[3], cmd[4]);
        if ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: Not such spatial container \""+cmd[2]+"\"";
                success = false;
                break;
            case protocol::NOT_A_NUM:
                ans = "ERROR: Invalid number";
                success = false;
                break;
            case protocol::OUT_OF_RNG:
                ans = "ERROR: Out of Range";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Not joined";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 4 && cmd[1] == "flip" ) { // flip [container] [index]
        cmdsize = 4;

        protocol::ErrorCode res = protocol::flip(cmd[2], cmd[3]);
        if ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: Not such spatial container \""+cmd[2]+"\"";
                success = false;
                break;
            case protocol::NOT_A_NUM:
                ans = "ERROR: Invalid number";
                success = false;
                break;
            case protocol::OUT_OF_RNG:
                ans = "ERROR: Out of Range";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Not joined";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "shuffle" ) { // shuffle [deck]
        cmdsize = 3;

        protocol::ErrorCode res = protocol::shuffle(cmd[2]);
        if ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: No such deck \""+cmd[2]+"\"";
                success = false;
                break;
            case protocol::NOT_A_NUM:
                ans = "ERROR: Invalid number";
                success = false;
                break;
            case protocol::OUT_OF_RNG:
                ans = "ERROR: Out of Range";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Not joined";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 4 && cmd[1] == "set" ) { // set [stat] [value]
        cmdsize = 4;

        protocol::ErrorCode res = protocol::set(cmd[2], cmd[3]);
        if ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "";
                success = true;
                break;
            case protocol::NOT_FOUND:
                ans = "ERROR: No such stat \""+cmd[2]+"\"";
                success = false;
                break;
            case protocol::NOT_A_NUM:
                ans = "ERROR: Invalid number";
                success = false;
                break;
            case protocol::OUT_OF_RNG:
                ans = "ERROR: Out of Range";
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Not joined";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "rename" ) { // rename [name]
        cmdsize = 3;

        protocol::ErrorCode res = protocol::rename(cmd[2]);
        if ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "Renamed to "+cmd[2];
                success = true;
                break;
            case protocol::RENAME:
                ans = "ERROR: There is already a player named \""+cmd[2]+'"';
                success = false;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Not joined";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "see" ) { // see [visible]
        cmdsize = 3;

        CardContainer* spatial = spatialByName(cmd[2], LOCALPLAYER);
        Player* plr = Players[cmd[2]];
        CardContainer* cards = nullptr;

        if ( spatial == nullptr ) {
            if ( plr == nullptr ) {
                ans = "ERROR: No such spatial or player \""+cmd[2]+"\"";
                success = false;
            } else {
                cards = plr->Equiped;
            }
        } else cards = spatial;

        if ( cards == nullptr && store_res && rsize >= sizeof(int) ) {
            *(int*)result = protocol::NOT_FOUND+27;
        } else {
            protocol::ErrorCode res = protocol::see(cmd[2], cards);
            if ( store_res && rsize >= sizeof(int) ) *(int*)result = res;

            switch ( res ) {
                case protocol::_NOERROR:
                    ans = "ID:\tNumber:\tX:\tY:\tRotation:\n\n";
                    for ( int i = 0 ; i < cards->size() ; i++ ) {
                        Card* crd = cards->at(i);
                        ans += std::to_string(i) + '\t' + crd->unparse_card() + '\t' + std::to_string(crd->x()) + '\t' + std::to_string(crd->y()) + '\t' + std::to_string(crd->rot()) + '\n';
                    }
                    success = true;
                    break;
                case protocol::NOT_FOUND:
                    ans = "ERROR: Not found container. Client version probably mismatching Server version";
                    success = false;
                    break;
                case protocol::SEND_ERROR:
                    ans = "ERROR: Failed to send";
                    success = false;
                    break;
                case protocol::PROTOCOL_ERR:
                    ans = "ERROR: Not joined";
                    success = false;
                    break;
                default:
                    ans = "ERROR: Unexpected error";
                    success = false;
                    break;
            }
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "cards" ) { // cards [nonvisible]
        cmdsize = 3;

        Deck* deck = deckByName(cmd[2]);
        Player* plr = Players[cmd[2]];
        CardContainer* cards = nullptr;

        if ( deck == nullptr ) {
            if ( plr == nullptr ) {
                ans = "ERROR: No such deck or player \""+cmd[2]+"\"";
                success = false;
            } else {
                cards = plr->Inventory;
            }
        } else cards = deck;

        if ( cards == nullptr && store_res && rsize >= sizeof(int) ) {
            *(int*)result = protocol::NOT_FOUND+27;
        } else {
            protocol::ErrorCode res = protocol::cards(cmd[2], cards);
            if ( store_res && rsize >= sizeof(int) ) *(int*)result = res;

            switch ( res ) {
                case protocol::_NOERROR:
                    ans = "ID:\tNumber:\tX:\tY:\tRotation:\n\n";
                    for ( int i = 0 ; i < cards->size() ; i++ ) {
                        Card* crd = cards->at(i);
                        ans += std::to_string(i) + '\t' + crd->unparse_card() + '\t' + std::to_string(crd->x()) + '\t' + std::to_string(crd->y()) + '\t' + std::to_string(crd->rot()) + '\n';
                    }
                    success = true;
                    break;
                case protocol::NOT_FOUND:
                    ans = "ERROR: Not found container. Client version probably mismatching Server version";
                    success = false;
                    break;
                case protocol::SEND_ERROR:
                    ans = "ERROR: Failed to send";
                    success = false;
                    break;
                case protocol::PROTOCOL_ERR:
                    ans = "ERROR: Not joined";
                    success = false;
                    break;
                default:
                    ans = "ERROR: Unexpected error";
                    success = false;
                    break;
            }
        }
    } else if ( cmd.size() >= 3 && cmd[1] == "stat" ) { // stat [player]
        cmdsize = 3;

        Player* plr = Players[cmd[2]];

        if ( plr == nullptr ) {
            ans = "ERROR: No such player \""+cmd[2]+"\"";
            success = false;
            if ( store_res && rsize >= sizeof(int) ) *(int*)result = protocol::NOT_FOUND+27;
        } else {
            std::string stats[3];
            protocol::ErrorCode res = protocol::stat(cmd[2], stats);
            if ( store_res && rsize >= sizeof(int) ) *(int*)result = res;

            switch ( res ) {
                case protocol::_NOERROR:
                    plr->Level=stats[0]; plr->Power=stats[1]; plr->Gold=stats[2];
                    ans = "LEVEL: "+stats[0]+"\nPOWER: "+stats[1]+"\nGOLD: "+stats[2];
                    success = true;
                    break;
                case protocol::NOT_FOUND:
                    ans = "ERROR: Not found container. Client version probably mismatching Server version";
                    success = false;
                    break;
                case protocol::SEND_ERROR:
                    ans = "ERROR: Failed to send";
                    success = false;
                    break;
                case protocol::PROTOCOL_ERR:
                    ans = "ERROR: Not joined";
                    success = false;
                    break;
                default:
                    ans = "ERROR: Unexpected error";
                    success = false;
                    break;
            }
        }
    } else if ( cmd.size() >= 2 && cmd[1] == "players" ) { // players
        cmdsize = 2;

        std::vector<std::string> plrs;
        protocol::ErrorCode res = protocol::players(&plrs);
        if ( store_res && rsize >= sizeof(int) ) *(int*)result = res;
        switch ( res ) {
            case protocol::_NOERROR:
                ans = "";
                for ( int i = 0 ; i < plrs.size() ; i++ )
                    ans += plrs[i] + '\n';
                success = true;
                break;
            case protocol::SEND_ERROR:
                ans = "ERROR: Failed to send";
                success = false;
                break;
            case protocol::PROTOCOL_ERR:
                ans = "ERROR: Not joined";
                success = false;
                break;
            default:
                ans = "ERROR: Unexpected error";
                success = false;
                break;
        }
    } else if ( cmd.size() >= 2 && cmd[1] == "status" ) { // status
        cmdsize = 2;
        Connection* conn = protocol::connection();
        if ( conn == nullptr ) {
            ans = "Not connected";
            success = false;
        } else {
            std::ostringstream answer;
            answer << "Connected\n";
            answer << "    Server: " << conn->Address() << '\n';
            answer << "    LocalPlayer: " << LOCALUSERNAME << '\n';

            ans = answer.str();
            success = true;
        }
    } else if ( cmd.size() >= 2 && cmd[1] == "help" ) { // help
        cmdsize = 2;

        std::ostringstream answer;
        answer << "Вы можете создавать свои переменные, используя синтаксис: \"[var] = [value]\",\n";
        answer << "и использовать их, через синтаксис: \"$[var]\"\n";
        answer << "Консоль поддерживает только строковые переменные.\n\n";

        answer << "Комманды:" << '\n';
        answer << "print [var]"                            << "\t\t\t" << "Вывести содержимое переменной"                                << '\n';
        answer << "printall"                               << "\t\t\t" << "Вывести все переменные и их содерживое"                       << '\n';
        answer << "history"                                << "\t\t\t" << "Вывести историю комманд"                                      << '\n';
        answer << "clear"                                  << "\t\t\t" << "Очистить консоль"                                             << '\n';
        answer << "clear-his"                              << "\t\t\t" << "Очистить историю отправленных комманд"                        << '\n';
        answer << "disconnect"                             << "\t\t\t" << "Отключиться от сервера"                                       << '\n';
        answer << "connect [address] [port]"               << "\t"     << "Подключиться к серверу по аддресу address:port"               << '\n';
        answer << "join [name]"                            << "\t\t\t" << "Присоединиться к игре как name"                               << '\n';
        answer << "rejoin [pass]"                          << "\t\t"   << "Перезайти к игре через пароль pass"                           << '\n';
        answer << "ping"                                   << "\t\t\t" << "Пропинговать сервак, чё непонятного???"                       << '\n';
        answer << "chat [message]"                         << "\t\t"   << "Написать в чат сообщение message"                             << '\n';
        answer << "act [action]"                           << "\t\t\t" << "Выполнить РП действие action в чате"                          << '\n';
        answer << "whisper [player] [message]"             << "\t"     << "Написать приватное сообщение message игроку player"           << '\n';
        answer << "roll [dice] [number]"                   << "\t\t"   << "Кинуть number костей кdice"                                   << '\n';
        answer << "deck [deck] [container] [x] [y]"        << "\t"     << "Взять карту из колоды deck и положить её в container в (x,y)" << '\n';
        answer << "move [spatial] [i] [container] [x] [y]" << "\t"     << "Переложить i-ю карту из spatial в container в (x,y)"          << '\n';
        answer << "rotate [spatial] [i] [rotation]"        << "\t"     << "Повернуть i-ю карту в spatial на значение rotation"           << '\n';
        answer << "flip [spatial] [i]"                     << "\t\t"   << "Перевернуть i-ю карту в spatial"                              << '\n';
        answer << "shuffle [deck]"                         << "\t\t"   << "Перемешать deck"                                              << '\n';
        answer << "set [stat] [value]"                     << "\t\t"   << "Изменить свой stat на value"                                  << '\n';
        answer << "rename [name]"                          << "\t\t"   << "Изменить свой никнейм на name"                                << '\n';
        answer << "see [visible]"                          << "\t\t"   << "Посмотреть список всех карт внутри visible"                   << '\n';
        answer << "cards [nonvisible]"                     << "\t\t"   << "Посмотреть список всех карт внутри nonvisible"                << '\n';
        answer << "stat [player]"                          << "\t\t"   << "Посмотреть stat игрока player"                                << '\n';
        answer << "players"                                << "\t\t\t" << "Посмотреть список всех игроков в игре"                        << '\n';
        answer << "status"                                 << "\t\t\t" << "Вывести информацию о сервере"                                 << '\n';
        answer << "help"                                   << "\t\t\t" << "Вывести этот текст"                                           << "\n\n";

        answer << "Понятия:" << '\n';
        answer << "container"  << '\t' << "Контейнер карт"                   << "\t\t\t" << "TREASURES, TRAPDOORS, TABLE, EQUIPPED, INVENTORY" << '\n';
        answer << "deck"       << '\t' << "Колода карт"                      << "\t\t\t" << "TREASURES, TRAPDOORS"                             << '\n';
        answer << "spatial"    << '\t' << "Пространственный контейнер карт"  << "\t"     << "TABLE, EQUIPPED, INVENTORY"                       << '\n';
        answer << "visible"    << '\t' << "Видимые контейнеры карт"          << "\t\t"   << "TABLE, EQUIPPED(Через имя игрока)"                << '\n';
        answer << "nonvisible" << '\t' << "Невидимые контейнеры карт"        << "\t\t"   << "TREAUSRES, TRAPDOORS, INVENTORY(Через имя игрока)"<< '\n';

        ans = answer.str();
        success = true;
    } else {
        ans = "ERROR: Unknown command \""+cmd[1]+"\". Try to run \"help\" to view the avaible list of commands";
        success = false;
    }

    if ( cmd.size() > cmdsize && cmdsize != -1 ) {
        if ( cmd[cmdsize] == ">" && cmd.size() > cmdsize+1 ) {
            if ( conVars.find(cmd[cmdsize+1]) == conVars.end() )
                conVars.insert({cmd[cmdsize+1], ans});
            else
                conVars[cmd[cmdsize+1]] = ans;
            ans = "";

            if ( cmd.size() > cmdsize+2 ) ans += "\nWarning: Unexpected token: \""+cmd[cmdsize]+"\"";
        } else ans += "\nWarning: Unexpected token: \""+cmd[cmdsize]+"\"";
    }
    if ( command != "" ) {
        if ( command[0] == ';' || ( command[0] == '&' && success ) || ( command[0] == '|' && !success ) ) {
            if ( store_res ) {
                char* res = (char*)result;
                (*res)++;
            }
            std::string ans2 = conInterpret(command.substr(1, command.size()-1), &success, result, rsize);
            if ( ans2 == "CLEAR" ) ans = ans2;
            else ans += ( ( ans == "" || ans2 == "" ) ? "" : "\n" ) + ans2;
        }
    }
    if ( succ != nullptr ) *succ = success;
    return ans;
}

void MainWindow::conOut ( QString text ) {
    if ( text != "" )
    ui->ConsoleOut->setText(ui->ConsoleOut->text() + text + '\n');
}

void MainWindow::conIn ( QString command, bool user, bool* succ, void* res, size_t rsize ) {
    conOut("> "+command);
    if ( user ) {
        scrollLocked = false;
        std::ofstream conHistory(CONHISTORY, std::ios::app | std::ios::ate | std::ios::binary );
        if ( conHistory ) {
            if ( !command.isEmpty() ) conHistory << command.toStdString() << '\n';
            conPatternSet = false;
            historyGetPointer = (int)conHistory.tellp()-1;
            historyGetPointerMax = historyGetPointer;
            conHistory.close();
        } else {
            std::cout << "WARNING: Can't access \""+CONHISTORY+'"';
        }
    }
    std::string result = conInterpret(command.toStdString(), succ, res, rsize);
    if ( result.substr(0, 5) == "CLEAR" ) {
        ui->ConsoleOut->clear();
        result = result.substr(5, result.size()-5);
    }
    conOut(QString::fromStdString((result)));
}

// GetPointer should point to '\n'. Moves pointer to next '\n'
// returns true if reached the top bounds
bool MainWindow::conHistoryDown ( std::ifstream* conHistory ) {
    if ( !conHistory->good() || historyGetPointer >= historyGetPointerMax ) return true;
    char c = '0';
    while ( c != '\n' && conHistory->tellg() != historyGetPointerMax ) {
        conHistory->seekg(1, std::ios::cur);
        c = conHistory->peek();
    } return false;
}

// GetPointer should point to '\n'. Moves pointer to prev '\n'
// returns true if reached the bottom bounds
bool MainWindow::conHistoryUp ( std::ifstream* conHistory ) {
    if ( !conHistory->good() || conHistory->tellg() == 0 ) return true;
    char c = '0';
    while ( c != '\n' && (int)conHistory->tellg() > 0 ) {
        conHistory->seekg(-1, std::ios::cur);
        c = conHistory->peek();
    } return false;
}

void MainWindow::conHistoryFind ( bool up ) {
    if ( !conPatternSet ) {
        conPattern = ui->ConsoleIn->text().toStdString();
        conPatternSet = true;
    }
    std::ifstream conHistory(CONHISTORY, std::ios::binary);
    conHistory.seekg(historyGetPointer);
    std::string line;
    bool reachedBounds;
    do {
        if (up) reachedBounds = conHistoryUp  (&conHistory);
        else    reachedBounds = conHistoryDown(&conHistory);
        historyGetPointer = conHistory.tellg();
        if ( reachedBounds ) break;

        if ( conHistory.peek() == '\n' ) conHistory.seekg(1, std::ios::cur); // Make sure it doesn't read first '\n' character
        std::getline(conHistory, line);
        conHistory.seekg(historyGetPointer);
    } while ( line.substr(0, conPattern.size() ) != conPattern );

    if ( !reachedBounds ) ui->ConsoleIn->setText(QString::fromStdString(line));
    conHistory.close();
}

void MainWindow::on_ConsoleIn_returnPressed()
{
    QString command = ui->ConsoleIn->text();
    ui->ConsoleIn->setText("");

    conIn(command, true);
}

void MainWindow::on_ConsoleIn_textEdited()
{
    conPatternSet = false;
}

void MainWindow::on_ConsoleVerticalScrollbar_rangeChanged() {
    if ( !scrollLocked ) {
        QScrollBar* bar = ui->ConsoleScrollArea->verticalScrollBar();
        bar->setValue(bar->maximum());
    }
}
void MainWindow::on_ConsoleVerticalScrollbar_valueChanged() {
    QScrollBar* bar = ui->ConsoleScrollArea->verticalScrollBar();

    if ( bar->value() == bar->maximum() ) scrollLocked = false;
    else scrollLocked = true;
}
